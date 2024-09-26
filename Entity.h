#pragma once

#include <memory>
#include "Transform.h"
#include "Mesh.h"

class Entity
{
public:
	Entity(const char* _name, std::shared_ptr<Mesh> _mesh);
	Entity(const char* _name, std::shared_ptr<Mesh> _mesh, std::shared_ptr<Transform> _transform);

	std::shared_ptr<Mesh> GetMesh();
	std::shared_ptr<Transform> GetTransform();
	const char* GetName();

	void SetTint(DirectX::XMFLOAT4 rgba);
	void SetTint(float r, float g, float b, float a);

	void Draw(Microsoft::WRL::ComPtr<ID3D11Buffer> constBuffer);

private:
	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<Transform> transform;
	DirectX::XMFLOAT4 tint;

	// Name for UI
	const char* name;
};
