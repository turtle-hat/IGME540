#include "Material.h"

#include <algorithm>

/// <summary>
/// Constructs a Material with the given shaders and tint
/// </summary>
/// <param name="_name">The internal name for the Material</param>
/// <param name="_vertexShader">The vertex shader to draw the Material with</param>
/// <param name="_pixelShader">The pixel shader to draw the Material with</param>
/// <param name="_colorTint">An RGBA color to multiply to the Material</param>
Material::Material(const char* _name, std::shared_ptr<SimpleVertexShader> _vertexShader, std::shared_ptr<SimplePixelShader> _pixelShader, DirectX::XMFLOAT4 _colorTint, float _roughness)
{
	name = _name;
	vertexShader = _vertexShader;
	pixelShader = _pixelShader;
	colorTint = _colorTint;
	roughness = std::clamp(_roughness, 0.0f, 1.0f);
}

/// <summary>
/// Gets the Vertex Shader the Material is using
/// </summary>
/// <returns>The Material's Vertex Shader</returns>
std::shared_ptr<SimpleVertexShader> Material::GetVertexShader()
{
	return vertexShader;
}

/// <summary>
/// Gets the Pixel Shader the Material is using
/// </summary>
/// <returns>The Material's Pixel Shader</returns>
std::shared_ptr<SimplePixelShader> Material::GetPixelShader()
{
	return pixelShader;
}

/// <summary>
/// Gets the Material's color tint
/// </summary>
/// <returns>The Material's RGBA color tint</returns>
DirectX::XMFLOAT4 Material::GetColorTint()
{
	return colorTint;
}

/// <summary>
/// Gets the Material's roughness
/// </summary>
/// <returns>The Material's roughness value</returns>
float Material::GetRoughness()
{
	return roughness;
}

/// <summary>
/// Gets the Material's internal name
/// </summary>
/// <returns>The Material's internal name</returns>
const char* Material::GetName()
{
	return name;
}

/// <summary>
/// Sets the Vertex Shader for the Material to use
/// </summary>
/// <param name="_vertexShader">A pointer to the Material's new Vertex Shader</param>
void Material::SetVertexShader(std::shared_ptr<SimpleVertexShader> _vertexShader)
{
	vertexShader = _vertexShader;
}

/// <summary>
/// Sets the Pixel Shader for the Material to use
/// </summary>
/// <param name="_pixelShader">A pointer to the Material's new Pixel Shader</param>
void Material::SetPixelShader(std::shared_ptr<SimplePixelShader> _pixelShader)
{
	pixelShader = _pixelShader;
}

/// <summary>
/// Sets the Material's color tint
/// </summary>
/// <param name="_colorTint">A new RGBA color to multiply to the Material</param>
void Material::SetColorTint(DirectX::XMFLOAT4 _colorTint)
{
	colorTint = _colorTint;
}

/// <summary>
/// Sets the Material's roughness
/// </summary>
/// <param name="_roughness">A new roughness for the Material in the range 0-1</param>
void Material::SetRoughness(float _roughness)
{
	roughness = std::clamp(_roughness, 0.0f, 1.0f);
}

/// <summary>
/// Adds the SRV for a texture to this material
/// </summary>
/// <param name="name">The name of the texture</param>
/// <param name="srv">The texture's SRV</param>
void Material::AddTextureSRV(std::string name, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv)
{
	textureSRVs.insert({ name, srv });
}

/// <summary>
/// Adds a sampler state to this material
/// </summary>
/// <param name="name">The name of the sampler</param>
/// <param name="sampler">The sampler state itself</param>
void Material::AddSampler(std::string name, Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler)
{
	samplers.insert({ name, sampler });
}

/// <summary>
/// Prepares the Material's texture SRVs and samplers for drawing
/// </summary>
void Material::PrepareMaterial()
{
	for (auto& t : textureSRVs) {
		pixelShader->SetShaderResourceView(t.first.c_str(), t.second);
	}
	for (auto& s : samplers) {
		pixelShader->SetSamplerState(s.first.c_str(), s.second);
	}
}
