#include "Transform.h"

using namespace DirectX;

/// <summary>
/// Constructs a Transformation with no translation, no rotation, normal scale
/// </summary>
Transform::Transform() :
	position(0.0f, 0.0f, 0.0f),
	rotation(0.0f, 0.0f, 0.0f),
	scale(1.0f, 1.0f, 1.0f)
{
	XMStoreFloat4x4(&world, XMMatrixIdentity());
}

DirectX::XMFLOAT3 Transform::GetPosition()
{
	return position;
}

DirectX::XMFLOAT3 Transform::GetRotation()
{
	return rotation;
}

DirectX::XMFLOAT3 Transform::GetScale()
{
	return scale;
}

DirectX::XMFLOAT4X4 Transform::GetWorld()
{
	return world;
}

DirectX::XMFLOAT4X4 Transform::GetWorldInverseTranspose()
{
	return worldInverseTranspose;
}

void Transform::SetPosition(float x, float y, float z)
{
	position = XMFLOAT3(x, y, z);
	isMatrixDirty = true;
}

void Transform::SetPosition(DirectX::XMFLOAT3 xyz)
{
	position = xyz;
	isMatrixDirty = true;
}

void Transform::SetRotation(float pitch, float yaw, float roll)
{
	rotation = XMFLOAT3(pitch, yaw, roll);
	isMatrixDirty = true;
}

void Transform::SetRotation(DirectX::XMFLOAT3 pitchYawRoll)
{
	rotation = pitchYawRoll;
	isMatrixDirty = true;
}

void Transform::SetScale(float x, float y, float z)
{
	scale = XMFLOAT3(x, y, z);
	isMatrixDirty = true;
}

void Transform::SetScale(DirectX::XMFLOAT3 xyz)
{
	scale = xyz;
	isMatrixDirty = true;
}

void Transform::MoveAbsolute(float x, float y, float z)
{
}

void Transform::MoveAbsolute(DirectX::XMFLOAT3 xyz)
{
}

void Transform::Rotate(float pitch, float yaw, float roll)
{
}

void Transform::Rotate(DirectX::XMFLOAT3 pitchYawRoll)
{
}

void Transform::Scale(float x, float y, float z)
{
}

void Transform::Scale(DirectX::XMFLOAT3 xyz)
{
}
