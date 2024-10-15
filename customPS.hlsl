// Starter code from PixelShader.hlsl

struct VertexToPixel
{
	float4 screenPosition	: SV_POSITION;
	float3 normal			: NORMAL;
	float2 uv				: TEXCOORD;
};

cbuffer PrimaryBuffer : register(b0)
{
	float4 colorTint;
	float totalTime;
	int maxIterations;
}

float4 main(VertexToPixel input) : SV_TARGET
{
	// Renders Burning Ship fractal
	// Based on pseudocode from Wikipedia https://en.wikipedia.org/wiki/Burning_Ship_fractal
	// and shader code by sixstring982 on ShaderToy https://www.shadertoy.com/view/ltG3Wm

	// Pixel X and Y, scaled to fit in range
	float2 uvScaled = float2(
		input.uv.x * 2.0f - 1.4f,
		input.uv.y * 2.0f - 1.4f);
	
	// The Real and Imaginary parts of the final value Z
	float2 z = uvScaled;

	// Iterate for this pixel
	int iteration = 0;
	for (int i = 1; i <= maxIterations; i++) {
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
	
	return float4(finalColor, 1);
}
