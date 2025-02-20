#pragma once

#include <DirectXMath.h>
#include "Transform.h"

Transform::Transform()
{
	m_f3Position = DirectX::XMFLOAT3();
	m_f3Rotation = DirectX::XMFLOAT3();
	m_f3Scale = DirectX::XMFLOAT3(1,1,1); // scale is (1,1,1) not (0,0,0)

	DirectX::XMStoreFloat4x4(&m_m4World, DirectX::XMMatrixIdentity());
	DirectX::XMStoreFloat4x4(&m_m4WorldInverseTranspose, DirectX::XMMatrixIdentity());

	m_bDirty = false;
}

#pragma region Setters
/// <summary>
/// Sets the position of the transform to the given x, y, and z values
/// </summary>
/// <param name="a_fXPosition">New x position</param>
/// <param name="a_fYPosition">New y position</param>
/// <param name="a_fZPosition">New z position</param>
void Transform::SetPosition(float a_fXPosition, float a_fYPosition, float a_fZPosition)
{
	m_f3Position = DirectX::XMFLOAT3(a_fXPosition, a_fYPosition, a_fZPosition);
	m_bDirty = true;
}
/// <summary>
/// Sets the position of the transform to the given position
/// </summary>
/// <param name="a_f3Position">New position</param>
void Transform::SetPosition(DirectX::XMFLOAT3 a_f3Position)
{
	m_f3Position = DirectX::XMFLOAT3(a_f3Position);
	m_bDirty = true;
}
/// <summary>
/// Sets the rotation of the transform to the given pitch, yaw, and roll values
/// </summary>
/// <param name="a_fPitch">New pitch</param>
/// <param name="a_fYaw">New yaw</param>
/// <param name="a_fRoll">New roll</param>
void Transform::SetRotation(float a_fPitch, float a_fYaw, float a_fRoll)
{
	m_f3Rotation = DirectX::XMFLOAT3(a_fPitch, a_fYaw, a_fRoll);
	m_bDirty = true;
}
/// <summary>
/// Sets the rotation of the transform to the given rotation
/// </summary>
/// <param name="a_f3Rotation">New rotation</param>
void Transform::SetRotation(DirectX::XMFLOAT3 a_f3Rotation)
{
	m_f3Rotation = a_f3Rotation;
	m_bDirty = true;
}
/// <summary>
/// Sets the scale of the transform to the given x, y and z values
/// </summary>
/// <param name="a_fXScale">New x scale</param>
/// <param name="a_fYScale">New Y scale</param>
/// <param name="a_fZScale">New Z scale</param>
void Transform::SetScale(float a_fXScale, float a_fYScale, float a_fZScale)
{
	m_f3Scale = DirectX::XMFLOAT3(a_fXScale, a_fYScale, a_fZScale);
	m_bDirty = true;
}
/// <summary>
/// Sets the scale of the transform to the given scale
/// </summary>
/// <param name="a_f3Scale">New Scale</param>
void Transform::SetScale(DirectX::XMFLOAT3 a_f3Scale)
{
	m_f3Scale = a_f3Scale;
	m_bDirty = true;
}
#pragma endregion
#pragma region Getters
/// <summary>
/// Gets the transform's right vector
/// </summary>
/// <returns>Right vector</returns>
DirectX::XMFLOAT3 Transform::GetRight()
{
	DirectX::XMVECTOR xvWorldRight = DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
	DirectX::XMVECTOR xvRotation = DirectX::XMQuaternionRotationRollPitchYaw(m_f3Rotation.x, m_f3Rotation.y, m_f3Rotation.z);
	DirectX::XMFLOAT3 f3LocalRight;
	XMStoreFloat3(&f3LocalRight, DirectX::XMVector3Rotate(xvWorldRight, xvRotation));
	return f3LocalRight;
}
/// <summary>
/// Gets the transform's up vector
/// </summary>
/// <returns>Up vector</returns>
DirectX::XMFLOAT3 Transform::GetUp()
{
	DirectX::XMVECTOR xvWorldUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	DirectX::XMVECTOR xvRotation = DirectX::XMQuaternionRotationRollPitchYaw(m_f3Rotation.x, m_f3Rotation.y, m_f3Rotation.z);
	DirectX::XMFLOAT3 f3LocalUp;
	XMStoreFloat3(&f3LocalUp, DirectX::XMVector3Rotate(xvWorldUp, xvRotation));
	return f3LocalUp;
}
/// <summary>
/// Gets the transform's forward vector
/// </summary>
/// <returns>Forward vector</returns>
DirectX::XMFLOAT3 Transform::GetForward()
{
	DirectX::XMVECTOR xvWorldForward = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	DirectX::XMVECTOR xvRotation = DirectX::XMQuaternionRotationRollPitchYaw(m_f3Rotation.x, m_f3Rotation.y, m_f3Rotation.z);
	DirectX::XMFLOAT3 f3LocalForward;
	XMStoreFloat3(&f3LocalForward, DirectX::XMVector3Rotate(xvWorldForward, xvRotation));
	return f3LocalForward;
}
/// <summary>
/// Gets the transform's current position
/// </summary>
/// <returns>Current position</returns>
DirectX::XMFLOAT3 Transform::GetPosition()
{
	return m_f3Position;
}
/// <summary>
/// Gets the transform's current rotation
/// </summary>
/// <returns>Current rotation</returns>
DirectX::XMFLOAT3 Transform::GetPitchYawRoll()
{
	return m_f3Rotation;
}
/// <summary>
/// Gets the transform's current scale
/// </summary>
/// <returns>Current scale</returns>
DirectX::XMFLOAT3 Transform::GetScale()
{
	return m_f3Scale;
}
/// <summary>
/// Gets the transform's world matrix
/// </summary>
/// <returns>World matrix</returns>
DirectX::XMFLOAT4X4 Transform::GetWorldMatrix()
{
	// if the transform is dirty, recalculate it
	if (m_bDirty)
	{
		DirectX::XMMATRIX t = DirectX::XMMatrixTranslation(m_f3Position.x, m_f3Position.y, m_f3Position.z);
		DirectX::XMMATRIX r = DirectX::XMMatrixRotationRollPitchYaw(m_f3Rotation.x, m_f3Rotation.y, m_f3Rotation.z);
		DirectX::XMMATRIX s = DirectX::XMMatrixScaling(m_f3Scale.x, m_f3Scale.y, m_f3Scale.z);

		// calculate the matrix
		DirectX::XMMATRIX world = s * r * t;

		//store it (and inverse)
		XMStoreFloat4x4(&m_m4World, world);
		XMStoreFloat4x4(&m_m4WorldInverseTranspose,XMMatrixInverse(0, XMMatrixTranspose(world)));

		// transform is no longer dirty
		m_bDirty = false;
	}

	return m_m4World;
}
/// <summary>
/// Gets the transpose of the transform's world matrix
/// </summary>
/// <returns>Transposed world matrix</returns>
DirectX::XMFLOAT4X4 Transform::GetWorldInverseTransposeMatrix()
{
	// if the transform is dirty, recalculate it
	if (m_bDirty)
	{
		DirectX::XMMATRIX t = DirectX::XMMatrixTranslation(m_f3Position.x, m_f3Position.y, m_f3Position.z);
		DirectX::XMMATRIX r = DirectX::XMMatrixRotationRollPitchYaw(m_f3Rotation.x, m_f3Rotation.y, m_f3Rotation.z);
		DirectX::XMMATRIX s = DirectX::XMMatrixScaling(m_f3Scale.x, m_f3Scale.y, m_f3Scale.z);

		// calculate the matrix
		DirectX::XMMATRIX world = s * r * t;

		//store it (and inverse)
		XMStoreFloat4x4(&m_m4World, world);
		XMStoreFloat4x4(&m_m4WorldInverseTranspose, XMMatrixInverse(0, XMMatrixTranspose(world)));

		// transform is no longer dirty
		m_bDirty = false;
	}

	return m_m4WorldInverseTranspose;
}
#pragma endregion
#pragma region Transformers
/// <summary>
/// Moves the transform by the given x, y, and z offsets in local space
/// </summary>
/// <param name="a_fXOffset">X position offset</param>
/// <param name="a_fYOffset">Y position offset</param>
/// <param name="a_fZOffset">Z position offset</param>
void Transform::MoveRelative(float a_fXOffset, float a_fYOffset, float a_fZOffset)
{
	DirectX::XMVECTOR xvOffset = DirectX::XMVectorSet(a_fXOffset, a_fYOffset, a_fZOffset, 0.0f);
	DirectX::XMVECTOR xvRotation = DirectX::XMQuaternionRotationRollPitchYaw(m_f3Rotation.x, m_f3Rotation.y, m_f3Rotation.z);
	DirectX::XMFLOAT3 f3AbsoluteOffset;
	XMStoreFloat3(&f3AbsoluteOffset, DirectX::XMVector3Rotate(xvOffset, xvRotation));
	MoveAbsolute(f3AbsoluteOffset);
}
/// <summary>
/// Moves the transform by the given offset in local space
/// </summary>
/// <param name="a_fZOffset">Position offset</param> 
void Transform::MoveRelative(DirectX::XMFLOAT3 a_f3Offset)
{
	DirectX::XMVECTOR xvOffset = DirectX::XMVectorSet(a_f3Offset.x, a_f3Offset.y, a_f3Offset.z, 0.0f);
	DirectX::XMVECTOR xvRotation = DirectX::XMQuaternionRotationRollPitchYaw(m_f3Rotation.x, m_f3Rotation.y, m_f3Rotation.z);
	DirectX::XMFLOAT3 f3AbsoluteOffset;
	XMStoreFloat3(&f3AbsoluteOffset, DirectX::XMVector3Rotate(xvOffset, xvRotation));
	MoveAbsolute(f3AbsoluteOffset);
}
/// <summary>
/// Moves the transform by the given x, y, and z offsets in world space
/// </summary>
/// <param name="a_fXOffset">X position offset</param>
/// <param name="a_fYOffset">Y position offset</param>
/// <param name="a_fZOffset">Z position offset</param>
void Transform::MoveAbsolute(float a_fXOffset, float a_fYOffset, float a_fZOffset)
{
	m_f3Position.x += a_fXOffset;
	m_f3Position.y += a_fYOffset;
	m_f3Position.z += a_fZOffset;
	m_bDirty = true;
}
/// <summary>
/// Moves the transform by the given offset in world space
/// </summary>
/// <param name="a_f3Offset">Position offset</param>
void Transform::MoveAbsolute(DirectX::XMFLOAT3 a_f3Offset)
{
	m_f3Position.x +=  a_f3Offset.x;
	m_f3Position.y += a_f3Offset.y;
	m_f3Position.z += a_f3Offset.z;
	m_bDirty = true;
}
/// <summary>
/// Rotates the transform by the given pitch, yaw, and roll offsets
/// </summary>
/// <param name="a_fPitch">Pitch offset</param>
/// <param name="a_fYaw">Yaw offset</param>
/// <param name="a_fRoll">Roll offset</param>
void Transform::Rotate(float a_fPitch, float a_fYaw, float a_fRoll)
{
	m_f3Rotation.x += a_fPitch;
	m_f3Rotation.y += a_fYaw;
	m_f3Rotation.z += a_fRoll;
	m_bDirty = true;
}
/// <summary>
/// Rotates the transform by the given rotation
/// </summary>
/// <param name="a_f3Rotation">Rotation</param>
void Transform::Rotate(DirectX::XMFLOAT3 a_f3Rotation)
{
	m_f3Rotation.x += a_f3Rotation.x;
	m_f3Rotation.y += a_f3Rotation.y;
	m_f3Rotation.z += a_f3Rotation.z;
	m_bDirty = true;
}
/// <summary>
/// Scales the transform by the given x, y, and z scalars
/// </summary>
/// <param name="a_fXScale">X scalar</param>
/// <param name="a_fYScale">Y scalar</param>
/// <param name="a_fZScale">Z scalar</param>
void Transform::Scale(float a_fXScale, float a_fYScale, float a_fZScale)
{
	m_f3Scale.x *= a_fXScale;
	m_f3Scale.y *= a_fXScale;
	m_f3Scale.z *= a_fXScale;
	m_bDirty = true;
}
/// <summary>
/// Scales the transform by the given scalar
/// </summary>
/// <param name="a_f3Scale">Scalar</param>
void Transform::Scale(DirectX::XMFLOAT3 a_f3Scale)
{
	m_f3Scale.x *= a_f3Scale.x;
	m_f3Scale.y *= a_f3Scale.y;
	m_f3Scale.z *= a_f3Scale.z;
	m_bDirty = true;
}
#pragma endregion


