#include "Entity.h"

using namespace std;
using namespace DirectX;

/// <summary>
/// Constructs a new Entity at XYZ (0, 0, 0)
/// </summary>
/// <param name="_name">The internal name for the Entity</param>
/// <param name="_mesh">The mesh the Entity will be drawn with</param>
Entity::Entity(const char* _name, shared_ptr<Mesh> _mesh, std::shared_ptr<Material> _material)
{
    // I forget how to make this constructor call the one with more parameters :P
    name = _name;
    mesh = _mesh;
    material = _material;
    transform = make_shared<Transform>();
}

/// <summary>
/// Constructs a new Entity
/// </summary>
/// <param name="_name">The internal name for the Entity</param>
/// <param name="_mesh">The mesh the Entity will be drawn with</param>
/// <param name="_transform">The Entity's Transform object</param>
Entity::Entity(const char* _name, shared_ptr<Mesh> _mesh, std::shared_ptr<Material> _material, shared_ptr<Transform> _transform)
{
    name = _name;
    mesh = _mesh;
    material = _material;
    transform = _transform;
}

/// <summary>
/// Gets the Entity's Mesh
/// </summary>
/// <returns>The Entity's Mesh</returns>
std::shared_ptr<Mesh> Entity::GetMesh()
{
    return mesh;
}

/// <summary>
/// Gets the Entity's Material
/// </summary>
/// <returns>The Entity's Material</returns>
std::shared_ptr<Material> Entity::GetMaterial()
{
    return material;
}

/// <summary>
/// Gets the Entity's Transform
/// </summary>
/// <returns>The Entity's Transform</returns>
std::shared_ptr<Transform> Entity::GetTransform()
{
    return transform;
}

/// <summary>
/// Gets the Entity's internal name
/// </summary>
/// <returns>The Entity's internal name</returns>
const char* Entity::GetName()
{
    return name;
}

/// <summary>
/// Sets the Entity's Material
/// </summary>
/// <param name="_material">The new Material for the Entity to use</param>
void Entity::SetMaterial(std::shared_ptr<Material> _material)
{
    material = _material;
}

/// <summary>
/// Adds the entity's data to the context buffer and draws its Mesh
/// </summary>
/// <param name="_camera">The camera being rendered from</param>
void Entity::Draw(std::shared_ptr<Camera> _camera)
{
    // Get vertex shader
    std::shared_ptr<SimpleVertexShader> vs = material->GetVertexShader();
    std::shared_ptr<SimplePixelShader> ps = material->GetPixelShader();

    // Set vertex and pixel shaders
    vs->SetShader();
    ps->SetShader();

    // FILL CONSTANT BUFFERS WITH ENTITY'S DATA
    // VERTEX
    vs->SetMatrix4x4("tfWorld", transform->GetWorld());
    vs->SetMatrix4x4("tfView", _camera->GetViewMatrix());
    vs->SetMatrix4x4("tfProjection", _camera->GetProjectionMatrix());
    // PIXEL
    ps->SetFloat4("colorTint", material->GetColorTint());

    // COPY DATA TO CONSTANT BUFFERS
    vs->CopyAllBufferData();
    ps->CopyAllBufferData();

    // Draw the entity's mesh
    mesh->Draw();
}
