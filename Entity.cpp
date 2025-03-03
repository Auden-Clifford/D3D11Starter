#pragma once

#include <memory>
#include "Transform.h"
#include <wrl/client.h>
#include "Mesh.h"
#include "Entity.h"
#include <d3d11.h>
#include "BufferStructs.h"

/// <summary>
/// Creates a new entity with the given mesh and a default transform
/// </summary>
/// <param name="a_mMesh">Mesh of this entity</param>
Entity::Entity(Mesh a_mMesh)
{
	m_spMesh = std::make_shared<Mesh>(a_mMesh);
	m_spTransform = std::make_shared<Transform>();
}

void Entity::Draw(Microsoft::WRL::ComPtr<ID3D11Buffer> a_cpConstantBuffer, DirectX::XMFLOAT4 a_f4Tint)
{
	//Bind the Constant Buffer resource for the Vertex Shader stage
	
	//calcualate the amount of space our struct needs (next multiple of 16)
	unsigned int size = sizeof(VertexShaderData);
	size = (size + 15) / 16 * 16;

	D3D11_BUFFER_DESC cbDesc = {}; // Sets struct to all zeros
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.ByteWidth = size; // Must be a multiple of 16
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	Graphics::Device->CreateBuffer(&cbDesc, 0, a_cpConstantBuffer.GetAddressOf());

	//Collect data for the current entity in a C++ struct 
	VertexShaderData vsdData;
	vsdData.colorTint = a_f4Tint;
	vsdData.world = m_spTransform->GetWorldMatrix();

	//Map / memcpy / Unmap the Constant Buffer resource
	D3D11_MAPPED_SUBRESOURCE mappedBuffer = {}; 
	Graphics::Context->Map(a_cpConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer); 

	memcpy(mappedBuffer.pData, &vsdData, sizeof(vsdData)); 

	Graphics::Context->Unmap(a_cpConstantBuffer.Get(), 0); 

	Graphics::Context->VSSetConstantBuffers( 
		0, // the slot (register) to bind the buffer to
		1, // number of buffers to set right now 
		a_cpConstantBuffer.GetAddressOf()); 

	//Set the correct Vertex and Index Buffers
	//Tell D3D to render using the currently bound resources
	m_spMesh->Draw();
}

#pragma region Getters
std::shared_ptr<Mesh> Entity::GetMesh()
{
	return m_spMesh;
}
std::shared_ptr<Transform> Entity::GetTransform()
{
	return m_spTransform;
}
#pragma endregion
