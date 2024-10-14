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
}

float4 main(VertexToPixel input) : SV_TARGET
{
	return float4(input.uv, 0, 1);
}
