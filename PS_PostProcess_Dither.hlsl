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
	
	// Quantize UV for dither map
	float2 ditherUV = input.uv / (pixelSize * 256.0f) / ditherPixelSize;
	// Find offset for dither map based on camera rotation
	// The minimum of the two screen dimensions seems to work the best for scaling the rotation speed
	float2 ditherOffset = floor(min(screenSize.x, screenSize.y) * cameraRotation.yx / cameraFov / ditherPixelSize) / 256.0f;
	// Offset UV by camera rotation and sample
	float4 sampleDitherMap = MapDither.Sample(DitherMapSampler, ditherUV + ditherOffset);

	// Find the pixel's grayscale value from the average of its RGB channels
	float baseGray = (sampleBase.r + sampleBase.g + sampleBase.b) / 3.0f;
	// Bias and compare with dither map to find whether this pixel is light or dark
	float isLight = saturate(ceil(baseGray + bias - sampleDitherMap.r));

	// Return light or dark color
	return float4(
		(isLight * colorLight) +
		((1.0f - isLight) * colorDark),
	1.0f);
}