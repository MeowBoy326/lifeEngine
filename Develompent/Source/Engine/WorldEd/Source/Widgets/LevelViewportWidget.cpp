#include <QMenu>

#include "Misc/EngineGlobals.h"
#include "Misc/WorldEdGlobals.h"
#include "System/World.h"
#include "System/EditorEngine.h"
#include "System/Package.h"
#include "System/ActorFactory.h"
#include "Widgets/LevelViewportWidget.h"

#include "Windows/MainWindow.h"

// Widgets
#include "Widgets/ActorClassesWidget.h"
#include "Widgets/ContentBrowserWidget.h"

WeLevelViewportWidget::WeLevelViewportWidget( QWidget* InParent /*= nullptr*/, CEditorLevelViewportClient* InViewportClient /*= nullptr*/, bool InDeleteViewportClient /*= false*/ )
	: WeViewportWidget( InParent, InViewportClient, InDeleteViewportClient )
{
	setContextMenuPolicy( Qt::CustomContextMenu );

	// Connect to slots
	connect( this, SIGNAL( customContextMenuRequested( const QPoint& ) ), this, SLOT( OnCustomContextMenuRequested( const QPoint& ) ) );
}

WeLevelViewportWidget::~WeLevelViewportWidget()
{}

void WeLevelViewportWidget::OnCustomContextMenuRequested( const QPoint& InPoint )
{
	// Save cursor position for spawn objects on this place
	contextMenuCursorPosition = mapFromGlobal( cursor().pos() );

	// Check if we allow open context menu or no
	CEditorLevelViewportClient*		viewportClient = ( CEditorLevelViewportClient* )GetViewport().GetViewportClient();
	check( viewportClient );
	if ( !viewportClient->IsAllowContextMenu() )
	{
		return;
	}

	// If yes we getting current actor class and build menu
	CClass*				currentActorClass = GEditorEngine->GetMainWindow()->GetActorClassesWidget()->GetSelectedClass();
	check( currentActorClass );

	// Get asset name for showing in context menu
	std::wstring		assetReference = GEditorEngine->GetMainWindow()->GetContentBrowserWidget()->GetSelectedAssetReference();

	// Parse reference for check asset type. If not supported for spawning, we not showing this action
	{
		std::wstring		packageName;
		std::wstring		assetName;
		EAssetType			assetType;
		if ( !ParseReferenceToAsset( assetReference, packageName, assetName, assetType ) || !GActorFactory.IsRegistered( assetType ) )
		{
			assetReference = TEXT( "" );
		}
	}

	// Main menu
	QMenu			contextMenu( this );
	QAction			actionActorAdd( QString::fromStdWString( �String::Format( TEXT( "Add %s Here" ), currentActorClass->GetName().c_str() ) ), this );
	QAction			actionAssetAdd( QString::fromStdWString( �String::Format( TEXT( "Add %s Here" ), assetReference.c_str() ) ), this );
	contextMenu.addAction( &actionActorAdd );
	
	// If asset reference not empty, we add action for spawn asset in world
	if ( !assetReference.empty() )
	{
		contextMenu.addAction( &actionAssetAdd );
	}

	// Connect to slots
	connect( &actionActorAdd, SIGNAL( triggered() ), this, SLOT( OnActorAdd() ) );
	connect( &actionAssetAdd, SIGNAL( triggered() ), this, SLOT( OnAssetAdd() ) );

	// Execute him
	contextMenu.exec( QCursor::pos() );
}

void WeLevelViewportWidget::OnActorAdd()
{
	// Getting viewport client
	CEditorLevelViewportClient*		viewportClient = ( CEditorLevelViewportClient* )GetViewport().GetViewportClient();
	check( viewportClient );

	// Convert from screen space to world space
	Vector		location = viewportClient->ScreenToWorld( Vector2D( contextMenuCursorPosition.x(), contextMenuCursorPosition.y() ), width(), height() );

	// Spawn new actor
	GWorld->SpawnActor( GEditorEngine->GetMainWindow()->GetActorClassesWidget()->GetSelectedClass(), location );
}

void WeLevelViewportWidget::OnAssetAdd()
{
	// Getting asset reference
	std::wstring		assetReference = GEditorEngine->GetMainWindow()->GetContentBrowserWidget()->GetSelectedAssetReference();

	// Parse reference for get asset type. If failed in parsing, do nothing
	std::wstring		packageName;
	std::wstring		assetName;
	EAssetType			assetType;
	if ( !ParseReferenceToAsset( assetReference, packageName, assetName, assetType ) )
	{
		return;
	}

	// Find asset. If failed, we exit from method
	TAssetHandle<CAsset>		asset = GPackageManager->FindAsset( assetReference, assetType );
	if ( !asset.IsAssetValid() )
	{
		return;
	}

	// Getting viewport client
	CEditorLevelViewportClient*		viewportClient = ( CEditorLevelViewportClient* )GetViewport().GetViewportClient();
	check( viewportClient );

	// Convert from screen space to world space
	Vector		location = viewportClient->ScreenToWorld( Vector2D( contextMenuCursorPosition.x(), contextMenuCursorPosition.y() ), width(), height() );

	// Spawn new actor
	GActorFactory.Spawn( asset, location );
}