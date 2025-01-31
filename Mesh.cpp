#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include "Vertex.h"
#include "Graphics.h"
#include "Mesh.h"

/// <summary>
/// Takes a set of verticies and indicies and creates a vertex and index buffer for this mesh
/// </summary>
/// <param name="a_pVerticies">Array of verticies</param>
/// <param name="a_uVerticiesLength">The number of verticies in the array</param>
/// <param name="a_pIndicies">Array of indicies</param>
/// <param name="a_uIndiciesLength">The number of indicies in the array</param>
Mesh::Mesh(Vertex* a_pVerticies, unsigned int a_uVerticiesLength, unsigned int* a_pIndicies, unsigned int a_uIndiciesLength)
{
	// initialize values
	m_uVertices = a_uVerticiesLength;
	m_uIndicies = a_uIndiciesLength;

	// Create a VERTEX BUFFER
	D3D11_BUFFER_DESC vbd = {}; 
	vbd.Usage = D3D11_USAGE_IMMUTABLE;	// Will NEVER change 
	vbd.ByteWidth = sizeof(Vertex) * a_uVerticiesLength;       // 3 = number of vertices in the buffer
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER; // Tells Direct3D this is a vertex buffer 
	vbd.CPUAccessFlags = 0;	// Note: We cannot access the data from C++ (this is good) 
	vbd.MiscFlags = 0; 
	vbd.StructureByteStride = 0;

	// specify initial vertex data
	D3D11_SUBRESOURCE_DATA initialVertexData = {}; 
	initialVertexData.pSysMem = a_pVerticies; // pSysMem = Pointer to System Memory

	//create the buffer
	Graphics::Device->CreateBuffer(&vbd, &initialVertexData, m_cpVertexBuffer.GetAddressOf());

	// Create an INDEX BUFFER
	D3D11_BUFFER_DESC ibd = {};
	ibd.Usage = D3D11_USAGE_IMMUTABLE;	// Will NEVER change
	ibd.ByteWidth = sizeof(unsigned int) * a_uIndiciesLength;	// 3 = number of indices in the buffer
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;	// Tells Direct3D this is an index buffer
	ibd.CPUAccessFlags = 0;	// Note: We cannot access the data from C++ (this is good)
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;

	//specify initial index data
	D3D11_SUBRESOURCE_DATA initialIndexData = {};
	initialIndexData.pSysMem = a_pIndicies; // pSysMem = Pointer to System Memory

	//create the index buffer
	Graphics::Device->CreateBuffer(&ibd, &initialIndexData, m_cpIndexBuffer.GetAddressOf()); 
}

// default destructor
Mesh::~Mesh(){}
