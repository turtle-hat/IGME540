// Starter code from PixelShader.hlsl
#include "ShaderStructs.hlsli"

cbuffer PrimaryBuffer : register(b0)
{
	float4 colorTint;
	float roughness;
	float3 cameraPosition;
	float3 ambientLight;
	float totalTime;
	float2 imageCenter;
	float2 zoomCenter;
	int maxIterations;
}

float4 main(VertexToPixel input) : SV_TARGET
{
	// Renders Burning Ship fractal
	// Based on pseudocode from Wikipedia https://en.wikipedia.org/wiki/Burning_Ship_fractal
	// and shader code by sixstring982 on ShaderToy https://www.shadertoy.com/view/ltG3Wm

	// Scale by time
	float scale = pow((sin(totalTime * 0.3f - 4.5f) + 1.05f) * 0.15f, 2);
	//float scale = pow((totalTime * 0.1f) % 0.51f + 0.35f, 5) * 1.5f;
	//float scale = pow((0.43) % 1.0f + 0.32f, 5) * 1.5f;

	// Pixel X and Y, scaled to fit in range
	float2 uvScaled = (input.uv + zoomCenter) * scale + imageCenter;
	
	// The Real and Imaginary parts of the final value Z
	float2 z = uvScaled;

	// Iterate for this pixel
	int iteration = 0;
	for (int i = 0; i < maxIterations; i++) {
		iteration = i;

		// Square each component of Z
		float2 zSquared = pow(z, 2);
		
		// Calculate new z point for this iteration
		z = float2(zSquared.x - zSquared.y + uvScaled.x, abs(2 * z.x * z.y) + uvScaled.y);

		// If the value starts escaping to infinity, break out of the loop
		if (zSquared.x + zSquared.y >= 4) {
			break;
		}
	}

	// How far the process got before breaking out
	float progress = (float)iteration / maxIterations;
	
	float3 finalColor = progress * colorTint.xyz;
	
	// Added gamma correction
	return float4(pow(finalColor, 1.0f / 2.2f), colorTint.w);
}
