#pragma once

#include <DirectXMath.h>

class Transform
{
public:
	// OOP stuff
	Transform();

	// Setters
	void SetPosition(float a_fXPosition, float a_fYPosition, float a_fZPosition);
	void SetPosition(DirectX::XMFLOAT3 a_f3Position);
	void SetRotation(float a_fPitch, float a_fYaw, float a_fRoll);
	void SetRotation(DirectX::XMFLOAT3 a_f3Rotation);
	void SetScale(float a_fXScale, float a_fYScale, float a_fZScale);
	void SetScale(DirectX::XMFLOAT3 a_f3Scale);

	// Getters
	DirectX::XMFLOAT3 GetPosition();
	DirectX::XMFLOAT3 GetPitchYawRoll();
	DirectX::XMFLOAT3 GetScale();
	DirectX::XMFLOAT4X4 GetWorldMatrix();
	DirectX::XMFLOAT4X4 GetWorldInverseTransposeMatrix();

	// Transformers (roll out!)
	void MoveAbsolute(float a_fXOffset, float a_fYOffset, float a_fZOffset);
	void MoveAbsolute(DirectX::XMFLOAT3 a_f3Offset);
	void Rotate(float a_fPitch, float a_fYaw, float a_fRoll);
	void Rotate(DirectX::XMFLOAT3 a_f3Rotation);
	void Scale(float a_fXScale, float a_fYScale, float a_fZScale);
	void Scale(DirectX::XMFLOAT3 a_f3Scale);


private:
	DirectX::XMFLOAT3 m_f3Position;
	DirectX::XMFLOAT3 m_f3Rotation;
	DirectX::XMFLOAT3 m_f3Scale;

	DirectX::XMFLOAT4X4 m_m4World;
	DirectX::XMFLOAT4X4 m_m4WorldInverseTranspose;

	bool m_bDirty;
};