#include "ShaderStructs.hlsli"

cbuffer PrimaryBuffer : register(b0)
{
	matrix world;
	matrix view;
	matrix projection;
}

float4 main(VertexShaderInput input) : SV_POSITION
{
	return mul(
		mul(projection, mul(view, world)),
		float4(input.localPosition, 1.0f)
	);
}