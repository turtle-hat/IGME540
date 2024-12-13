#include "ShaderStructs.hlsli"

cbuffer PrimaryBuffer : register(b0) {

}

Texture2D BaseRender		: register(t0);
SamplerState ClampSampler	: register(s0);

float4 main(VertexToPixel_PostProcess input) : SV_TARGET
{
	float4 pixelColor = BaseRender.Sample(ClampSampler, input.uv);

	return float4(1.0f - pixelColor.xyz, pixelColor.a);
}