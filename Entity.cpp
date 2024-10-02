#include "Entity.h"
#include "BufferStructs.h"

using namespace std;
using namespace DirectX;

/// <summary>
/// Constructs a new Entity at XYZ (0, 0, 0)
/// </summary>
/// <param name="_name">The internal name for the Entity</param>
/// <param name="_mesh">The mesh the Entity will be drawn with</param>
Entity::Entity(const char* _name, shared_ptr<Mesh> _mesh) :
    tint(1.0f, 1.0f, 1.0f, 1.0f)
{
    // I forget how to make this constructor call the one with more parameters :P
    name = _name;
    mesh = _mesh;
    transform = make_shared<Transform>();
}

/// <summary>
/// Constructs a new Entity
/// </summary>
/// <param name="_name">The internal name for the Entity</param>
/// <param name="_mesh">The mesh the Entity will be drawn with</param>
/// <param name="_transform">The Entity's Transform object</param>
Entity::Entity(const char* _name, shared_ptr<Mesh> _mesh, shared_ptr<Transform> _transform) :
    tint(1.0f, 1.0f, 1.0f, 1.0f)
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

DirectX::XMFLOAT4 Entity::GetTint()
{
    return tint;
}

void Entity::SetTint(DirectX::XMFLOAT4 _rgba)
{
    tint = _rgba;
}

void Entity::SetTint(float _r, float _g, float _b, float _a)
{
    SetTint(XMFLOAT4(_r, _g, _b, _a));
}

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
