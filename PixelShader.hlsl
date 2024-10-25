#include "ShaderStructs.hlsli"
#include "ShaderLighting.hlsli"

#define LIGHT_COUNT	8

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
		// If it's active
		if (lights[i].Active) {
			// Run a different lighting equation on it depending on the type of light
			switch (lights[i].Type) {
				case LIGHT_TYPE_DIRECTIONAL:
					lightsFinal += LightDirectional(lights[i], input.normal, colorTint.xyz, roughness, input.worldPosition, cameraPosition);
					break;
				case LIGHT_TYPE_POINT:
					lightsFinal += LightPoint(lights[i], input.normal, colorTint.xyz, roughness, input.worldPosition, cameraPosition);
					break;
				case LIGHT_TYPE_SPOT:
					lightsFinal += LightSpot(lights[i], input.normal, colorTint.xyz, roughness, input.worldPosition, cameraPosition);
					break;
				default:
					break;
			}
		}
	}

	return float4(lightsFinal, 1.0f);
}
