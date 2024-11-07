#include "ShaderStructs.hlsli"

TextureCube MapCube : register(t0); // "t" registers for textures

SamplerState BasicSampler : register(s0); // "s" registers for samplers

float4 main(VertexToPixel_Sky input) : SV_TARGET
{
	return MapCube.Sample(BasicSampler, input.sampleDirection);
}
