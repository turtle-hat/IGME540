#pragma once

#include <memory>
#include <DirectXMath.h>

#include "SimpleShader.h"

class Material
{
public:
	Material(const char* _name, std::shared_ptr<SimpleVertexShader> _vertexShader, std::shared_ptr<SimplePixelShader> _pixelShader, DirectX::XMFLOAT4 _colorTint);

	std::shared_ptr<SimpleVertexShader> GetVertexShader();
	std::shared_ptr<SimplePixelShader> GetPixelShader();
	DirectX::XMFLOAT4 GetColorTint();
	const char* GetName();

	void SetVertexShader(std::shared_ptr<SimpleVertexShader> _vertexShader);
	void SetPixelShader(std::shared_ptr<SimplePixelShader> _pixelShader);
	void SetColorTint(DirectX::XMFLOAT4 _colorTint);

private:
	std::shared_ptr<SimpleVertexShader> vertexShader;
	std::shared_ptr<SimplePixelShader> pixelShader;
	DirectX::XMFLOAT4 colorTint;

	const char* name;
};

