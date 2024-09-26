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

	void Draw();

private:
	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<Transform> transform;

	// Name for UI
	const char* name;
};
