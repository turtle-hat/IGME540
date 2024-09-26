#include "Entity.h"

using namespace std;

Entity::Entity(const char* _name, shared_ptr<Mesh> _mesh)
{
    // I forget how to make this constructor call the one with more parameters :P
    name = _name;
    mesh = _mesh;
    transform = make_shared<Transform>();
}

Entity::Entity(const char* _name, shared_ptr<Mesh> _mesh, shared_ptr<Transform> _transform)
{
    name = _name;
    mesh = _mesh;
    transform = _transform;
}

std::shared_ptr<Mesh> Entity::GetMesh()
{
    return mesh;
}

std::shared_ptr<Transform> Entity::GetTransform()
{
    return transform;
}

const char* Entity::GetName()
{
    return name;
}

void Entity::Draw()
{
    // Draw the entity's mesh
    mesh->Draw();
}
