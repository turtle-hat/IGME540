#include "ShaderStructs.hlsli"
#include "ShaderLighting.hlsli"

// Data from our primary constant buffer
cbuffer PrimaryBuffer : register(b0)
{
	float4 colorTint;
	float roughness;
	float3 cameraPosition;
	Light lightDirectional0;
	float3 lightAmbient;
}

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
	input.normal = normalize(input.normal);

	float3 directionToLight = -normalize(lightDirectional0.Direction);

	float4 diffuseTerm = DiffuseLambert(input.normal, colorTint, directionToLight, lightDirectional0.Color, lightDirectional0.Intensity);

	return float4(diffuseTerm.xyz + lightAmbient, 1.0f);
}
