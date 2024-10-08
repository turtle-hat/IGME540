#include "Material.h"

/// <summary>
/// Constructs a Material with the given shaders and tint
/// </summary>
/// <param name="_name">The internal name for the Material</param>
/// <param name="_vertexShader">The vertex shader to draw the Material with</param>
/// <param name="_pixelShader">The pixel shader to draw the Material with</param>
/// <param name="_colorTint">An RGBA color to multiply to the Material</param>
Material::Material(const char* _name, std::shared_ptr<SimpleVertexShader> _vertexShader, std::shared_ptr<SimplePixelShader> _pixelShader, DirectX::XMFLOAT4 _colorTint)
{
	name = _name;
	vertexShader = _vertexShader;
	pixelShader = _pixelShader;
	colorTint = _colorTint;
}

/// <summary>
/// Gets the Vertex Shader the Material is currently using
/// </summary>
/// <returns>The Material's Vertex Shader</returns>
std::shared_ptr<SimpleVertexShader> Material::GetVertexShader()
{
	return vertexShader;
}

/// <summary>
/// Gets the Pixel Shader the Material is currently using
/// </summary>
/// <returns>The Material's Pixel Shader</returns>
std::shared_ptr<SimplePixelShader> Material::GetPixelShader()
{
	return pixelShader;
}

/// <summary>
/// Gets the Material's current color tint
/// </summary>
/// <returns>The Material's RGBA color tint</returns>
DirectX::XMFLOAT4 Material::GetColorTint()
{
	return colorTint;
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
