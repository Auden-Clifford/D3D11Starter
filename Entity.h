#pragma once

#include "Transform.h"
#include <wrl/client.h>
#include "Mesh.h"
#include "Camera.h"

class Entity
{
public:
	Entity(Mesh a_mMesh);

	void Draw(std::shared_ptr<Camera> a_spCamera, Microsoft::WRL::ComPtr<ID3D11Buffer> a_cpConstantBuffer, DirectX::XMFLOAT4 a_f4Tint);

	// Getters
	std::shared_ptr<Mesh> GetMesh();
	std::shared_ptr<Transform> GetTransform();



private:
	std::shared_ptr<Transform> m_spTransform;
	std::shared_ptr<Mesh> m_spMesh;
};