#pragma once
#include <DirectXMath.h>
#include <memory>
#include "SimpleShader.h"
#include <unordered_map>

class Material
{
public:
	Material(DirectX::XMFLOAT4 a_f4ColorTint, std::shared_ptr<SimpleVertexShader> a_spVertexShader, std::shared_ptr<SimplePixelShader> a_spPixelShader);

	// getters
	DirectX::XMFLOAT4 GetColorTint();
	std::shared_ptr<SimpleVertexShader> GetVertexShader();
	std::shared_ptr<SimplePixelShader> GetPixelShader();
	DirectX::XMFLOAT2 GetUVScale();
	DirectX::XMFLOAT2 GetUVOffset();
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> GetTextureSRVs();
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11SamplerState>> GetSamplers();

	// setters
	void SetColorTint(DirectX::XMFLOAT4 a_f4ColorTint);
	void SetColorTint(float a_fRed, float a_fGreen, float a_fBlue, float a_fAlpha);
	void SetVertexShader(std::shared_ptr<SimpleVertexShader> a_spVertexShader);
	void SetPixelShader(std::shared_ptr<SimplePixelShader> a_spPixelShader);
	void SetUVScale(DirectX::XMFLOAT2 a_f2UVScale);
	void SetUVScale(float a_fU, float a_fV);
	void SetUVOffset(DirectX::XMFLOAT2 a_f2UVOffset);
	void SetUVOffset(float a_fU, float a_fV);

	void AddTextureSRV(std::string a_sShaderResourceName, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> a_cpTextureSRV);
	void AddSampler(std::string a_sShaderResourceName, Microsoft::WRL::ComPtr<ID3D11SamplerState> a_cpSampler);
	void PrepareMaterial();

private:
	DirectX::XMFLOAT4 m_f4ColorTint;
	std::shared_ptr<SimpleVertexShader> m_spVertexShader;
	std::shared_ptr<SimplePixelShader> m_spPixelShader;
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> m_htTextureSRVs;
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11SamplerState>> m_htSamplers;

	DirectX::XMFLOAT2 m_f2UVScale;
	DirectX::XMFLOAT2 m_f2UVOffset;
};