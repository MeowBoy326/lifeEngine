#include "Misc/EngineGlobals.h"
#include "Misc/WorldEdGlobals.h"
#include "Misc/Template.h"
#include "System/World.h"
#include "System/EditorEngine.h"
#include "Render/EditorCommonDrawHelper.h"
#include "Render/Scene.h"
#include "Render/RenderUtils.h"
#include "EngineDefines.h"

/*
==================
CEditorCommonDrawHelper::CEditorCommonDrawHelper
==================
*/
CEditorCommonDrawHelper::CEditorCommonDrawHelper()
	: bDrawColoredOrigin( true )
	, bDrawWorldBox( false )
	, perspectiveGridSize( HALF_WORLD_MAX1 )
	, gridColorHi( 127, 127, 127 )
	, gridColorLo( 63, 63, 63 )
	, worldBoxColor( 40, 40, 40 )
{}

/*
==================
CEditorCommonDrawHelper::DrawGridSection
==================
*/
void CEditorCommonDrawHelper::DrawGridSection( int32 InViewportLocX, int32 InViewportGridY, Vector& InStart, Vector& InEnd, float& InStartX, float& InEndX, int32 InAxis, bool InIsAlphaCase, const class CSceneView* InSceneView, struct SceneDepthGroup& InSDG )
{
	if ( !InViewportGridY )
	{
		return;
	}
	InViewportGridY = Max<int32>( InViewportGridY, 5 );

	const Matrix&				projectionMatrix = InSceneView->GetProjectionMatrix();
	Matrix	invViewProjMatrix	= Math::InverseMatrix( projectionMatrix ) * Math::InverseMatrix( InSceneView->GetViewMatrix() );
	int32	start				= Math::Trunc( ( Vector4D( -1.f, -1.f, -1.f, 1.f ) * invViewProjMatrix )[ InAxis ] / InViewportGridY ) + InViewportLocX;
	int32	end					= Math::Trunc( ( Vector4D( +1.f, +1.f, +1.f, 1.f ) * invViewProjMatrix )[ InAxis ] / InViewportGridY ) - InViewportLocX;
	if ( start > end )
	{
		Swap( start, end );
	}

	float	sizeX		= InSceneView->GetSizeX();
	float	zoom		= ( 1.0f / projectionMatrix[ 0 ].x ) * 2.0f / sizeX;
	int32   dist		= Math::Trunc( sizeX * zoom / InViewportGridY );

	// Figure out alpha interpolator for fading in the grid lines.
	float	alpha		= 0.f;
	int32	incBits		= 0;
	if ( dist + dist >= sizeX / 4 )
	{
		while ( ( dist >> incBits ) >= sizeX / 4 )
		{
			incBits++;
		}
		alpha			= 2.f - 2.f * ( float )dist / ( float )( ( 1 << incBits ) * sizeX / 4.f );
	}
	else
	{
		alpha			= 1.f;
	}

	int32	iStart		= Max<int32>( start - 1, -HALF_WORLD_MAX / InViewportGridY ) >> incBits;
	int32	iEnd		= Min<int32>( end + 1, +HALF_WORLD_MAX / InViewportGridY ) >> incBits;
	for ( int32 index = iStart; index < iEnd; ++index )
	{
		InStartX		= ( index * InViewportGridY ) << incBits;
		InEndX			= ( index * InViewportGridY ) << incBits;
		if ( ( index & 1 ) != InIsAlphaCase )
		{
			Vector4D		background( InSceneView->GetBackgroundColor().ToNormalizedVector4D() );
			Vector4D		grid( 0.25f, 0.25f, 0.25f, 0.f );
			Vector4D		color = background + ( grid - background ) * ( float )( ( ( index << incBits ) & 7 ) ? 0.25f : 1.f );
			if ( index & 1 )
			{
				color		= background + ( color - background ) * alpha;
			}

			InSDG.simpleElements.AddLine( InStart, InEnd, CColor( color ) );
		}
	}
}

/*
==================
CEditorCommonDrawHelper::DrawGrid
==================
*/
void CEditorCommonDrawHelper::DrawGrid( const class CSceneView* InSceneView, ELevelViewportType InViewportType, class CScene* InScene )
{
	Assert( InScene );
	SceneDepthGroup&		SDG = InScene->GetSDG( SDG_WorldEdBackground );
	Vector					origin;
	Vector					a( 0.f, 0.f, 0.f );			// Start line
	Vector					b( 0.f, 0.f, 0.f );			// End line
	Math::GetOriginMatrix( Math::InverseMatrix( InSceneView->GetViewMatrix() ), origin );

	// Draw 3D perspective grid
	if ( InViewportType == LVT_Perspective )
	{
		// Index of middle line (axis)
		uint32		midleLine = ( 63 - 1 ) / 2;

		// Draw grid
		for ( uint32 index = 0; index < 63; index++ )
		{
			if ( index == midleLine )
			{
				continue;
			}

			a.x = ( perspectiveGridSize / 4.f ) * ( -1.0 + 2.0 * index / ( 63 - 1 ) );			b.x = a.x;
			a.z = ( perspectiveGridSize / 4.f );												b.z = -( perspectiveGridSize / 4.f );
			a.y = 0.f;																			b.y = 0.f;
			SDG.simpleElements.AddLine( a, b, ( index == midleLine ) ? gridColorHi : gridColorLo );

			a.z = a.x;									b.z = b.x;
			a.x = ( perspectiveGridSize / 4.f );		b.x = -( perspectiveGridSize / 4.f );
			SDG.simpleElements.AddLine( a, b, ( index == midleLine ) ? gridColorHi : gridColorLo );
		}
	}

	// Draw ortho grid
	else
	{
		for ( int32 alphaCase = 0; alphaCase <= 1; ++alphaCase )
		{
			switch ( InViewportType )
			{
			case LVT_OrthoXZ:
				// Do X-Axis lines.
				a.x = +HALF_WORLD_MAX1;			a.y = 0.0;
				b.x = -HALF_WORLD_MAX1;			b.y = 0.0;
				DrawGridSection( origin.z, g_EditorEngine->GetConstraints().GetGridTranslationSize(), a, b, a.z, b.z, 2, alphaCase, InSceneView, SDG );
				
				// Do Z-Axis lines.
				a.z = +HALF_WORLD_MAX1;			a.y = 0.f;
				b.z = -HALF_WORLD_MAX1;			b.y = 0.f;
				DrawGridSection( origin.x, g_EditorEngine->GetConstraints().GetGridTranslationSize(), a, b, a.x, b.x, 0, alphaCase, InSceneView, SDG );
				break;

			case LVT_OrthoXY:
				// Do X-Axis lines.
				a.x = +HALF_WORLD_MAX1;			a.z = 0.f;
				b.x = -HALF_WORLD_MAX1;			b.z = 0.f;
				DrawGridSection( origin.y, g_EditorEngine->GetConstraints().GetGridTranslationSize(), a, b, a.y, b.y, 1, alphaCase, InSceneView, SDG );
				
				// Do Y-Axis lines.
				a.y = +HALF_WORLD_MAX1;			a.z = 0.f;
				b.y = -HALF_WORLD_MAX1;			b.z = 0.f;
				DrawGridSection( origin.x, g_EditorEngine->GetConstraints().GetGridTranslationSize(), a, b, a.x, b.x, 0, alphaCase, InSceneView, SDG );
				break;

			case LVT_OrthoYZ:
				// Do Y-Axis lines.
				a.y = +HALF_WORLD_MAX1;			a.x = 0.f;
				b.y = -HALF_WORLD_MAX1;			b.x = 0.f;
				DrawGridSection( origin.z, g_EditorEngine->GetConstraints().GetGridTranslationSize(), a, b, a.z, b.z, 2, alphaCase, InSceneView, SDG );

				// Do Z-Axis lines.
				a.z = +HALF_WORLD_MAX1;			a.x = 0.f;
				b.z = -HALF_WORLD_MAX1;			b.x = 0.f;
				DrawGridSection( origin.y, g_EditorEngine->GetConstraints().GetGridTranslationSize(), a, b, a.y, b.y, 1, alphaCase, InSceneView, SDG );
				break;
			}
		}
	}

	// Draw axis lines
	if ( bDrawColoredOrigin )
	{
		a = Vector( 0.f, 0.f, ( float )HALF_WORLD_MAX1 );			b = Vector( 0.f, 0.f, 0.f );
		SDG.simpleElements.AddLine( a, b, CColor( 64, 255, 64 ) );

		a = Vector( 0.f, 0.f, 0.f );								b = Vector( 0.f, 0.f, ( float )-HALF_WORLD_MAX1 );
		SDG.simpleElements.AddLine( a, b, CColor( 32, 128, 32 ) );

		a = Vector( 0.f, ( float )HALF_WORLD_MAX1, 0.f );			b = Vector( 0.f, 0.f, 0.f );
		SDG.simpleElements.AddLine( a, b, CColor( 64, 64, 255 ) );

		a = Vector( 0.f, 0.f, 0.f );								b = Vector( 0.f, ( float )-HALF_WORLD_MAX1, 0.f );
		SDG.simpleElements.AddLine( a, b, CColor( 32, 32, 128 ) );

		a = Vector( ( float )HALF_WORLD_MAX1, 0.f, 0.f );			b = Vector( 0.f, 0.f, 0.f );
		SDG.simpleElements.AddLine( a, b, CColor( 255, 64, 64 ) );

		a = Vector( 0.f, 0.f, 0.f );								b = Vector( ( float )-HALF_WORLD_MAX1, 0.f, 0.f );
		SDG.simpleElements.AddLine( a, b, CColor( 128, 32, 32 ) );
	}

	// Draw orthogonal worldframe
	if ( bDrawWorldBox )
	{
		DrawWireframeBox( SDG, CBox( Vector( -HALF_WORLD_MAX1, -HALF_WORLD_MAX1, -HALF_WORLD_MAX1 ), Vector( HALF_WORLD_MAX1, HALF_WORLD_MAX1, HALF_WORLD_MAX1 ) ), worldBoxColor );
	}
}