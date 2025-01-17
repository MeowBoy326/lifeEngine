#include "Render/BoundShaderStateCache.h"
#include "RHI/BaseShaderRHI.h"

/*
==================
CBoundShaderStateKey::CBoundShaderStateKey
==================
*/
CBoundShaderStateKey::CBoundShaderStateKey( VertexDeclarationRHIParamRef_t InVertexDeclaration, VertexShaderRHIParamRef_t InVertexShader, PixelShaderRHIParamRef_t InPixelShader, HullShaderRHIParamRef_t InHullShader /*= nullptr*/, DomainShaderRHIParamRef_t InDomainShader /*= nullptr*/, GeometryShaderRHIParamRef_t InGeometryShader /*= nullptr*/ ) :
	hash( 94875494 ),
	vertexDeclaration( InVertexDeclaration ),
	vertexShader( InVertexShader ),
	pixelShader( InPixelShader ),
	hullShader( InHullShader ),
	domainShader( InDomainShader ),
	geometryShader( InGeometryShader )
{
	Assert( vertexDeclaration );

	hash = vertexDeclaration->GetHash( hash );
	hash = Sys_MemFastHash( vertexShader, hash );
	hash = Sys_MemFastHash( pixelShader, hash );
	hash = Sys_MemFastHash( hullShader, hash );
	hash = Sys_MemFastHash( domainShader, hash );
	hash = Sys_MemFastHash( geometryShader, hash );
}