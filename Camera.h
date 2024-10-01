#pragma once

#include <memory>
#include <DirectXMath.h>
#include "Input.h"
#include "Transform.h"

class Camera
{
public:
	// Constructors
	Camera(const char* _name, std::shared_ptr<Transform> _transform, float _aspect);
	Camera(const char* _name, std::shared_ptr<Transform> _transform, float _aspect, float _fov);
	Camera(const char* _name, std::shared_ptr<Transform> _transform, float _aspect, float _fov, bool _isPerspective);

	// Getters
	std::shared_ptr<Transform> GetTransform();
	const char* GetName();
	float GetFov();
	float GetMoveSpeed();
	float GetLookSpeed();
	float GetNearClip();
	float GetFarClip();

	// Setters
	void SetAspect(float _aspect);
	void SetFov(float _fov);
	void SetMoveSpeed(float _speed);
	void SetLookSpeed(float _speed);
	void SetNearClip(float _distance);
	void SetFarClip(float _distance);

	void UpdateViewMatrix();

private:
	std::shared_ptr<Transform> transform;
	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 projection;

	// Name for UI
	const char* name;
	// Near and far clip planes
	float nearDist;
	float farDist;
	// Aspect Ratio
	float aspect;
	// Field of View, in radians
	float fov;
	// Move speed in units per second
	float moveSpeed;
	// Move speed in milliradians per pixel
	float lookSpeed;
	// Whether the camera uses a perspective projection instead of an orthographic projection
	bool isPerspective;

	// Called by SetAspect() and SetFov()
	void UpdateProjectionMatrix(float _aspect);
};

