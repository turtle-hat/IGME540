#include "ShaderStructs.hlsli"

// Data from our primary constant buffer
cbuffer PrimaryBuffer : register(b0)
{
	float4x4 tfWorld;
	float4x4 tfView;
	float4x4 tfProjection;
	float4x4 tfWorldIT;
}

VertexToPixel_Normal main(VertexShaderInput input)
{
	// Set up output struct
	VertexToPixel_Normal output;

	// Build wvp matrix and find the screen position
	matrix wvp = mul(tfProjection, mul(tfView, tfWorld));
	output.screenPosition = mul(wvp, float4(input.localPosition, 1.0f));
	// Send other data through the pipeline
	output.normal = mul((float3x3)tfWorldIT, input.normal);
	output.tangent = mul((float3x3)tfWorld, input.tangent);
	output.uv = input.uv;
	output.worldPosition = mul(tfWorld, float4(input.localPosition, 1)).xyz;

	// Whatever we return will make its way through the pipeline to the
	// next programmable stage we're using (the pixel shader for now)
	return output;
}