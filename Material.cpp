#pragma once
#include "Material.h"
#include <DirectXMath.h>
#include <memory>
#include "SimpleShader.h"
#include <algorithm>

Material::Material(DirectX::XMFLOAT4 a_f4ColorTint, std::shared_ptr<SimpleVertexShader> a_spVertexShader, std::shared_ptr<SimplePixelShader> a_spPixelShader, float a_fRoughness)
{
	m_f4ColorTint = a_f4ColorTint;
	m_spVertexShader = a_spVertexShader;
	m_spPixelShader = a_spPixelShader;
	m_fRoughness = std::clamp(a_fRoughness, 0.0f, 1.0f);

	// default UV scale and offset
	m_f2UVScale = DirectX::XMFLOAT2(1, 1);
	m_f2UVOffset = DirectX::XMFLOAT2(0, 0);
}

/// <summary>
/// Adds the given texture SRV to the SRV hashtable under the given key
/// </summary>
/// <param name="a_sShaderResourceName">Hash table key; name of the shader variable</param>
/// <param name="a_cpTextureSRV">ComPtr to the SRV</param>
void Material::AddTextureSRV(std::string a_sShaderResourceName, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> a_cpTextureSRV)
{
	m_htTextureSRVs.insert({ a_sShaderResourceName, a_cpTextureSRV });
}
/// <summary>
/// Adds the given sampler state to the sampler state hash table unde the given key
/// </summary>
/// <param name="a_sShaderResourceName">Hash table key; name of the shader variable</param>
/// <param name="a_cpSampler">ComPtr to the sampler state</param>
void Material::AddSampler(std::string a_sShaderResourceName, Microsoft::WRL::ComPtr<ID3D11SamplerState> a_cpSampler)
{
	m_htSamplers.insert({ a_sShaderResourceName,a_cpSampler });
}
/// <summary>
/// Binds this material's SRVs and sampler states to the pixel shader
/// </summary>
void Material::PrepareMaterial()
{
	for (auto& t : m_htTextureSRVs) { m_spPixelShader->SetShaderResourceView(t.first.c_str(), t.second); }
	for (auto& s : m_htSamplers) { m_spPixelShader->SetSamplerState(s.first.c_str(), s.second); }
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
/// <summary>
/// Gets the material's UV scale
/// </summary>
/// <returns>UV scale</returns>
DirectX::XMFLOAT2 Material::GetUVScale()
{
	return m_f2UVScale;
}
/// <summary>
/// Gets the material's UV offset
/// </summary>
/// <returns>UV offset</returns>
DirectX::XMFLOAT2 Material::GetUVOffset()
{
	return m_f2UVOffset;
}
/// <summary>
/// Gets this material's hash table of texture SRVs
/// </summary>
/// <returns>Hash table containing texture SRVs</returns>
std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> Material::GetTextureSRVs()
{
	return m_htTextureSRVs;
}
/// <summary>
/// Gets this material's hash table of sampler states
/// </summary>
/// <returns>Hash table containing sampler states</returns>
std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11SamplerState>> Material::GetSamplers()
{
	return m_htSamplers;
}
/// <summary>
/// Gets this material's roughness
/// </summary>
/// <returns></returns>
float Material::GetRoughness()
{
	return m_fRoughness;
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
/// <summary>
/// Sets the material's UV scale to the given scale
/// </summary>
/// <param name="a_f2UVScale">New UV sale</param>
void Material::SetUVScale(DirectX::XMFLOAT2 a_f2UVScale)
{
	m_f2UVScale = a_f2UVScale;
}
/// <summary>
/// Sets the material's UV scale to the given U and V values
/// </summary>
/// <param name="a_fU">U scale</param>
/// <param name="a_fV">V scale</param>
void Material::SetUVScale(float a_fU, float a_fV)
{
	m_f2UVScale = DirectX::XMFLOAT2(a_fU, a_fV);
}
/// <summary>
/// Sets the material's UV offset to the given offset
/// </summary>
/// <param name="a_f2UVOffset"></param>
void Material::SetUVOffset(DirectX::XMFLOAT2 a_f2UVOffset)
{
	m_f2UVOffset = a_f2UVOffset;
}
/// <summary>
/// Sets the material's UV offset to the given U and V offsets
/// </summary>
/// <param name="a_fU">U offset</param>
/// <param name="a_fV">V offset</param>
void Material::SetUVOffset(float a_fU, float a_fV)
{
	m_f2UVOffset = DirectX::XMFLOAT2(a_fU, a_fV);
}
void Material::SetRoughness(float a_fRoughness)
{
	m_fRoughness = a_fRoughness;
}
#pragma endregion
