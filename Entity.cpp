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
Entity::Entity(Mesh a_mMesh, Material a_mtMaterial)
{
	m_spMesh = std::make_shared<Mesh>(a_mMesh);
	m_spTransform = std::make_shared<Transform>();
	m_spMaterial = std::make_shared<Material>(a_mtMaterial);
}

void Entity::Draw(std::shared_ptr<Camera> a_spCamera, float a_fTotalTime)
{
	m_spMaterial->GetVertexShader()->SetShader();
	m_spMaterial->GetPixelShader()->SetShader();

	//Collect data for the current entity in a C++ struct 
	//m_spMaterial->GetVertexShader()->SetFloat4("colorTint", m_spMaterial->GetColorTint()); 
	m_spMaterial->GetVertexShader()->SetMatrix4x4("world", m_spTransform->GetWorldMatrix()); 
	m_spMaterial->GetVertexShader()->SetMatrix4x4("view", a_spCamera->GetViewMatrix()); 
	m_spMaterial->GetVertexShader()->SetMatrix4x4("projection", a_spCamera->GetProjectionMatrix());

	m_spMaterial->GetPixelShader()->SetFloat4("colorTint", m_spMaterial->GetColorTint());
	m_spMaterial->GetPixelShader()->SetFloat2("uvScale", m_spMaterial->GetUVScale());
	m_spMaterial->GetPixelShader()->SetFloat2("uvOffset", m_spMaterial->GetUVOffset());
	m_spMaterial->GetPixelShader()->SetFloat("totalTime", a_fTotalTime);
	

	//Map / memcpy / Unmap the Constant Buffer resource
	m_spMaterial->GetVertexShader()->CopyAllBufferData();
	m_spMaterial->GetPixelShader()->CopyAllBufferData();

	// bind texture & sampler
	m_spMaterial->PrepareMaterial();

	//Set the correct Vertex and Index Buffers
	//Tell D3D to render using the currently bound resources
	m_spMesh->Draw();
}

#pragma region Getters
/// <summary>
/// Gets the entity's mesh
/// </summary>
/// <returns>Mesh</returns>
std::shared_ptr<Mesh> Entity::GetMesh()
{
	return m_spMesh;
}
/// <summary>
/// Gets the entity's transform
/// </summary>
/// <returns>Transform</returns>
std::shared_ptr<Transform> Entity::GetTransform()
{
	return m_spTransform;
}
/// <summary>
/// Gets the entity's material
/// </summary>
/// <returns>Material</returns>
std::shared_ptr<Material> Entity::GetMaterial()
{
	return m_spMaterial;
}
#pragma endregion
#pragma region Setters
/// <summary>
/// Sets the entity's mesh to the given mesh
/// </summary>
/// <param name="a_spMesh">New mesh</param>
void Entity::SetMesh(std::shared_ptr<Mesh> a_spMesh)
{
	m_spMesh = a_spMesh;
}
/// <summary>
/// Sets the entity's transform to the given transform
/// </summary>
/// <param name="a_spTransform">New transform</param>
void Entity::SetTransform(std::shared_ptr<Transform> a_spTransform)
{
	m_spTransform = a_spTransform;
}
/// <summary>
/// Sets the entity's material to the given material
/// </summary>
/// <param name="a_spMaterial">New material</param>
void Entity::setMaterial(std::shared_ptr<Material> a_spMaterial)
{
	m_spMaterial = a_spMaterial;
}
#pragma endregion

