#include "ShadowMap.h"
#include <DirectXMath.h>
#include "Window.h"
#include "Graphics.h"

using namespace DirectX;

ShadowMap::ShadowMap(std::shared_ptr<Light> a_spLight, std::shared_ptr<SimpleVertexShader> a_spShadowVertexShader, int a_nResolution, float a_fProjectionSize, float a_fNearPlaneDistance, float a_fFarPlaneDistance, float a_fBackupDistance)
{
	m_nResolution = a_nResolution;
	m_spShadowVertexShader = a_spShadowVertexShader;

	// Create the actual texture that will be the shadow map
	D3D11_TEXTURE2D_DESC tdShadowDesc = {};
	tdShadowDesc.Width = a_nResolution; // Ideally a power of 2 (like 1024)
	tdShadowDesc.Height = a_nResolution; // Ideally a power of 2 (like 1024)
	tdShadowDesc.ArraySize = 1;
	tdShadowDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	tdShadowDesc.CPUAccessFlags = 0;
	tdShadowDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	tdShadowDesc.MipLevels = 1;
	tdShadowDesc.MiscFlags = 0;
	tdShadowDesc.SampleDesc.Count = 1;
	tdShadowDesc.SampleDesc.Quality = 0;
	tdShadowDesc.Usage = D3D11_USAGE_DEFAULT;
	//Microsoft::WRL::ComPtr<ID3D11Texture2D> cpShadowTexture;
	Graphics::Device->CreateTexture2D(&tdShadowDesc, 0, m_cpTexture.GetAddressOf());

	// Create the depth/stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC DepthStenciViewlDesc = {};
	DepthStenciViewlDesc.Format = DXGI_FORMAT_D32_FLOAT;
	DepthStenciViewlDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	DepthStenciViewlDesc.Texture2D.MipSlice = 0;
	Graphics::Device->CreateDepthStencilView(
		m_cpTexture.Get(),
		&DepthStenciViewlDesc,
		m_cpDepthStencilView.GetAddressOf());
	// Create the SRV for the shadow map
	D3D11_SHADER_RESOURCE_VIEW_DESC ShaderResourceViewDesc = {};
	ShaderResourceViewDesc.Format = DXGI_FORMAT_R32_FLOAT;
	ShaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	ShaderResourceViewDesc.Texture2D.MipLevels = 1;
	ShaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	Graphics::Device->CreateShaderResourceView(
		m_cpTexture.Get(),
		&ShaderResourceViewDesc,
		m_cpShaderResourceView.GetAddressOf());

	// create the light view matrix
	XMStoreFloat4x4(&m_m4View,
		XMMatrixLookToLH(
			-XMLoadFloat3(&a_spLight->Direction) * a_fBackupDistance, // Position: "Backing up" 20 units from origin 
			XMLoadFloat3(&a_spLight->Direction), // Direction: light's direction
			XMVectorSet(0, 1, 0, 0))); // Up: World up vector (Y axis)) 
	// create the light projection matrix
	XMStoreFloat4x4(&m_m4Projection,
		XMMatrixOrthographicLH(
			a_fProjectionSize,
			a_fProjectionSize,
			a_fNearPlaneDistance,
			a_fFarPlaneDistance));
	/*
	// create a rasterizer state for depth biasing
	D3D11_RASTERIZER_DESC RasterizerDescription = {};
	RasterizerDescription.FillMode = D3D11_FILL_SOLID;
	RasterizerDescription.CullMode = D3D11_CULL_BACK;
	RasterizerDescription.DepthClipEnable = true;
	RasterizerDescription.DepthBias = 1000; // Min. precision units, not world units!
	RasterizerDescription.SlopeScaledDepthBias = 1.0f; // Bias more based on slope
	Graphics::Device->CreateRasterizerState(&RasterizerDescription, &m_cpRasterizerState);

	// set up the shadow sampler
	D3D11_SAMPLER_DESC SamplerDescription = {};
	SamplerDescription.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	SamplerDescription.ComparisonFunc = D3D11_COMPARISON_LESS;
	SamplerDescription.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	SamplerDescription.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	SamplerDescription.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	SamplerDescription.BorderColor[0] = 1.0f; // Only need the first component
	Graphics::Device->CreateSamplerState(&SamplerDescription, &m_cpSamplerState);
	*/
}

void ShadowMap::Draw(std::vector<Entity> a_vEntities, Microsoft::WRL::ComPtr<ID3D11RasterizerState> a_cpShadowRasterizer)
{
	// set the render target's depth buffer to the shadow map
	Graphics::Context->ClearDepthStencilView(m_cpDepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0); // reset depth values to 1.0
	ID3D11RenderTargetView* nullRTV{};
	Graphics::Context->OMSetRenderTargets(1, &nullRTV, m_cpDepthStencilView.Get()); // set the shadow map as the depth buffer and unbind the back buffer
	Graphics::Context->PSSetShader(0, 0, 0); // unbind the pixel shader

	// create a viewport that matches the shadow map's resolution
	D3D11_VIEWPORT viewport = {};
	viewport.Width = (float)m_nResolution;
	viewport.Height = (float)m_nResolution;
	viewport.MaxDepth = 1.0f;
	Graphics::Context->RSSetViewports(1, &viewport);

	//enable the specialized rasterizer state for depth biasing
	Graphics::Context->RSSetState(a_cpShadowRasterizer.Get());

	// loop through the entities and draw them using the specialized shader
	m_spShadowVertexShader->SetShader();
	m_spShadowVertexShader->SetMatrix4x4("view", m_m4View);
	m_spShadowVertexShader->SetMatrix4x4("projection", m_m4Projection);
	// Loop and draw all entities
	for (auto& e : a_vEntities)
	{
		m_spShadowVertexShader->SetMatrix4x4("world", e.GetTransform()->GetWorldMatrix());
		m_spShadowVertexShader->CopyAllBufferData();
		// Draw the mesh directly to avoid the entity's material
		// Note: Your code may differ significantly here!
		e.GetMesh()->Draw();
	}

	// reset the pipeline
	viewport.Width = (float)Window::Width();
	viewport.Height = (float)Window::Height();
	Graphics::Context->RSSetViewports(1, &viewport);
	Graphics::Context->OMSetRenderTargets(
		1,
		Graphics::BackBufferRTV.GetAddressOf(),
		Graphics::DepthBufferDSV.Get());
	Graphics::Context->RSSetState(0);
}

#pragma region GETTERS
/// <summary>
/// Gets the shadow map's depth stencil view
/// </summary>
/// <returns>Depth stencil view</returns>
Microsoft::WRL::ComPtr<ID3D11DepthStencilView> ShadowMap::GetDSV()
{
	return m_cpDepthStencilView;
}
Microsoft::WRL::ComPtr<ID3D11Texture2D> ShadowMap::GetTexture()
{
	return m_cpTexture;
}
/// <summary>
/// Gets the shadow map's shader resource view
/// </summary>
/// <returns>Shader resource view</returns>
Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> ShadowMap::GetSRV()
{
	return m_cpShaderResourceView;
}
/// <summary>
/// Gets the shadow map's vertex shader
/// </summary>
/// <returns>Vertex Shader</returns>
std::shared_ptr<SimpleVertexShader> ShadowMap::GetVertexShader()
{
	return m_spShadowVertexShader;
}
/// <summary>
/// Gets the shadow map's view matrix
/// </summary>
/// <returns>View matrix</returns>
DirectX::XMFLOAT4X4 ShadowMap::GetViewMatrix()
{
	return m_m4View;
}
/// <summary>
/// Gets the shadow map's projection matrix
/// </summary>
/// <returns>Projection matrix</returns>
DirectX::XMFLOAT4X4 ShadowMap::GetProjectionMatrix()
{
	return m_m4Projection;
}
/// <summary>
/// Gets the shadow map's resolution
/// </summary>
/// <returns></returns>
int ShadowMap::GetResolution()
{
	return m_nResolution;
}
#pragma endregion


