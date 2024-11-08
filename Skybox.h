#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <memory>
#include <DirectXMath.h>

#include "SimpleShader.h"
#include "Camera.h"
#include "Mesh.h"

class Skybox
{
public:
	Skybox(
		const char* _name,
		std::shared_ptr<Mesh> _mesh,
		Microsoft::WRL::ComPtr<ID3D11SamplerState> _samplerState,
		std::shared_ptr<SimpleVertexShader> _vertexShader,
		std::shared_ptr<SimplePixelShader> _pixelShader,
		const wchar_t* right,
		const wchar_t* left,
		const wchar_t* up,
		const wchar_t* down,
		const wchar_t* front,
		const wchar_t* back
	);
	void Draw(std::shared_ptr<Camera> camera);

private:
	// Helper for creating a cubemap from 6 individual textures
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> CreateCubemap(
		const wchar_t* right,
		const wchar_t* left,
		const wchar_t* up,
		const wchar_t* down,
		const wchar_t* front,
		const wchar_t* back);

	// Sampler options
	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;
	// Cube map texture's SRV
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
	// Depth buffer comparison type
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthState;
	// Switches to draw object's inside faces
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState;

	// Internal name for ImGui
	const char* name;

	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<SimpleVertexShader> vertexShader;
	std::shared_ptr<SimplePixelShader> pixelShader;
};

