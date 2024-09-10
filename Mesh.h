#pragma once

#include <d3d11.h>
#include <wrl/client.h>

#include "Graphics.h"
#include "Vertex.h"

class Mesh
{
private:
	// Vertex and index buffers, as well as the size of each
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
	int vertexCount;
	int indexCount;

public:
	// Constructor/Destructor
	Mesh(Vertex* vertices, unsigned int* indices);
	~Mesh();
	// Accessors for vertex and index buffer info
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetVertexBuffer();
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetIndexBuffer();
	int GetVertexCount();
	int GetIndexCount();
	// Draws Mesh to screen
	void Draw();
};

