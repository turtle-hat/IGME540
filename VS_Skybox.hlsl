#include "ShaderStructs.hlsli"

// Data from our primary constant buffer
cbuffer PrimaryBuffer : register(b0)
{
	float4x4 tfView;
	float4x4 tfProjection;
}

VertexToPixel_Sky main(VertexShaderInput input)
{
	// Set up output struct
	VertexToPixel_Sky output;

	float4x4 tfViewNoTranslation = tfView;
	tfViewNoTranslation._14 = 0;
	tfViewNoTranslation._24 = 0;
	tfViewNoTranslation._34 = 0;

	// Build View/Projection matrix and find the screen position
	matrix viewProjection = mul(tfProjection, tfViewNoTranslation);
	output.screenPosition = mul(viewProjection, float4(input.localPosition, 1.0f));
	// Sets skybox depth to 1
	output.screenPosition.z = output.screenPosition.w;
	
	// Sets the direction to sample in
	output.sampleDirection = input.localPosition;

	// Whatever we return will make its way through the pipeline to the
	// next programmable stage we're using (the pixel shader for now)
	return output;
}
