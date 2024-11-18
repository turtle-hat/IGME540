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
	// Added gamma correction
	return float4(pow(input.uv, 1.0f / 2.2f), 0, 1);
}
