#pragma once

#include <DirectXMath.h>

// Stores info that will be applied to all shapes when rendering
struct VertexShaderData
{
	// Color multiplied to each channel of all triangles (RGBA)
	DirectX::XMFLOAT4 colorTint;
	// Position added to all triangles (XYZ)
	DirectX::XMFLOAT3 positionOffset;
} ;