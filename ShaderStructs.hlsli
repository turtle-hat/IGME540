#ifndef __GGP_SHADER_STRUCTS__
#define __GGP_SHADER_STRUCTS__

// Struct representing a single vertex worth of data
// - This should match the vertex definition in our C++ code
// - By "match", I mean the size, order and number of members
// - The name of the struct itself is unimportant, but should be descriptive
// - Each variable must have a semantic, which defines its usage
struct VertexShaderInput
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
    float3 localPosition	: POSITION; // XYZ position
    float3 normal			: NORMAL; // Normal vector
    float2 uv				: TEXCOORD; // UV coordinate
};

// Struct representing the data we're sending down the pipeline
// - Should match our pixel shader's input (hence the name: Vertex to Pixel)
// - At a minimum, we need a piece of data defined tagged as SV_POSITION
// - The name of the struct itself is unimportant, but should be descriptive
// - Each variable must have a semantic, which defines its usage
struct VertexToPixel
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
    float4 screenPosition	: SV_POSITION; // XYZW position (System Value Position)
    float3 normal			: NORMAL; // Normal vector
    float2 uv				: TEXCOORD; // UV coordinate
    float3 worldPosition	: POSITION;
};

#endif
