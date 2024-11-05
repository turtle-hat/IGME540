#pragma once

#include <d3d11.h>
#include <wrl/client.h>

#include "Graphics.h"
#include "Vertex.h"

class Mesh
{
public:
	// Constructors/Destructor
	Mesh(const char* _name, Vertex* _vertices, size_t _vertexCount, unsigned int* _indices, size_t _indexCount);
	Mesh(const char* _name, const wchar_t* _path);
	~Mesh();
	// Draws Mesh to screen
	void Draw();
	// Accessors for Mesh info
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetVertexBuffer();
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetIndexBuffer();
	int GetVertexCount();
	int GetIndexCount();
	const char* GetName();

private:
	// Vertex and index buffers, as well as the size of each
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
	unsigned int vertexCount;
	unsigned int indexCount;

	// Name for UI
	const char* name;

	// Code for calculating tangents
	void CalculateTangents(Vertex* verts, int numVerts, unsigned int* indices, int numIndices);
	// Code for creating vertex and index buffers
	void InitializeBuffers(Vertex* _vertices, unsigned int _vertexCount, unsigned int* _indices, unsigned int _indexCount);
};

