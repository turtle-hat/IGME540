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

Texture2D SurfaceTexture : register(t0); // "t" registers for textures

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
	float4 sampleDS = SurfaceTexture.Sample(BasicSampler, input.uv);
	float3 sampleDiffuse = sampleDS.rgb;
	float sampleSpecular = sampleDS.a;

	float3 surfaceColor = float3(
		sampleDiffuse.x * colorTint.x,
		sampleDiffuse.y * colorTint.y,
		sampleDiffuse.z * colorTint.z
	);

	// Start accumulator with ambient light value
	float3 lightsFinal = lightAmbient;

	// For each light in the scene
	for (int i = 0; i < LIGHT_COUNT; i++) {
		// If it's active
		if (lights[i].Active) {
			// Run a different lighting equation on it depending on the type of light
			switch (lights[i].Type) {
				case LIGHT_TYPE_DIRECTIONAL:
					lightsFinal += LightDirectional(lights[i], input.normal, surfaceColor, roughness, sampleSpecular, input.worldPosition, cameraPosition);
					break;
				case LIGHT_TYPE_POINT:
					lightsFinal += LightPoint(lights[i], input.normal, surfaceColor, roughness, sampleSpecular, input.worldPosition, cameraPosition);
					break;
				case LIGHT_TYPE_SPOT:
					lightsFinal += LightSpot(lights[i], input.normal, surfaceColor, roughness, sampleSpecular, input.worldPosition, cameraPosition);
					break;
				default:
					break;
			}
		}
	}

	// Return the result of our lighting equations
	return float4(lightsFinal, 1.0f);
}
