#pragma once

#include <DirectXMath.h>

// Stores info that will be applied to all shapes when rendering
struct VertexShaderData
{
	// Transform matrix for each triangle's position in the world
	DirectX::XMFLOAT4X4 tfWorld;
	// Transform matrix for the camera's view
	DirectX::XMFLOAT4X4 tfView;
	// Transform matrix for the camera's projection
	DirectX::XMFLOAT4X4 tfProjection;
	// Color multiplied to each channel of all triangles (RGBA)
	DirectX::XMFLOAT4 colorTint;
} ;