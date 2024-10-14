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
	float totalTime;
}

float4 main(VertexToPixel input) : SV_TARGET
{
	return float4(1.0f, sin(totalTime), 1.0f, 1.0f);
}
