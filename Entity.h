#pragma once

#include "Transform.h"
#include <wrl/client.h>
#include "Mesh.h"
#include "Camera.h"
#include "Material.h"

class Entity
{
public:
	Entity(Mesh a_mMesh, Material a_mtMaterial);

	void Draw(std::shared_ptr<Camera> a_spCamera);

	// Getters
	std::shared_ptr<Mesh> GetMesh();
	std::shared_ptr<Transform> GetTransform();
	std::shared_ptr<Material> GetMaterial();

	// Setters
	void SetMesh(std::shared_ptr<Mesh> a_spMesh);
	void SetTransform(std::shared_ptr<Transform> a_spTransform);
	void setMaterial(std::shared_ptr<Material> a_spMaterial);
private:
	std::shared_ptr<Transform> m_spTransform;
	std::shared_ptr<Mesh> m_spMesh;
	std::shared_ptr<Material> m_spMaterial;
};