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
	input.tangent = normalize(input.tangent);

	// Sample the textures at this pixel
	float4 sampleDS = MapDiffuse.Sample(BasicSampler, input.uv * uvScale + uvPosition);
	// Pull info out of the diffuse/specular texture
	float3 sampleDiffuse = pow(sampleDS.rgb, 2.2f); // Gamma uncorrected so it gets the expected value after end correction
	float sampleSpecular = sampleDS.a;
	float3 surfaceColor = sampleDiffuse * colorTint.rgb;

	// Unpack and normalize the normal map
	float3 sampleNormalUnpacked = SampleUnpacked(MapNormal, BasicSampler, input.uv * uvScale + uvPosition).rgb;
	// Calculate normal from map
	float3 mapNormal = NormalFromMap(input.normal, input.tangent, sampleNormalUnpacked);

	// Get necessary vectors for sampling reflection
	float3 cameraDirectionOut = normalize(cameraPosition - input.worldPosition);
	float3 cameraReflection = reflect(-cameraDirectionOut, mapNormal);
	// Sample Cubemap with gamma uncorrected
	float3 sampleCubeReflection = pow(MapCube.Sample(BasicSampler, cameraReflection).rgb, 2.2f);

	// Color of surface with all lighting calculated
	float3 litColor = CalculateLightingLambertPhong(
		lights,
		lightAmbient,
		mapNormal,
		surfaceColor,
		roughness,
		sampleSpecular,
		input.worldPosition,
		cameraPosition
	);

	// Return the result of our lighting equations
	return float4(
		pow(CalculateReflections(
			litColor,
			sampleCubeReflection,
			mapNormal,
			cameraDirectionOut
		), 1.0f / 2.2f),
		1.0f
	);
}
