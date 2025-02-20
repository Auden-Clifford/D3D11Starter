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
	XMStoreFloat4x4(&m_m4View, DirectX::XMMatrixLookToLH(
		XMLoadFloat3(&m_spTransform->GetPosition()),
		XMLoadFloat3(&m_spTransform->GetForward()), 
		DirectX::XMVECTOR(0.0f, 1.0f, 0.0f, 0.0f))); // world up vector
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
		int cursorMovementX = Input::GetMouseXDelta() * m_fMouseLookSpeed;
		int cursorMovementY = Input::GetMouseYDelta() * m_fMouseLookSpeed;
		
		// clamp x rotation
		//cursorMovementY = std::clamp(cursorMovementY, m_spTransform->GetPitchYawRoll().x - ( / 2), )
		m_spTransform->Rotate(cursorMovementY, cursorMovementX, 0.0f);
	}
}
#pragma endregion

