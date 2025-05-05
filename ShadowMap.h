#pragma once

#include "Graphics.h"
#include "Lights.h"
#include <DirectXMath.h>
#include <memory>
#include "SimpleShader.h"
#include "Entity.h"

class ShadowMap
{
public:
	ShadowMap(std::shared_ptr<Light> a_spLight, std::shared_ptr<SimpleVertexShader> a_spShadowVertexShader, int a_nResolution, float a_fProjectionSize, float a_fNearPlaneDistance, float a_fFarPlaneDistance, float a_fBackupDistance);

	void Draw(std::vector<Entity> a_vEntities, Microsoft::WRL::ComPtr<ID3D11RasterizerState> a_cpShadowRasterizer);

	// getters
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> GetDSV();
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetSRV();
	Microsoft::WRL::ComPtr<ID3D11Texture2D> GetTexture();
	std::shared_ptr<SimpleVertexShader> GetVertexShader();
	DirectX::XMFLOAT4X4 GetViewMatrix();
	DirectX::XMFLOAT4X4 GetProjectionMatrix();
	int GetResolution();

private:
	//std::shared_ptr<Light> m_spLight;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_cpDepthStencilView;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_cpTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_cpShaderResourceView;
	//Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_cpRasterizerState;
	//Microsoft::WRL::ComPtr<ID3D11SamplerState> m_cpSamplerState;

	std::shared_ptr<SimpleVertexShader> m_spShadowVertexShader;

	DirectX::XMFLOAT4X4 m_m4View;
	DirectX::XMFLOAT4X4 m_m4Projection;

	int m_nResolution;
	//float m_fProjectionSize;
	//float m_fNearPlaneDistance;
	//float m_fFarPlaneDistance;
};