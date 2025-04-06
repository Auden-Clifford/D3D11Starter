#include "Sky.h"
#include "WICTextureLoader.h"
#include "Graphics.h"

using namespace DirectX;

Sky::Sky(
	std::shared_ptr<Mesh> a_spMesh, 
	Microsoft::WRL::ComPtr<ID3D11SamplerState> a_cpSamplerState,
	const wchar_t* a_wsRight,
	const wchar_t* a_wsLeft,
	const wchar_t* a_wsUp,
	const wchar_t* a_wsDown,
	const wchar_t* a_wsFront,
	const wchar_t* a_wsBack)
{
	m_spMesh = a_spMesh;
	m_cpSamplerState = a_cpSamplerState;

	// create the cubemap SRV
	m_cpTextureSRV = CreateCubemap(a_wsRight, a_wsLeft, a_wsUp, a_wsDown, a_wsFront, a_wsBack);

	// create rasterizer state
	D3D11_RASTERIZER_DESC rdRasterizerDescription = {};

	rdRasterizerDescription.FillMode = D3D11_FILL_SOLID;
	rdRasterizerDescription.CullMode = D3D11_CULL_FRONT;

	Graphics::Device->CreateRasterizerState(&rdRasterizerDescription, m_cpRasterizerState.GetAddressOf());

	// create depth stencil
	D3D11_DEPTH_STENCIL_DESC ddDepthStencilDescription = {};

	ddDepthStencilDescription.DepthEnable = true;
	ddDepthStencilDescription.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

	Graphics::Device->CreateDepthStencilState(&ddDepthStencilDescription, m_cpDepthStencilState.GetAddressOf());
}

// draws the skybox
void Sky::Draw(std::shared_ptr<Camera> a_spCamera)
{
	// change render states
	Graphics::Context->RSSetState(m_cpRasterizerState.Get());
	Graphics::Context->OMSetDepthStencilState(m_cpDepthStencilState.Get(), 0);

	// set the shaders
	m_spVertexShader->SetShader();
	m_spPixelShader->SetShader();

	// collect external data for vertex shader
	m_spVertexShader->SetMatrix4x4("view", a_spCamera->GetViewMatrix());
	m_spVertexShader->SetMatrix4x4("projection", a_spCamera->GetProjectionMatrix());

	// copy data to constant buffer
	m_spVertexShader->CopyAllBufferData();

	// sampler state and SRV
	m_spPixelShader->SetShaderResourceView("SkyTexture", m_cpTextureSRV);
	m_spPixelShader->SetSamplerState("BasicSampler", m_cpSamplerState);

	m_spMesh->Draw();

	// reset the render states
	Graphics::Context->RSSetState(nullptr);
	Graphics::Context->OMSetDepthStencilState(nullptr, 0);
}

// --------------------------------------------------------
// Author: Chris Cascioli
// Loads six individual textures (the six faces of a cube map), then
// creates a blank cube map and copies each of the six textures to
// another face.  Afterwards, creates a shader resource view for
// the cube map and cleans up all of the temporary resources.
// --------------------------------------------------------
Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Sky::CreateCubemap(
	const wchar_t* a_wsRight,
	const wchar_t* a_wsLeft,
	const wchar_t* a_wsUp,
	const wchar_t* a_wsDown,
	const wchar_t* a_wsFront,
	const wchar_t* a_wsBack)
{
	// Load the 6 textures into an array.
	// - We need references to the TEXTURES, not SHADER RESOURCE VIEWS!
	// - Explicitly NOT generating mipmaps, as we don't need them for the sky!
	// - Order matters here!  +X, -X, +Y, -Y, +Z, -Z
	Microsoft::WRL::ComPtr<ID3D11Texture2D> textures[6] = {};
	CreateWICTextureFromFile(Graphics::Device.Get(), a_wsRight, (ID3D11Resource**)textures[0].GetAddressOf(), 0);
	CreateWICTextureFromFile(Graphics::Device.Get(), a_wsLeft, (ID3D11Resource**)textures[1].GetAddressOf(), 0);
	CreateWICTextureFromFile(Graphics::Device.Get(), a_wsUp, (ID3D11Resource**)textures[2].GetAddressOf(), 0);
	CreateWICTextureFromFile(Graphics::Device.Get(), a_wsDown, (ID3D11Resource**)textures[3].GetAddressOf(), 0);
	CreateWICTextureFromFile(Graphics::Device.Get(), a_wsFront, (ID3D11Resource**)textures[4].GetAddressOf(), 0);
	CreateWICTextureFromFile(Graphics::Device.Get(), a_wsBack, (ID3D11Resource**)textures[5].GetAddressOf(), 0);

	// We'll assume all of the textures are the same color format and resolution,
	// so get the description of the first texture
	D3D11_TEXTURE2D_DESC faceDesc = {};
	textures[0]->GetDesc(&faceDesc);

	// Describe the resource for the cube map, which is simply 
	// a "texture 2d array" with the TEXTURECUBE flag set.  
	// This is a special GPU resource format, NOT just a 
	// C++ array of textures!!!
	D3D11_TEXTURE2D_DESC cubeDesc = {};
	cubeDesc.ArraySize = 6;            // Cube map!
	cubeDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE; // We'll be using as a texture in a shader
	cubeDesc.CPUAccessFlags = 0;       // No read back
	cubeDesc.Format = faceDesc.Format; // Match the loaded texture's color format
	cubeDesc.Width = faceDesc.Width;   // Match the size
	cubeDesc.Height = faceDesc.Height; // Match the size
	cubeDesc.MipLevels = 1;            // Only need 1
	cubeDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE; // This should be treated as a CUBE, not 6 separate textures
	cubeDesc.Usage = D3D11_USAGE_DEFAULT; // Standard usage
	cubeDesc.SampleDesc.Count = 1;
	cubeDesc.SampleDesc.Quality = 0;

	// Create the final texture resource to hold the cube map
	Microsoft::WRL::ComPtr<ID3D11Texture2D> cubeMapTexture;
	Graphics::Device->CreateTexture2D(&cubeDesc, 0, cubeMapTexture.GetAddressOf());

	// Loop through the individual face textures and copy them,
	// one at a time, to the cube map texure
	for (int i = 0; i < 6; i++)
	{
		// Calculate the subresource position to copy into
		unsigned int subresource = D3D11CalcSubresource(
			0,  // Which mip (zero, since there's only one)
			i,  // Which array element?
			1); // How many mip levels are in the texture?

		// Copy from one resource (texture) to another
		Graphics::Context->CopySubresourceRegion(
			cubeMapTexture.Get(),  // Destination resource
			subresource,           // Dest subresource index (one of the array elements)
			0, 0, 0,               // XYZ location of copy
			textures[i].Get(),     // Source resource
			0,                     // Source subresource index (we're assuming there's only one)
			0);                    // Source subresource "box" of data to copy (zero means the whole thing)
	}

	// At this point, all of the faces have been copied into the 
	// cube map texture, so we can describe a shader resource view for it
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = cubeDesc.Format;         // Same format as texture
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE; // Treat this as a cube!
	srvDesc.TextureCube.MipLevels = 1;        // Only need access to 1 mip
	srvDesc.TextureCube.MostDetailedMip = 0;  // Index of the first mip we want to see

	// Make the SRV
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cubeSRV;
	Graphics::Device->CreateShaderResourceView(cubeMapTexture.Get(), &srvDesc, cubeSRV.GetAddressOf());

	// Send back the SRV, which is what we need for our shaders
	return cubeSRV;
}
