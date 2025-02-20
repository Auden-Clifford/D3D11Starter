#pragma once

#include "Transform.h"
#include <memory>
#include <DirectXMath.h>


class Camera
{
public:
	// OOP stuff
	Camera(float a_fAspectRatio, DirectX::XMFLOAT3 a_f3Position, DirectX::XMFLOAT3 a_f3Orientation, float a_fFieldOfView, float a_fNearPlaneDistance, float a_fFarPlaneDistance, float a_fMovementSpeed, float a_fMouseLookSpeed);

	// Getters
	DirectX::XMFLOAT4X4 GetViewMatrix();
	DirectX::XMFLOAT4X4 GetProjectionMatrix();

	void UpdateProjectionMatrix(float a_fAspectRatio);
	void UpdateViewMatrix();
	void Update(float a_fDeltaTime);

private:
	std::shared_ptr<Transform> m_spTransform;
	DirectX::XMFLOAT4X4 m_m4View;
	DirectX::XMFLOAT4X4 m_m4Projection;

	float m_fFieldOfView;
	float m_fNearPlaneDistance;
	float m_fFarPlaneDistance;
	float m_fMovementSpeed;
	float m_fMouseLookSpeed;
};