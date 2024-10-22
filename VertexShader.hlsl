#include "ShaderStructs.hlsli"

// Data from our primary constant buffer
cbuffer PrimaryBuffer : register(b0)
{
	float4x4 tfWorld;
	float4x4 tfView;
	float4x4 tfProjection;
	float4x4 tfWorldIT;
}

// --------------------------------------------------------
// The entry point (main method) for our vertex shader
// 
// - Input is exactly one vertex worth of data (defined by a struct)
// - Output is a single struct of data to pass down the pipeline
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
VertexToPixel main( VertexShaderInput input )
{
	// Set up output struct
	VertexToPixel output;

	// Here we're essentially passing the input position directly through to the next
	// stage (rasterizer), though it needs to be a 4-component vector now.  
	// - To be considered within the bounds of the screen, the X and Y components 
	//   must be between -1 and 1.  
	// - The Z component must be between 0 and 1.  
	// - Each of these components is then automatically divided by the W component, 
	//   which we're leaving at 1.0 for now (this is more useful when dealing with 
	//   a perspective projection matrix, which we'll get to in the future).
	
	// Build wvp matrix and find the screen position
    matrix wvp = mul(tfProjection, mul(tfView, tfWorld));
	output.screenPosition = mul(wvp, float4(input.localPosition, 1.0f));
	// Send other data through the pipeline
	output.normal = mul((float3x3)tfWorldIT, input.normal);
	output.uv = input.uv;
	output.worldPosition = mul(tfWorld, float4(input.localPosition, 1)).xyz;

	// Whatever we return will make its way through the pipeline to the
	// next programmable stage we're using (the pixel shader for now)
	return output;
}
