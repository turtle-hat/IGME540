#pragma once

#include <memory>
#include "Transform.h"
#include "Mesh.h"
#include "Camera.h"

class Entity
{
public:
	// Constructor
	Entity(const char* _name, std::shared_ptr<Mesh> _mesh);
	Entity(const char* _name, std::shared_ptr<Mesh> _mesh, std::shared_ptr<Transform> _transform);

	// Getters
	std::shared_ptr<Mesh> GetMesh();
	std::shared_ptr<Transform> GetTransform();
	const char* GetName();
	DirectX::XMFLOAT4 GetTint();

	// Setters
	void SetTint(DirectX::XMFLOAT4 _rgba);
	void SetTint(float _r, float _g, float _b, float _a);

	void Draw(Microsoft::WRL::ComPtr<ID3D11Buffer> constBuffer, std::shared_ptr<Camera> _camera);

private:
	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<Transform> transform;
	DirectX::XMFLOAT4 tint;

	// Name for UI
	const char* name;
};
