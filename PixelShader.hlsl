#include "ShaderStructs.hlsli"
#include "ShaderLighting.hlsli"

#define LIGHT_COUNT	5

// Data from our primary constant buffer
cbuffer PrimaryBuffer : register(b0)
{
	float4 colorTint;
	float roughness;
	float3 cameraPosition;
	float3 lightAmbient;

	Light lights[LIGHT_COUNT];
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

	// Start accumulator with ambient light value
	float3 lightsFinal = lightAmbient;

	// For each light in the scene
	for (int i = 0; i < LIGHT_COUNT; i++) {
		switch (lights[i].Type) {
			case LIGHT_TYPE_DIRECTIONAL:
				lightsFinal += LightDirectional(normalize(lights[i].Direction), lights[i].Color, input.normal, colorTint, roughness, input.worldPosition, cameraPosition);
				break;
			case LIGHT_TYPE_POINT:
				lightsFinal += LightDirectional(normalize(lights[i].Direction), lights[i].Color, input.normal, colorTint, roughness, input.worldPosition, cameraPosition);
				break;

		}
	}

	return float4(lightsFinal, 1.0f);
}
