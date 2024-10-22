// Starter code from PixelShader.hlsl
#include "ShaderStructs.hlsli"

cbuffer PrimaryBuffer : register(b0)
{
	float4 colorTint;
	float roughness;
	float3 cameraPosition;
	float3 ambientLight;
}

float4 main(VertexToPixel input) : SV_TARGET
{
	return float4(input.uv, 0, 1);
}
