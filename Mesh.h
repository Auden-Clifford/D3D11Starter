#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include "Vertex.h"
#include "Graphics.h"

class Mesh
{
public:
	// OOP stuff
	Mesh(Vertex* a_pVerticies, unsigned int a_uVerticiesLength, unsigned int* a_pIndicies, unsigned int a_uIndiciesLength);
	~Mesh();

	// primary functions
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetVertexBuffer();
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetIndexBuffer();

	unsigned int GetIndexCount();
	unsigned int GetVertexCount();
	void Draw();

private:
	// geometry data buffers
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_cpVertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_cpIndexBuffer;

	unsigned int m_uIndicies; //number of indices in index buffer
	unsigned int m_uVertices; //number of vertices in vertex buffer
	//unsigned int m_nFaces;
};