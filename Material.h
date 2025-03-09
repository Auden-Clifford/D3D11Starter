#pragma once
#include <DirectXMath.h>
#include <memory>
#include "SimpleShader.h"

class Material
{
public:
	Material(DirectX::XMFLOAT4 a_f4ColorTint, std::shared_ptr<SimpleVertexShader> a_spVertexShader, std::shared_ptr<SimplePixelShader> a_spPixelShader);

	// getters
	DirectX::XMFLOAT4 GetColorTint();
	std::shared_ptr<SimpleVertexShader> GetVertexShader();
	std::shared_ptr<SimplePixelShader> GetPixelShader();

	// setters
	void SetColorTint(DirectX::XMFLOAT4 a_f4ColorTint);
	void SetColorTint(float a_fRed, float a_fGreen, float a_fBlue, float a_fAlpha);
	void SetVertexShader(std::shared_ptr<SimpleVertexShader> a_spVertexShader);
	void SetPixelShader(std::shared_ptr<SimplePixelShader> a_spPixelShader);

private:
	DirectX::XMFLOAT4 m_f4ColorTint;
	std::shared_ptr<SimpleVertexShader> m_spVertexShader;
	std::shared_ptr<SimplePixelShader> m_spPixelShader;
};