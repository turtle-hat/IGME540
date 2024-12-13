#include "ShaderStructs.hlsli"

Texture2D BaseRender		: register(t0);
SamplerState ClampSampler	: register(s0);

float4 main(VertexToPixel_PostProcess input) : SV_TARGET
{
	float4 pixelColor = BaseRender.Sample(ClampSampler, input.uv);

	return float4(-pixelColor.xyz, pixelColor.a);
}