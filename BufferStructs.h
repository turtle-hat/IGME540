#pragma once

#include <DirectXMath.h>

// Stores info that will be applied to all shapes when rendering
struct VertexShaderData
{
	// Transform matrix for each triangle's position in the world
	DirectX::XMFLOAT4X4 tfWorld;
	// Color multiplied to each channel of all triangles (RGBA)
	DirectX::XMFLOAT4 colorTint;
} ;