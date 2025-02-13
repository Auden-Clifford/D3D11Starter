#pragma once

#include <memory>
#include "Transform.h"
#include <wrl/client.h>
#include "Mesh.h"
#include "Entity.h"
#include <d3d11.h>

/// <summary>
/// Creates a new entity with the given mesh and a default transform
/// </summary>
/// <param name="a_mMesh">Mesh of this entity</param>
Entity::Entity(Mesh a_mMesh)
{
	m_spMesh = std::make_shared<Mesh>(a_mMesh);
	m_spTransform = std::make_shared<Transform>();
}

void Entity::Draw(Microsoft::WRL::ComPtr<ID3D11Buffer> a_cpConstantBuffer)
{

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
