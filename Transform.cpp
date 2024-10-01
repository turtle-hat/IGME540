#include "Transform.h"

using namespace DirectX;

/// <summary>
/// Constructs a Transformation with no translation, no rotation, normal scale
/// </summary>
Transform::Transform() :
	position(0.0f, 0.0f, 0.0f),
	rotation(0.0f, 0.0f, 0.0f),
	scale(1.0f, 1.0f, 1.0f),
	right(1.0f, 0.0f, 0.0f),
	up(0.0f, 1.0f, 0.0f),
	forward(0.0f, 0.0f, 1.0f)
{
	XMStoreFloat4x4(&world, XMMatrixIdentity());
	XMStoreFloat4x4(&worldInverseTranspose,
		XMMatrixInverse(0, XMMatrixTranspose(XMMatrixIdentity()))
	);
	areMatricesDirty = false;
	areVerticesDirty = false;
}

/// <summary>
/// Gets the Transform's position
/// </summary>
/// <returns>The Transform's x, y, and z positions</returns>
DirectX::XMFLOAT3 Transform::GetPosition()
{
	return position;
}

/// <summary>
/// Gets the Transform's rotation
/// </summary>
/// <returns>The Transform's rotations about the X, Y, and Z axes</returns>
DirectX::XMFLOAT3 Transform::GetRotation()
{
	return rotation;
}

/// <summary>
/// Gets the Transform's scale
/// </summary>
/// <returns>The Transform's scale along the x, y, and z axes</returns>
DirectX::XMFLOAT3 Transform::GetScale()
{
	return scale;
}

/// <summary>
/// Gets the Transform's world matrix.
/// Rebuilds matrices if they have been mutated
/// </summary>
/// <returns>The world matrix representing the Transform</returns>
DirectX::XMFLOAT4X4 Transform::GetWorld()
{
	if (areMatricesDirty) {
		RebuildMatrices();
	}
	return world;
}

/// <summary>
/// Sets the Transform's world inverse transpose matrix.
/// Rebuilds matrices if they have been mutated
/// </summary>
/// <returns>The transposed then inverted world matrix representing the Transform</returns>
DirectX::XMFLOAT4X4 Transform::GetWorldInverseTranspose()
{
	if (areMatricesDirty) {
		RebuildMatrices();
	}
	return worldInverseTranspose;
}

/// <summary>
/// Gets the Transform's forward vector
/// </summary>
/// <returns></returns>
DirectX::XMFLOAT3 Transform::GetForward()
{
	return forward;
}

DirectX::XMFLOAT3 Transform::GetRight()
{
	return right;
}

DirectX::XMFLOAT3 Transform::GetUp()
{
	return up;
}

/// <summary>
/// Sets the Transform's position
/// </summary>
/// <param name="x">The Transform's new X position</param>
/// <param name="y">The Transform's new Y position</param>
/// <param name="z">The Transform's new Z position</param>
void Transform::SetPosition(float x, float y, float z)
{
	position = XMFLOAT3(x, y, z);
	areMatricesDirty = true;
}

/// <summary>
/// Sets the Transform's position
/// </summary>
/// <param name="xyz">The Transform's new X, Y, and Z positions</param>
void Transform::SetPosition(DirectX::XMFLOAT3 xyz)
{
	position = xyz;
	areMatricesDirty = true;
}

/// <summary>
/// Sets the Transform's rotation
/// </summary>
/// <param name="pitch">The Transform's new rotation about the X axis</param>
/// <param name="yaw">The Transform's new rotation about the Y axis</param>
/// <param name="roll">The Transform's new rotation about the Z axis</param>
void Transform::SetRotation(float pitch, float yaw, float roll)
{
	rotation = XMFLOAT3(pitch, yaw, roll);
	areMatricesDirty = true;
	areVerticesDirty = true;
}

/// <summary>
/// Sets the Transform's rotation
/// </summary>
/// <param name="pitchYawRoll">The Transform's new rotation about the X, Y, and Z axes</param>
void Transform::SetRotation(DirectX::XMFLOAT3 pitchYawRoll)
{
	rotation = pitchYawRoll;
	areMatricesDirty = true;
	areVerticesDirty = true;
}

/// <summary>
/// Sets the Transform's scale
/// </summary>
/// <param name="x">The Transform's new scale about the X axis</param>
/// <param name="y">The Transform's new scale about the Y axis</param>
/// <param name="z">The Transform's new scale about the Z axis</param>
void Transform::SetScale(float x, float y, float z)
{
	scale = XMFLOAT3(x, y, z);
	areMatricesDirty = true;
}

/// <summary>
/// Sets the Transform's scale
/// </summary>
/// <param name="xyz">The Transform's new scale about the X, Y, and Z axes</param>
void Transform::SetScale(DirectX::XMFLOAT3 xyz)
{
	scale = xyz;
	areMatricesDirty = true;
}

/// <summary>
/// Applies a translation to the Transform relative to the world
/// </summary>
/// <param name="x">How far to translate the Transform along the world's X axis</param>
/// <param name="y">How far to translate the Transform along the world's Y axis</param>
/// <param name="z">How far to translate the Transform along the world's Z axis</param>
void Transform::MoveAbsolute(float x, float y, float z)
{
	// Build translation vector and pass to overload
	XMFLOAT3 translation(x, y, z);
	MoveAbsolute(translation);
}

/// <summary>
/// Applies a translation to the Transform relative to the world
/// </summary>
/// <param name="xyz">How far to translate the Transform along the X, Y, and Z axes</param>
void Transform::MoveAbsolute(DirectX::XMFLOAT3 xyz)
{
	// Add translation vector to position and store the result back
	XMStoreFloat3(&position, XMLoadFloat3(&position) + XMLoadFloat3(&xyz));
	areMatricesDirty = true;
}

/// <summary>
/// Applies a translation to the Transform relative to itself
/// </summary>
/// <param name="x">How far to translate the Transform along its X axis</param>
/// <param name="y">How far to translate the Transform along its Y axis</param>
/// <param name="z">How far to translate the Transform along its Z axis</param>
void Transform::MoveRelative(float x, float y, float z)
{
	XMFLOAT3 translation(x, y, z);
	MoveRelative(translation);
}

/// <summary>
/// Applies a translation to the Transform relative to itself
/// </summary>
/// <param name="xyz">How far to translate the Transform along its X, Y, and Z axes</param>
void Transform::MoveRelative(DirectX::XMFLOAT3 xyz)
{
	// Rotates the movement vector by the transform's rotation before adding it to the position
	XMStoreFloat3(
		&position, 
		XMLoadFloat3(&position) + XMVector3Rotate(
			XMLoadFloat3(&xyz),
			XMQuaternionRotationRollPitchYaw(rotation.x, rotation.y, rotation.z)
		)
	);
	areMatricesDirty = true;
}

/// <summary>
/// Adds an amount to the Transform's current rotation along each world axis
/// </summary>
/// <param name="pitch">How much to add to the Transform's rotation about the world's X axis</param>
/// <param name="yaw">How much to add to the Transform's rotation about the world's Y axis</param>
/// <param name="roll">How much to add to the Transform's rotation about the world's Z axis</param>
void Transform::Rotate(float pitch, float yaw, float roll)
{
	// Build rotation vector and pass to overload
	XMFLOAT3 rotation(pitch, yaw, roll);
	Rotate(rotation);
}

/// <summary>
/// Adds an amount to the Transform's current rotation along each world axis
/// </summary>
/// <param name="pitchYawRoll">How much to add to the Transform's rotation about the world's X, Y, and Z axes</param>
void Transform::Rotate(DirectX::XMFLOAT3 pitchYawRoll)
{
	// Add rotation vector to rotation and store the result back
	// (I'm not worrying about gimbal lock)
	XMStoreFloat3(&rotation, XMLoadFloat3(&rotation) + XMLoadFloat3(&pitchYawRoll));
	areMatricesDirty = true;
	areVerticesDirty = true;
}

/// <summary>
/// Multiplies the Transform's scale by an amount along each of its axes
/// </summary>
/// <param name="x">How much to multiply to the Transform's scale along its X axis</param>
/// <param name="y">How much to multiply to the Transform's scale along its Y axis</param>
/// <param name="z">How much to multiply to the Transform's scale along its Z axis</param>
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

/// <summary>
/// Multiplies the Transform's scale by an amount along each of its axes
/// </summary>
/// <param name="xyz">How much to multiply to the Transform's scale along its X, Y, and Z axes</param>
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

/// <summary>
/// Recalculates the Transform's World and World Inverse Transpose matrices,
/// then marks them as no longer dirty
/// </summary>
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

/// <summary>
/// Recalculates the Transform's Forward, Right, and Up vertices,
/// then marks them as no longer dirty
/// </summary>
void Transform::RebuildVertices()
{
	XMVECTOR rotQuat = XMQuaternionRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);
	// Set all three vertices
	right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	up = XMFLOAT3(0.0f, 1.0f, 1.0f);
	forward = XMFLOAT3(0.0f, 0.0f, 1.0f);
	// Rotate each vertex by the Transform's rotation values
	XMStoreFloat3(&right, XMVector3Rotate(XMLoadFloat3(&right), rotQuat));
	XMStoreFloat3(&up, XMVector3Rotate(XMLoadFloat3(&up), rotQuat));
	XMStoreFloat3(&forward, XMVector3Rotate(XMLoadFloat3(&forward), rotQuat));
	areVerticesDirty = false;
}
