#include "ShaderStructs.hlsli"

cbuffer PrimaryBuffer : register(b0) {
	float3 colorLight;
	int ditherPixelSize;

	float3 colorDark;
	float bias;

	float3 cameraRotation;
	float cameraFov;
	
	float2 pixelSize;
	float2 screenSize;
}

Texture2D BaseRender : register(t0);
Texture2D MapDither : register(t1);
SamplerState ClampSampler : register(s0);
SamplerState DitherMapSampler : register(s1);

float4 main(VertexToPixel_PostProcess input) : SV_TARGET
{
	// Quantize UV for base render and sample
	float2 baseUV = floor(input.uv / pixelSize / ditherPixelSize) * pixelSize * ditherPixelSize;
	float4 sampleBase = BaseRender.Sample(DitherMapSampler, baseUV);
	
	float2 ditherOffset = floor(screenSize * cameraRotation.yx / cameraFov / ditherPixelSize) / 256.0f;
	float2 ditherUV = input.uv / (pixelSize * 256.0f) / ditherPixelSize;

	

	float4 sampleDitherMap = MapDither.Sample(DitherMapSampler, ditherUV + ditherOffset);

	//return sampleDitherMap;

	float baseGrey = (sampleBase.x + sampleBase.y + sampleBase.z) / 3.0f;
	float isLight = saturate(ceil(baseGrey + bias - sampleDitherMap.r));

	// Return average color
	return float4(
		(isLight * colorLight) +
		((1.0f - isLight) * colorDark),
	1.0f);
}