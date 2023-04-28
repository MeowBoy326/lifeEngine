/**
 * BasePassPixelShader.hlsl: Pixel shader code for base pass shader.
 * 
 * Copyright Broken Singularity, All Rights Reserved.
 * Authors: Yehor Pohuliaka (zombiHello)
 */

#include "Common.hlsl"
#include "BasePassCommon.hlsl"

// Output of pixel shader
struct PS_OUT
{
	float4	diffuseRoughness	: SV_TARGET0;
	float4	normalMetal			: SV_TARGET1;
	float4	emissionAO			: SV_TARGET2;
};

// Diffuse texture
Texture2D		diffuseTexture;
SamplerState	diffuseSampler;

// Normal texture
Texture2D		normalTexture;
SamplerState	normalSampler;

// Metallic texture
Texture2D		metallicTexture;
SamplerState	metallicSampler;

// Roughness texture
Texture2D		roughnessTexture;
SamplerState	roughnessSampler;

// Emission texture
Texture2D		emissionTexture;
SamplerState	emissionSampler;

// AO texture
Texture2D		aoTexture;
SamplerState	aoSampler;

void MainPS( VS_OUT In, out PS_OUT Out )
{
	float4 	diffuseColor 		= diffuseTexture.Sample( diffuseSampler, In.texCoord0 );
	if ( diffuseColor.a < 0.5f )
	{
		discard;
	}

	Out.diffuseRoughness.a		= roughnessTexture.Sample( roughnessSampler, In.texCoord0 ).r;
	Out.diffuseRoughness.rgb	= pow( diffuseColor.rgb
								#if WITH_EDITOR
									+ In.colorOverlay
								#endif // WITH_EDITOR
									, float3( 2.2f, 2.2f, 2.2f ) );

	Out.normalMetal.rgb 		= normalize( MulMatrix( normalTexture.Sample( normalSampler, In.texCoord0 ).rgb * 2.f - 1.f, In.tbnMatrix ) );
	Out.normalMetal.a			= metallicTexture.Sample( metallicSampler, In.texCoord0 ).r;

	Out.emissionAO.rgb			= emissionTexture.Sample( emissionSampler, In.texCoord0 );
	Out.emissionAO.a			= aoTexture.Sample( aoSampler, In.texCoord0 ).r;
}
