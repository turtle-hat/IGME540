#include "ShaderStructs.hlsli"
#include "ShaderLighting.hlsli"

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
Texture2D MapNormal : register(t0); // "t" registers for textures

SamplerState BasicSampler : register(s0); // "s" registers for samplers

// --------------------------------------------------------
// The entry point (main method) for our pixel shader
// 
// - Input is the data coming down the pipeline (defined by the struct)
// - Output is a single color (float4)
// - Has a special semantic (SV_TARGET), which means 
//    "put the output of this into the current render target"
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
{
	// Renormalize the normal
	input.normal = normalize(input.normal);

// Sample the texture at this pixel
float4 sampleDS = MapDiffuse.Sample(BasicSampler, input.uv * uvScale + uvPosition);
float3 sampleDiffuse = sampleDS.rgb;
float sampleSpecular = sampleDS.a;
float3 surfaceColor = sampleDiffuse * colorTint.rgb;

// Return the result of our lighting equations
return float4(
	CalculateLightingLambertPhong(
		lights,
		lightAmbient,
		input.normal,
		surfaceColor,
		roughness,
		sampleSpecular,
		input.worldPosition,
		cameraPosition
	),
	1.0f
);
}
