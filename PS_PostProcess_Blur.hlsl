#include "ShaderStructs.hlsli"

cbuffer PrimaryBuffer : register(b0) {
	int blurRadius;
	float2 pixelSize;
}

Texture2D BaseRender		: register(t0);
SamplerState ClampSampler	: register(s0);

float4 main(VertexToPixel_PostProcess input) : SV_TARGET
{
	// Start accumulators for averaging the color
	float4 totalColor = 0;
	int numSamples = 0;

	// Loop through box
	for (int i = -blurRadius; i <= blurRadius; i++) {
		for (int j = -blurRadius; j <= blurRadius; j++) {
			// Calculate this sample's UV
			float2 uv = input.uv + float2(i, j) * pixelSize;

			// Add this sample to the accumulators
			totalColor += BaseRender.Sample(ClampSampler, uv);
			numSamples++;
		}
	}

	// Return average color
	return totalColor / numSamples;
}