#pragma once
#include "Material.h"
#include <DirectXMath.h>
#include <memory>
#include "SimpleShader.h"

Material::Material(DirectX::XMFLOAT4 a_f4ColorTint, std::shared_ptr<SimpleVertexShader> a_spVertexShader, std::shared_ptr<SimplePixelShader> a_spPixelShader)
{
	m_f4ColorTint = a_f4ColorTint;
	m_spVertexShader = a_spVertexShader;
	m_spPixelShader = a_spPixelShader;
}

#pragma region Getters
/// <summary>
/// Gets the material's color tint
/// </summary>
/// <returns>Float4 color tint</returns>
DirectX::XMFLOAT4 Material::GetColorTint()
{
	return m_f4ColorTint;
}
/// <summary>
/// Gets the material's vertex shader
/// </summary>
/// <returns>Vertex shader</returns>
std::shared_ptr<SimpleVertexShader> Material::GetVertexShader()
{
	return m_spVertexShader;
}
/// <summary>
/// Gets the material's pixel shader
/// </summary>
/// <returns>pixel shader</returns>
std::shared_ptr<SimplePixelShader> Material::GetPixelShader()
{
	return m_spPixelShader;
}
#pragma endregion
#pragma region Setters
/// <summary>
/// Sets the material's color tint to the given float4 tint
/// </summary>
/// <param name="a_f4ColorTint">Color tint</param>
void Material::SetColorTint(DirectX::XMFLOAT4 a_f4ColorTint)
{
	m_f4ColorTint = a_f4ColorTint;
}
/// <summary>
/// Sets the material's color tint to the given RGBA values
/// </summary>
/// <param name="a_fRed">Red value</param>
/// <param name="a_fGreen">Green value</param>
/// <param name="a_fBlue">Blue value</param>
/// <param name="a_fAlpha">Alpha value</param>
void Material::SetColorTint(float a_fRed, float a_fGreen, float a_fBlue, float a_fAlpha)
{
	m_f4ColorTint = DirectX::XMFLOAT4(a_fRed, a_fGreen, a_fBlue, a_fAlpha);
}
/// <summary>
/// Sets the material's vertex shader to the given vertex shader
/// </summary>
/// <param name="a_spVertexShader">Vertex shader</param>
void Material::SetVertexShader(std::shared_ptr<SimpleVertexShader> a_spVertexShader)
{
	m_spVertexShader = a_spVertexShader;
}
/// <summary>
/// Sets the material's pixel shader to the given pixel shader
/// </summary>
/// <param name="a_spPixelShader">Pixel shader</param>
void Material::SetPixelShader(std::shared_ptr<SimplePixelShader> a_spPixelShader)
{
	m_spPixelShader = a_spPixelShader;
}
#pragma endregion
