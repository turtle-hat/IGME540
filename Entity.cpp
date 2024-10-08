#include "Entity.h"
#include "BufferStructs.h"

using namespace std;
using namespace DirectX;

/// <summary>
/// Constructs a new Entity at XYZ (0, 0, 0)
/// </summary>
/// <param name="_name">The internal name for the Entity</param>
/// <param name="_mesh">The mesh the Entity will be drawn with</param>
Entity::Entity(const char* _name, shared_ptr<Mesh> _mesh, std::shared_ptr<Material> _material) :
    tint(1.0f, 1.0f, 1.0f, 1.0f)
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
Entity::Entity(const char* _name, shared_ptr<Mesh> _mesh, std::shared_ptr<Material> _material, shared_ptr<Transform> _transform) :
    tint(1.0f, 1.0f, 1.0f, 1.0f)
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
/// Gets the Entity's RGBA color tint
/// </summary>
/// <returns>The Entity's RGBA color tint. Values are from 0.0f to 1.0f</returns>
DirectX::XMFLOAT4 Entity::GetTint()
{
    return tint;
}

/// <summary>
/// Sets the Entity's RGBA color tint
/// </summary>
/// <param name="_rgba">The tint's RGBA values from 0.0f to 1.0f</param>
void Entity::SetTint(DirectX::XMFLOAT4 _rgba)
{
    tint = _rgba;
}

/// <summary>
/// Sets the Entity's RGBA color tint
/// </summary>
/// <param name="_r">The tint's red value from 0.0f to 1.0f</param>
/// <param name="_g">The tint's green value from 0.0f to 1.0f</param>
/// <param name="_b">The tint's blue value from 0.0f to 1.0f</param>
/// <param name="_a">The tint's alpha value from 0.0f to 1.0f</param>
void Entity::SetTint(float _r, float _g, float _b, float _a)
{
    SetTint(XMFLOAT4(_r, _g, _b, _a));
}

/// <summary>
/// Adds the entity's data to the context buffer and draws its Mesh
/// </summary>
/// <param name="_constBuffer">The constant buffer to use</param>
/// <param name="_camera">The camera being rendered from</param>
void Entity::Draw(Microsoft::WRL::ComPtr<ID3D11Buffer> _constBuffer, std::shared_ptr<Camera> _camera)
{
    // Data to be sent through the primary constant buffer
    VertexShaderData constBufferData = {};

    // FILL CONSTANT BUFFER WITH ENTITY'S DATA
    constBufferData.tfWorld = transform->GetWorld();
    constBufferData.tfView = _camera->GetViewMatrix();
    constBufferData.tfProjection = _camera->GetProjectionMatrix();
    constBufferData.colorTint = tint;

    // COPY DATA TO CONSTANT BUFFER
	// Map constant buffer's location on GPU
    D3D11_MAPPED_SUBRESOURCE mappedBuffer = {};
    Graphics::Context->Map(_constBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer);
    // Copy data to GPU memory
    memcpy(mappedBuffer.pData, &constBufferData, sizeof(constBufferData));
    // Unmap location on GPU
    Graphics::Context->Unmap(_constBuffer.Get(), 0);

    // Draw the entity's mesh
    mesh->Draw();
}
