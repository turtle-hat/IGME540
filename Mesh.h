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
	Mesh(Vertex* _vertices, unsigned int* _indices, unsigned int _vertexCount, unsigned int _indexCount);
	~Mesh();
	// Draws Mesh to screen
	void Draw();
	// Accessors for Mesh info
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetVertexBuffer();
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetIndexBuffer();
	int GetVertexCount();
	int GetIndexCount();
};

