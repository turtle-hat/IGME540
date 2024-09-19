#pragma once

#include <DirectXMath.h>

// Represents a transformation in 3D space
class Transform
{
public:
	// Constructor
	Transform();

	// Getters
	DirectX::XMFLOAT3 GetPosition();
	DirectX::XMFLOAT3 GetRotation();
	DirectX::XMFLOAT3 GetScale();
	DirectX::XMFLOAT4X4 GetWorld();
	DirectX::XMFLOAT4X4 GetWorldInverseTranspose();

	// Setters
	void SetPosition(float x, float y, float z);
	void SetPosition(DirectX::XMFLOAT3 xyz);
	void SetRotation(float pitch, float yaw, float roll);
	void SetRotation(DirectX::XMFLOAT3 pitchYawRoll);
	void SetScale(float x, float y, float z);
	void SetScale(DirectX::XMFLOAT3 xyz);

	// Mutators
	void MoveAbsolute(float x, float y, float z);
	void MoveAbsolute(DirectX::XMFLOAT3 xyz);
	void Rotate(float pitch, float yaw, float roll);
	void Rotate(DirectX::XMFLOAT3 pitchYawRoll);
	void Scale(float x, float y, float z);
	void Scale(DirectX::XMFLOAT3 xyz);

private:
	// Translation
	DirectX::XMFLOAT3 position;
	// Rotation
	DirectX::XMFLOAT3 rotation;
	// Scale
	DirectX::XMFLOAT3 scale;
	// Final World Matrix
	DirectX::XMFLOAT4X4 world;
	// Final Inverse Transpose World Matrix
	DirectX::XMFLOAT4X4 worldInverseTranspose;

	// Whether transform matrix needs to be rebuilt
	bool isMatrixDirty;
};
