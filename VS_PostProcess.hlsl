#include "ShaderStructs.hlsli"

VertexToPixel_PostProcess main( uint id : SV_VertexID )
{
	VertexToPixel_PostProcess output;

	// Generates (0, 0), (2, 0), and (0, 2) from id
	output.uv = float2(
		(id << 1) & 2,
		id & 2
	);

	// Generates correct vertex positions from UV coordinates
	output.position = float4(output.uv, 0, 1);
	output.position.x = output.position.x * 2 - 1;
	output.position.y = output.position.y * -2 + 1;

	return output;
}