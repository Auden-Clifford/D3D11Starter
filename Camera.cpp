#pragma once

#include "Transform.h"
#include <memory>
#include <DirectXMath.h>
#include "Camera.h"
#include "Input.h"
#include <algorithm>
#include <numbers>

/// <summary>
/// Creates a new Camera with the given settings
/// </summary>
/// <param name="a_fAspectRatio">Camera aspect ratio</param>
/// <param name="a_f3Position">Initial position</param>
/// <param name="a_f3Orientation">Initial orientation</param>
/// <param name="a_fFieldOfView">Field of View</param>
/// <param name="a_fNearPlaneDistance">Distance to the near clip plane</param>
/// <param name="a_fFarPlaneDistance">Distance to the far clip plane</param>
/// <param name="a_fMovementSpeed">Movement speed</param>
/// <param name="a_fMouseLookSpeed">Mouse look speed</param>
Camera::Camera(float a_fAspectRatio, DirectX::XMFLOAT3 a_f3Position, DirectX::XMFLOAT3 a_f3Orientation, float a_fFieldOfView, float a_fNearPlaneDistance, float a_fFarPlaneDistance, float a_fMovementSpeed, float a_fMouseLookSpeed)
{
	// initialize transform
	m_spTransform = std::make_shared<Transform>();
	m_spTransform->SetPosition(a_f3Position);
	m_spTransform->SetRotation(a_f3Orientation);

	// initialize settings
	m_fFieldOfView = a_fFieldOfView;
	m_fNearPlaneDistance = a_fNearPlaneDistance;
	m_fFarPlaneDistance = a_fFarPlaneDistance;
	m_fMovementSpeed = a_fMovementSpeed;
	m_fMouseLookSpeed = a_fMouseLookSpeed;

	// update projection & view matrix
	UpdateProjectionMatrix(a_fAspectRatio);
	UpdateViewMatrix();
}

#pragma region Getters
/// <summary>
/// Gets the camera's view matrix
/// </summary>
/// <returns></returns>
DirectX::XMFLOAT4X4 Camera::GetViewMatrix() 
{
	return m_m4View;
}
/// <summary>
/// Gets the camera's projection matrix
/// </summary>
/// <returns></returns>
DirectX::XMFLOAT4X4 Camera::GetProjectionMatrix()
{
	return m_m4Projection;
}
/// <summary>
/// Gets the camera's transform
/// </summary>
/// <returns>Transform</returns>
std::shared_ptr<Transform> Camera::GetTransform()
{
	return m_spTransform;
}
/// <summary>
/// Gets the camera's field of view
/// </summary>
/// <returns>Field of view</returns>
float Camera::GetFieldOfView()
{
	return m_fFieldOfView;
}
/// <summary>
/// Gets the camera's near clip plane distance
/// </summary>
/// <returns>Near clip plane distance</returns>
float Camera::GetNearClipPlaneDistance()
{
	return m_fNearPlaneDistance;
}
/// <summary>
/// Gets the camera's far clip plane distance
/// </summary>
/// <returns>Far clip plane distance</returns>
float Camera::GetFarClipPlaneDistance()
{
	return m_fFarPlaneDistance;
}
/// <summary>
/// Gets the camera's movement speed
/// </summary>
/// <returns>Movement speed</returns>
float Camera::GetMovementSpeed()
{
	return m_fMovementSpeed;
}
/// <summary>
/// Gets the camera's mouse look speed
/// </summary>
/// <returns>Mouse look speed</returns>
float Camera::GetMouseLookSpeed()
{
	return m_fMouseLookSpeed;
}
#pragma endregion
#pragma region Updates
/// <summary>
/// Updates the projection matrix with the given aspect ratio
/// </summary>
/// <param name="a_fAspectRatio"></param>
void Camera::UpdateProjectionMatrix(float a_fAspectRatio)
{
	XMStoreFloat4x4(&m_m4Projection, DirectX::XMMatrixPerspectiveFovLH(m_fFieldOfView, a_fAspectRatio, m_fNearPlaneDistance, m_fFarPlaneDistance));
}
/// <summary>
/// Updates the View matrix to match the transform position and rotation
/// </summary>
void Camera::UpdateViewMatrix()
{
	DirectX::XMFLOAT3 f3Position = m_spTransform->GetPosition();
	DirectX::XMFLOAT3 f3Forward = m_spTransform->GetForward();

	XMStoreFloat4x4(&m_m4View, DirectX::XMMatrixLookToLH(
		XMLoadFloat3(&f3Position),  
		XMLoadFloat3(&f3Forward),   
		DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f))); // world up vector 
}
void Camera::Update(float a_fDeltaTime)
{
	//WASD movement
	if (Input::KeyDown('W'))
	{
		m_spTransform->MoveRelative(0.0f, 0.0f, m_fMovementSpeed * a_fDeltaTime);
	}
	if(Input::KeyDown('S'))
	{
		m_spTransform->MoveRelative(0.0f, 0.0f, -m_fMovementSpeed * a_fDeltaTime);
	}
	if (Input::KeyDown('A'))
	{
		m_spTransform->MoveRelative(-m_fMovementSpeed * a_fDeltaTime, 0.0f, 0.0f);
	}
	if (Input::KeyDown('D'))
	{
		m_spTransform->MoveRelative(m_fMovementSpeed * a_fDeltaTime, 0.0f, 0.0f);
	}

	// move up with SPACEBAR
	if (Input::KeyDown(VK_SPACE))
	{
		m_spTransform->MoveRelative(0.0f, m_fMovementSpeed * a_fDeltaTime, 0.0f);
	}
	// move down with SHIFT
	if (Input::KeyDown(VK_SHIFT))
	{
		m_spTransform->MoveRelative(0.0f, -m_fMovementSpeed * a_fDeltaTime, 0.0f);
	}

	// get mouse input
	if (Input::MouseLeftDown())
	{
		float cursorMovementX = Input::GetMouseXDelta() * m_fMouseLookSpeed;
		float cursorMovementY = Input::GetMouseYDelta() * m_fMouseLookSpeed;
		
		// Apply the rotation
		float newPitch = m_spTransform->GetPitchYawRoll().x + cursorMovementY;
		float newYaw = m_spTransform->GetPitchYawRoll().y + cursorMovementX;

		// Clamp the pitch to the range [-PI/2, PI/2] to prevent over-rotation
		newPitch = std::clamp(newPitch, (-DirectX::XM_PI / 2) + 0.01f, (DirectX::XM_PI / 2) - 0.01f);

		m_spTransform->SetRotation(newPitch, newYaw, 0.0f);
	}

	UpdateViewMatrix();
}
#pragma endregion

