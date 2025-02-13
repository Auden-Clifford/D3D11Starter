#pragma once

#include "Transform.h"
#include <wrl/client.h>
#include "Mesh.h"

class Entity
{
public:
	Entity(Mesh a_mMesh);

	void Draw(Microsoft::WRL::ComPtr<ID3D11Buffer> a_cpConstantBuffer);

	// Getters
	std::shared_ptr<Mesh> GetMesh();
	std::shared_ptr<Transform> GetTransform();



private:
	std::shared_ptr<Transform> m_spTransform;
	std::shared_ptr<Mesh> m_spMesh;
};