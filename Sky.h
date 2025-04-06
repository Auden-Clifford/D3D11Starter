#pragma once

#include <wrl/client.h>
#include <d3d11.h>
#include <memory>
#include "Mesh.h"
#include "SimpleShader.h"
#include "Camera.h"

class Sky
{
public:
	Sky(
		std::shared_ptr<Mesh> a_spMesh, 
		Microsoft::WRL::ComPtr<ID3D11SamplerState> a_cpSamplerState,
		const wchar_t* a_wsRight,
		const wchar_t* a_wsLeft,
		const wchar_t* a_wsUp,
		const wchar_t* a_wsDown,
		const wchar_t* a_wsFront,
		const wchar_t* a_wsBack);

	void Draw(std::shared_ptr<Camera> a_spCamera);
private:
	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_cpSamplerState;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_cpTextureSRV;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_cpDepthStencilState;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_cpRasterizerState;
	std::shared_ptr<Mesh> m_spMesh;
	std::shared_ptr<SimplePixelShader> m_spPixelShader;
	std::shared_ptr<SimpleVertexShader> m_spVertexShader;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> CreateCubemap(
		const wchar_t* a_wsRight,
		const wchar_t* a_wsLeft,
		const wchar_t* a_wsUp,
		const wchar_t* a_wsDown,
		const wchar_t* a_wsFront,
		const wchar_t* a_wsBack);
};