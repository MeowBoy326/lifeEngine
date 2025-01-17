#include "Misc/CoreGlobals.h"
#include "Misc/EngineGlobals.h"
#include "Misc/PhysicsGlobals.h"
#include "System/CameraManager.h"
#include "System/Package.h"
#include "PhysicsInterface.h"
#include "Actors/Actor.h"
#include "System/World.h"
#include "Logger/LoggerMacros.h"
#include "Render/Scene.h"

#if WITH_EDITOR
#include "WorldEd.h"
#endif // WITH_EDITOR

/*
==================
CWorld::CWorld
==================
*/
CWorld::CWorld() 
	: isBeginPlay( false )
	, scene( new CScene() )
#if WITH_EDITOR
	, name( TEXT( "Unknown" ) )
#endif // WITH_EDITOR
{}

/*
==================
CWorld::~CWorld
==================
*/
CWorld::~CWorld()
{
	CleanupWorld();
	delete scene;
}

/*
==================
CWorld::BeginPlay
==================
*/
void CWorld::BeginPlay()
{
	// If allready playing started, do nothing
	if ( isBeginPlay )
	{
		return;
	}

	// Init all actors
	for ( uint32 index = 0; index < ( uint32 )actors.size(); ++index )
	{
		actors[ index ]->BeginPlay();
	}

	// Init all physics in actors
	for ( uint32 index = 0; index < actors.size(); ++index )
	{
		actors[ index ]->InitPhysics();
	}

	g_CameraManager->BeginPlay();
	isBeginPlay = true;
}

/*
==================
CWorld::EndPlay
==================
*/
void CWorld::EndPlay()
{
	// If not playing, do nothing
	if ( !isBeginPlay )
	{
		return;
	}

	// End play and destroy physics for all actors
	for ( uint32 index = 0; index < ( uint32 ) actors.size(); ++index )
	{
		ActorRef_t		actor = actors[ index ];
		actor->EndPlay();
		actor->TermPhysics();
	}

	g_CameraManager->EndPlay();
	isBeginPlay = false;
}

/*
==================
CWorld::Tick
==================
*/
void CWorld::Tick( float InDeltaTime )
{
	// Tick all actors
	for ( uint32 index = 0, count = ( uint32 )actors.size(); index < count; ++index )
	{
		AActor*		actor = actors[ index ];
		actor->Tick( InDeltaTime );
		actor->SyncPhysics();
	}

	// Destroy actors if need
	if ( !actorsToDestroy.empty() )
	{
		for ( uint32 index = 0, count = actorsToDestroy.size(); index < count; ++index )
		{
			DestroyActor( actorsToDestroy[ index ], true );
		}
		actorsToDestroy.clear();
	}
}

/*
==================
CWorld::Serialize
==================
*/
void CWorld::Serialize( CArchive& InArchive )
{
	if ( InArchive.IsSaving() )
	{
		InArchive << ( uint32 )actors.size();
		for ( uint32 index = 0, count = ( uint32 )actors.size(); index < count; ++index )
		{
			AActor*			actor = actors[ index ];
			InArchive << actor->GetClass()->GetName();
			actor->Serialize( InArchive );
		}
	}
	else
	{
		// Clear world
		CleanupWorld();

		uint32		countActors = 0;
		InArchive << countActors;

		for ( uint32 index = 0; index < countActors; ++index )
		{
			// Serialize class name
			uint32				classNameSize = 0;
			std::wstring		className;
			InArchive << className;

			// Spawn actor, serialize and add to array
			AActor*			actor = SpawnActor( CClass::StaticFindClass( className.c_str() ), Math::vectorZero, Math::quaternionZero );
			actor->Serialize( InArchive );
		}
	}

#if WITH_EDITOR
	// Getting path and file name
	filePath	= InArchive.GetPath();
	name		= filePath;
	{
		Sys_NormalizePathSeparators( name );
		std::size_t			pathSeparatorPos = name.find_last_of( PATH_SEPARATOR );
		if ( pathSeparatorPos != std::string::npos )
		{
			name.erase( 0, pathSeparatorPos + 1 );
		}

		uint32 dotPos = name.find_last_of( TEXT( "." ) );
		if ( dotPos != std::string::npos )
		{
			name.erase( dotPos, name.size() + 1 );
		}
	}

	bDirty = false;
#endif // WITH_EDITOR
}

/*
==================
CWorld::CleanupWorld
==================
*/
void CWorld::CleanupWorld()
{
	// If we playing, end it
	if ( isBeginPlay )
	{
		EndPlay();
	}

	// Wait while render thread is rendering of the frame
	FlushRenderingCommands();

	// Call event of destroyed actor in each object
	for ( uint32 index = 0, count = actors.size(); index < count; ++index )
	{
		actors[ index ]->Destroyed();
	}

	// Broadcast event of destroyed actors
#if WITH_EDITOR
	if ( !actors.empty() )
	{
		EditorDelegates::onActorsDestroyed.Broadcast( actors );
	}
#endif // WITH_EDITOR

	g_PhysicsScene.RemoveAllBodies();
	scene->Clear();
	actors.clear();
	actorsToDestroy.clear();

#if WITH_EDITOR
	bDirty		= false;
	selectedActors.clear();
	filePath	= TEXT( "" );
	name		= TEXT( "Unknown" );
#endif // WITH_EDITOR
}

/*
==================
CWorld::SpawnActor
==================
*/
ActorRef_t CWorld::SpawnActor( class CClass* InClass, const Vector& InLocation, const CRotator& InRotation /* = Math::rotatorZero */, const CName& InName /* = NAME_None */)
{
	Assert( InClass );

	AActor*		actor = InClass->CreateObject<AActor>( nullptr, InName );
	Assert( actor );

	// Set default actor location with rotation
	if ( InName == NAME_None )
	{
		actor->SetCName( InClass->GetCName() );
	}
    actor->AddActorLocation( InLocation );
    actor->AddActorRotation( InRotation );

	// Call event of spawn actor
	actor->Spawned();

	// If gameplay is started - call BeginPlay in spawned actor and init physics
	if ( isBeginPlay )
	{
		actor->BeginPlay();
		actor->InitPhysics();
	}

	actors.push_back( actor );
	
	// Broadcast event of spawned actor
#if WITH_EDITOR
	std::vector<ActorRef_t>		spawnedActors = { actor };
	EditorDelegates::onActorsSpawned.Broadcast( spawnedActors );
	MarkDirty();
#endif // WITH_EDITOR

	return actor;
}

/*
==================
CWorld::DestroyActor
==================
*/
void CWorld::DestroyActor( ActorRef_t InActor, bool InIsIgnorePlaying )
{
	Assert( InActor );

	// If actor allready penging kill, exit from method
	if ( InActor->IsPendingKill() )
	{
		return;
	}

	// If world in play, put this actor to actorsToDestroy for remove after tick
	if ( !InIsIgnorePlaying && InActor->IsPlaying() )
	{
		actorsToDestroy.push_back( InActor );
		return;
	}

	// Destroy actor

	// Broadcast event of destroy actor
#if WITH_EDITOR
	// Unselect actor if him is selected
	if ( InActor->IsSelected() )
	{
		UnselectActor( InActor );
	}

	std::vector<ActorRef_t>		destroyedActors = { InActor };
	EditorDelegates::onActorsDestroyed.Broadcast( destroyedActors );
	MarkDirty();
#endif // WITH_EDITOR

	// Call events of destroyed actor
	InActor->Destroyed();

	// Remove actor from array of all actors in world
	for ( uint32 index = 0, count = actors.size(); index < count; ++index )
	{
		if ( actors[ index ] == InActor )
		{
			actors.erase( actors.begin() + index );
			break;
		}
	}
}

#if ENABLE_HITPROXY
/*
==================
CWorld::UpdateHitProxiesId
==================
*/
void CWorld::UpdateHitProxiesId()
{
	for ( uint32 index = 0, count = actors.size(); index < count; ++index )
	{
		actors[ index ]->SetHitProxyId( index+1 );
	}
}
#endif // ENABLE_HITPROXY

#if WITH_EDITOR
/*
==================
CWorld::SelectActor
==================
*/
void CWorld::SelectActor( ActorRef_t InActor )
{
	Assert( InActor );
	if ( InActor->IsSelected() )
	{
		return;
	}

	InActor->SetSelected( true );
	selectedActors.push_back( InActor );

	EditorDelegates::onActorsSelected.Broadcast( std::vector<ActorRef_t>{ InActor } );
}

/*
==================
CWorld::UnselectActor
==================
*/
void CWorld::UnselectActor( ActorRef_t InActor )
{
	Assert( InActor );
	if ( !InActor->IsSelected() )
	{
		return;
	}

	InActor->SetSelected( false );
	for ( uint32 index = 0, count = selectedActors.size(); index < count; ++index )
	{
		if ( selectedActors[ index ] == InActor )
		{
			selectedActors.erase( selectedActors.begin() + index );
			break;
		}
	}

	EditorDelegates::onActorsUnselected.Broadcast( std::vector<ActorRef_t>{ InActor } );
}

/*
==================
CWorld::SelectActors
==================
*/
void CWorld::SelectActors( const std::vector<ActorRef_t>& InActors )
{
	bool		bNeedBroadcast = false;
	for ( uint32 index = 0, count = InActors.size(); index < count; ++index )
	{
		ActorRef_t		actor = InActors[index];
		if ( !actor->IsSelected() )
		{
			bNeedBroadcast = true;
			actor->SetSelected( true );
			selectedActors.push_back( actor );
		}
	}

	if ( bNeedBroadcast )
	{
		EditorDelegates::onActorsSelected.Broadcast( InActors );
	}
}

/*
==================
CWorld::UnselectActors
==================
*/
void CWorld::UnselectActors( const std::vector<ActorRef_t>& InActors )
{
	std::vector<ActorRef_t>		unselectedActors;
	for ( uint32 index = 0, count = InActors.size(); index < count; ++index )
	{
		ActorRef_t		actor = InActors[index];
		if ( actor->IsSelected() )
		{
			actor->SetSelected( false );
			unselectedActors.push_back( actor );

			// Remove actor from selected array
			// FIXME: Maybe need optimize it
			for ( uint32 indexSelected = 0, countSelected = selectedActors.size(); indexSelected < countSelected; ++indexSelected )
			{
				if ( selectedActors[indexSelected] == actor )
				{
					selectedActors.erase( selectedActors.begin() + index );
					break;
				}
			}
		}
	}

	if ( !unselectedActors.empty() )
	{
		EditorDelegates::onActorsUnselected.Broadcast( unselectedActors );
	}
}

/*
==================
CWorld::UnselectAllActors
==================
*/
void CWorld::UnselectAllActors()
{
	for ( uint32 index = 0, count = selectedActors.size(); index < count; ++index )
	{
		selectedActors[ index ]->SetSelected( false );
	}

	std::vector<ActorRef_t>		unselectedActors;
	std::swap( selectedActors, unselectedActors );
	EditorDelegates::onActorsUnselected.Broadcast( unselectedActors );
}
#endif // WITH_EDITOR