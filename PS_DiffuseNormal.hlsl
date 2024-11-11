#include "ShaderStructs.hlsli"
#include "ShaderLighting.hlsli"
#include "ShaderNormals.hlsli"

// Data from our primary constant buffer
cbuffer PrimaryBuffer : register(b0)
{
	float4 colorTint;
	float roughness;
	float3 cameraPosition;

	float2 uvPosition;
	float2 uvScale;

	Light lights[LIGHT_COUNT];
	float3 lightAmbient;
}

Texture2D MapDiffuse : register(t0); // "t" registers for textures
Texture2D MapNormal : register(t1);
TextureCube MapCube : register(t2);

SamplerState BasicSampler : register(s0); // "s" registers for samplers

float4 main(VertexToPixel_Normal input) : SV_TARGET
{
	
	// Renormalize the normal and tangent
	input.normal = normalize(input.normal);
	return MapCube.Sample(BasicSampler, input.normal);
	input.tangent = normalize(input.tangent);

	// Sample the textures at this pixel
	float4 sampleDS = MapDiffuse.Sample(BasicSampler, input.uv * uvScale + uvPosition);
	// Unpack and normalize the normal map
	float3 sampleNormalUnpacked = SampleUnpacked(MapNormal, BasicSampler, input.uv * uvScale + uvPosition);

	
	// Pull info out of the diffuse texture
	float3 sampleDiffuse = sampleDS.rgb;
	float sampleSpecular = sampleDS.a;
	float3 surfaceColor = sampleDiffuse * colorTint.rgb;

	// Return the result of our lighting equations
	return float4(
		CalculateLightingLambertPhong(
			lights,
			lightAmbient,
			NormalFromMap(input.normal, input.tangent, sampleNormalUnpacked),
			surfaceColor,
			roughness,
			sampleSpecular,
			input.worldPosition,
			cameraPosition
		),
		1.0f
	);
}
