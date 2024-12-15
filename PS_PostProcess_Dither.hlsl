#include "ShaderStructs.hlsli"

cbuffer PrimaryBuffer : register(b0) {
	int ditherPixelSize;
	float2 pixelSize;
}

Texture2D BaseRender : register(t0);
Texture2D MapDither : register(t1);
SamplerState ClampSampler : register(s0);
SamplerState DitherMapSampler : register(s1);

float4 main(VertexToPixel_PostProcess input) : SV_TARGET
{
	float2 uv = (input.uv / (pixelSize * 256.0f) / ditherPixelSize);

	float4 sampleBase = BaseRender.Sample(ClampSampler, input.uv);
	float4 sampleDitherMap = MapDither.Sample(DitherMapSampler, uv);

	// Return average color
	return sampleDitherMap;
}