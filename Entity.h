#pragma once

#include <memory>
#include "Transform.h"
#include "Mesh.h"
#include "Material.h"
#include "Camera.h"

class Entity
{
public:
	// Constructor
	Entity(const char* _name, std::shared_ptr<Mesh> _mesh, std::shared_ptr<Material> _material);
	Entity(const char* _name, std::shared_ptr<Mesh> _mesh, std::shared_ptr<Material> _material, std::shared_ptr<Transform> _transform);

	// Getters
	std::shared_ptr<Mesh> GetMesh();
	std::shared_ptr<Material> GetMaterial();
	std::shared_ptr<Transform> GetTransform();
	const char* GetName();

	// Setters
	void SetMaterial(std::shared_ptr<Material> _material);

	void Draw(std::shared_ptr<Camera> _camera);

private:
	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<Material> material;
	std::shared_ptr<Transform> transform;

	// Name for UI
	const char* name;
};
