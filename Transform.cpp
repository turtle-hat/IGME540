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
	if (areMatricesDirty) {
		RebuildMatrices();
	}
	return world;
}

DirectX::XMFLOAT4X4 Transform::GetWorldInverseTranspose()
{
	if (areMatricesDirty) {
		RebuildMatrices();
	}
	return worldInverseTranspose;
}

void Transform::SetPosition(float x, float y, float z)
{
	position = XMFLOAT3(x, y, z);
	areMatricesDirty = true;
}

void Transform::SetPosition(DirectX::XMFLOAT3 xyz)
{
	position = xyz;
	areMatricesDirty = true;
}

void Transform::SetRotation(float pitch, float yaw, float roll)
{
	rotation = XMFLOAT3(pitch, yaw, roll);
	areMatricesDirty = true;
}

void Transform::SetRotation(DirectX::XMFLOAT3 pitchYawRoll)
{
	rotation = pitchYawRoll;
	areMatricesDirty = true;
}

void Transform::SetScale(float x, float y, float z)
{
	scale = XMFLOAT3(x, y, z);
	areMatricesDirty = true;
}

void Transform::SetScale(DirectX::XMFLOAT3 xyz)
{
	scale = xyz;
	areMatricesDirty = true;
}

void Transform::MoveAbsolute(float x, float y, float z)
{
	// Build translation vector and pass to overload
	XMFLOAT3 translation(x, y, z);
	MoveAbsolute(translation);
}

void Transform::MoveAbsolute(DirectX::XMFLOAT3 xyz)
{
	// Add translation vector to position and store the result back
	XMStoreFloat3(&position, XMLoadFloat3(&position) + XMLoadFloat3(&xyz));
	areMatricesDirty = true;
}

void Transform::Rotate(float pitch, float yaw, float roll)
{
	// Build rotation vector and pass to overload
	XMFLOAT3 rotation(pitch, yaw, roll);
	Rotate(rotation);
}

void Transform::Rotate(DirectX::XMFLOAT3 pitchYawRoll)
{
	// Add rotation vector to rotation and store the result back
	// (I'm not worrying about gimbal lock)
	XMStoreFloat3(&rotation, XMLoadFloat3(&rotation) + XMLoadFloat3(&pitchYawRoll));
	areMatricesDirty = true;
}

void Transform::Scale(float x, float y, float z)
{
	// Because DirectXMath doesn't seem to include element-wise multiplication,
	// scaling multiplication isn't performed SIMD
	scale = XMFLOAT3(
		scale.x * x,
		scale.y * y,
		scale.z * z
	);
	areMatricesDirty = true;
}

void Transform::Scale(DirectX::XMFLOAT3 xyz)
{
	// Probably more performant to repeat code here
	// rather than make new function call(?)
	scale = XMFLOAT3(
		scale.x * xyz.x,
		scale.y * xyz.y,
		scale.z * xyz.z
	);
	areMatricesDirty = true;
}

void Transform::RebuildMatrices()
{
	// Multiply Scale to Rotation to Translation to calculate the new World matrix
	XMMATRIX newWorld =
		XMMatrixScalingFromVector(XMLoadFloat3(&scale)) *
		XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&rotation)) *
		XMMatrixTranslationFromVector(XMLoadFloat3(&position));
	
	// Store World matrix
	XMStoreFloat4x4(&world, newWorld);

	// Transpose and invert World matrix to calculate World Inverse Transpose and store it
	XMStoreFloat4x4(&worldInverseTranspose, 
		XMMatrixInverse(0, XMMatrixTranspose(newWorld))
	);

	// Mark matrices as clean
	areMatricesDirty = false;
}
