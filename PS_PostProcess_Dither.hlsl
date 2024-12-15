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
	float4 sampleBase = BaseRender.Sample(ClampSampler, input.uv);
	float4 sampleDitherMap = MapDither.Sample(DitherMapSampler, input.uv * pixelSize);

	// Return average color
	return sampleBase - sampleDitherMap;
}