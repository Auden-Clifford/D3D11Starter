#include "Game.h"
#include "Graphics.h"
#include "Vertex.h"
#include "Input.h"
#include "PathHelpers.h"
#include "Window.h"
#include "Mesh.h"
#include "SimpleShader.h"
#include <DirectXMath.h>
#include "Material.h"
#include "WICTextureLoader.h"
#include <wrl/client.h>
#include "ShadowMap.h"

// This code assumes files are in "ImGui" subfolder!
// Adjust as necessary for your own folder structure and project setup
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"

// Needed for a helper function to load pre-compiled shader files
#pragma comment(lib, "d3dcompiler.lib")
#include <d3dcompiler.h>

// For the DirectX Math library
using namespace DirectX;

#pragma region Temporary Variables
static float backgroundColor[4] = { .015f, .020f, .030f, 0.0f };
static float demoWindowVisible = false;
// static float tint[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
static int nShadowMapResolution = 2048;
static float fLightProjectionSize = 20.0f;
static int nBlurRadius = 0;
#pragma endregion

// --------------------------------------------------------
// Called once per program, after the window and graphics API
// are initialized but before the game loop begins
// --------------------------------------------------------
void Game::Initialize()
{
	// Initialize ImGui itself & platform/renderer backends
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplWin32_Init(Window::Handle());
	ImGui_ImplDX11_Init(Graphics::Device.Get(), Graphics::Context.Get());
	// Pick a style (uncomment one of these 3)
	ImGui::StyleColorsDark(); 

	// Helper methods for loading shaders, creating some basic
	// geometry to draw and some simple camera matrices.
	//  - You'll be expanding and/or replacing these later
	LoadShaders();
	CreateGeometry();

	// create cameras
	m_vCameras.push_back(std::make_shared<Camera>(Window::AspectRatio(), XMFLOAT3(0.0f, 0.0f, -10.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), 45.0f, 0.1f, 500.0f, 10.0f, 0.01f));
	m_vCameras.push_back(std::make_shared<Camera>(Window::AspectRatio(), XMFLOAT3(10.0f, 5.0f, -10.0f), XMFLOAT3(XM_PI / 6.0f, -XM_PI / 6.0f, 0.0f), 20.0f, 0.1f, 500.0f, 10.0f, 0.01f));

	// set the first camera as active
	m_spActiveCamera = m_vCameras[0];

	// set the ambient light
	//m_f3AmbientLight = XMFLOAT3(0.31f, 0.19f, 0.32f);

	// set the other lights
	std::shared_ptr<Light> spDirectionalLight1 = std::make_shared<Light>();
	spDirectionalLight1->Type = LIGHT_TYPE_DIRECTIONAL;
	spDirectionalLight1->Direction = DirectX::XMFLOAT3(0.0f, -0.3f, -1.0f);
	spDirectionalLight1->Color = DirectX::XMFLOAT3(1.0f, 0.67f, 0.94f);
	spDirectionalLight1->Intensity = 5.0;
	m_vLights.push_back(*spDirectionalLight1.get());

	std::shared_ptr<Light> spDirectionalLight2 = std::make_shared<Light>();
	spDirectionalLight2->Type = LIGHT_TYPE_DIRECTIONAL;
	spDirectionalLight2->Direction = DirectX::XMFLOAT3(0.0f, 0.3f, 1.0f);
	spDirectionalLight2->Color = DirectX::XMFLOAT3(1.0f, 0.67f, 0.94f);
	spDirectionalLight2->Intensity = 0.25;
	m_vLights.push_back(*spDirectionalLight2.get());

	std::shared_ptr<Light> spDirectionalLight3 = std::make_shared<Light>();
	spDirectionalLight3->Type = LIGHT_TYPE_DIRECTIONAL;
	spDirectionalLight3->Direction = DirectX::XMFLOAT3(0.0f, -1.0f, 0.0f);
	spDirectionalLight3->Color = DirectX::XMFLOAT3(0.68f, 0.87f, 1.0f);
	spDirectionalLight3->Intensity = 0.5;
	m_vLights.push_back(*spDirectionalLight3.get());

	std::shared_ptr<Light> spDirectionalLight4 = std::make_shared<Light>();
	spDirectionalLight4->Type = LIGHT_TYPE_DIRECTIONAL;
	spDirectionalLight4->Direction = DirectX::XMFLOAT3(-1.0f, -0.3f, -0.5f);
	spDirectionalLight4->Color = DirectX::XMFLOAT3(1.0f, 0.67f, 0.94f);
	spDirectionalLight4->Intensity = 5.0;
	m_vLights.push_back(*spDirectionalLight4.get());

#pragma region Shadow mapping
	// create shadow maps
	std::shared_ptr<SimpleVertexShader> spShadowVertexShader = std::make_shared<SimpleVertexShader>(
		Graphics::Device, Graphics::Context, FixPath(L"ShadowMapVertexShader.cso").c_str());

	m_vShadowMaps.push_back(ShadowMap(spDirectionalLight1, spShadowVertexShader, nShadowMapResolution, fLightProjectionSize, 1.0f, 100.0f, 20.0f));
	m_vShadowMaps.push_back(ShadowMap(spDirectionalLight4, spShadowVertexShader, nShadowMapResolution, fLightProjectionSize, 1.0f, 100.0f, 20.0f));

	// create a rasterizer state for depth biasing
	D3D11_RASTERIZER_DESC shadowRastDesc = {};
	shadowRastDesc.FillMode = D3D11_FILL_SOLID;
	shadowRastDesc.CullMode = D3D11_CULL_BACK;
	shadowRastDesc.DepthClipEnable = true;
	shadowRastDesc.DepthBias = 1000; // Min. precision units, not world units!
	shadowRastDesc.SlopeScaledDepthBias = 1.0f; // Bias more based on slope
	Graphics::Device->CreateRasterizerState(&shadowRastDesc, &m_cpShadowRasterizer);

	// set up the shadow sampler
	D3D11_SAMPLER_DESC shadowSampDesc = {};
	shadowSampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	shadowSampDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
	shadowSampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.BorderColor[0] = 1.0f; // Only need the first component
	Graphics::Device->CreateSamplerState(&shadowSampDesc, &m_cpShadowSampler);
#pragma endregion
#pragma region Post process
	// Sampler state for post processing
	D3D11_SAMPLER_DESC ppSampDesc = {};
	ppSampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	ppSampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	ppSampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	ppSampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	ppSampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	Graphics::Device->CreateSamplerState(&ppSampDesc, m_cpPostProcessSampler.GetAddressOf());

	// create the render targets
	MakePostProcessRenderTargets();

	// load post process vertex shader
	m_spPostProcessVertexShader = std::make_shared<SimpleVertexShader>(
		Graphics::Device, Graphics::Context, FixPath(L"FullscreenVertexShader.cso").c_str());
	m_spBlurPixelShader = std::make_shared<SimplePixelShader>(
		Graphics::Device, Graphics::Context, FixPath(L"BlurPixelShader.cso").c_str());
#pragma endregion
	
	//Light PointLight1 = {};
	//PointLight1.Type = LIGHT_TYPE_POINT;
	////DirectionalLight5.Direction = DirectX::XMFLOAT3(0, 1, -1);
	//PointLight1.Position = DirectX::XMFLOAT3(-5, 5, 0);
	//PointLight1.Range = 20;
	//PointLight1.Color = DirectX::XMFLOAT3(1, 1, 0);
	//PointLight1.Intensity = 2;
	//m_vLights.push_back(PointLight1);
	//
	//Light PointLight2 = {};
	//PointLight2.Type = LIGHT_TYPE_POINT;
	////DirectionalLight5.Direction = DirectX::XMFLOAT3(0, 1, -1);
	//PointLight2.Position = DirectX::XMFLOAT3(25, -15, 3);
	//PointLight2.Range = 60;
	//PointLight2.Color = DirectX::XMFLOAT3(0.5f, 0.2f, 1);
	//PointLight2.Intensity = 0.5;
	//m_vLights.push_back(PointLight2);
	//
	//Light SpotLight1 = {};
	//SpotLight1.Type = LIGHT_TYPE_SPOT;
	//SpotLight1.Position = DirectX::XMFLOAT3(15, 2, 0);
	//SpotLight1.Range = 60;
	//SpotLight1.Direction = DirectX::XMFLOAT3(0, -1, 0);
	//SpotLight1.Color = DirectX::XMFLOAT3(0.2f, 1, 0.2f);
	//SpotLight1.Intensity = 2.0f;
	//SpotLight1.SpotInnerAngle = 0.2f;
	//SpotLight1.SpotOuterAngle = 0.5f;
	//m_vLights.push_back(SpotLight1);

	// Set initial graphics API state
	//  - These settings persist until we change them
	//  - Some of these, like the primitive topology & input layout, probably won't change
	//  - Others, like setting shaders, will need to be moved elsewhere later
	{
		// Tell the input assembler (IA) stage of the pipeline what kind of
		// geometric primitives (points, lines or triangles) we want to draw.  
		// Essentially: "What kind of shape should the GPU draw with our vertices?"
		Graphics::Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// Ensure the pipeline knows how to interpret all the numbers stored in
		// the vertex buffer. For this course, all of your vertices will probably
		// have the same layout, so we can just set this once at startup.
		//Graphics::Context->IASetInputLayout(inputLayout.Get());

		// Set the active vertex and pixel shaders
		//  - Once you start applying different shaders to different objects,
		//    these calls will need to happen multiple times per frame
		//Graphics::Context->VSSetShader(vertexShader.Get(), 0, 0);
		//Graphics::Context->PSSetShader(pixelShader.Get(), 0, 0);
	}
}


// --------------------------------------------------------
// Clean up memory or objects created by this class
// 
// Note: Using smart pointers means there probably won't
//       be much to manually clean up here!
// --------------------------------------------------------
Game::~Game()
{
	// ImGui clean up
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}


// --------------------------------------------------------
// Loads shaders from compiled shader object (.cso) files
// and also created the Input Layout that describes our 
// vertex data to the rendering pipeline. 
// - Input Layout creation is done here because it must 
//    be verified against vertex shader byte code
// - We'll have that byte code already loaded below
// --------------------------------------------------------
void Game::LoadShaders()
{
	/*
	// BLOBs (or Binary Large OBjects) for reading raw data from external files
	// - This is a simplified way of handling big chunks of external data
	// - Literally just a big array of bytes read from a file
	ID3DBlob* pixelShaderBlob;
	ID3DBlob* vertexShaderBlob;

	// Loading shaders
	//  - Visual Studio will compile our shaders at build time
	//  - They are saved as .cso (Compiled Shader Object) files
	//  - We need to load them when the application starts
	{
		// Read our compiled shader code files into blobs
		// - Essentially just "open the file and plop its contents here"
		// - Uses the custom FixPath() helper from Helpers.h to ensure relative paths
		// - Note the "L" before the string - this tells the compiler the string uses wide characters
		D3DReadFileToBlob(FixPath(L"PixelShader.cso").c_str(), &pixelShaderBlob);
		D3DReadFileToBlob(FixPath(L"VertexShader.cso").c_str(), &vertexShaderBlob);

		// Create the actual Direct3D shaders on the GPU
		//Graphics::Device->CreatePixelShader(
		//	pixelShaderBlob->GetBufferPointer(),	// Pointer to blob's contents
		//	pixelShaderBlob->GetBufferSize(),		// How big is that data?
		//	0,										// No classes in this shader
		//	pixelShader.GetAddressOf());			// Address of the ID3D11PixelShader pointer

		//Graphics::Device->CreateVertexShader(
		//	vertexShaderBlob->GetBufferPointer(),	// Get a pointer to the blob's contents
		//	vertexShaderBlob->GetBufferSize(),		// How big is that data?
		//	0,										// No classes in this shader
		//	vertexShader.GetAddressOf());			// The address of the ID3D11VertexShader pointer
	}

	// Create an input layout 
	//  - This describes the layout of data sent to a vertex shader
	//  - In other words, it describes how to interpret data (numbers) in a vertex buffer
	//  - Doing this NOW because it requires a vertex shader's byte code to verify against!
	//  - Luckily, we already have that loaded (the vertex shader blob above)
	{
		D3D11_INPUT_ELEMENT_DESC inputElements[2] = {};

		// Set up the first element - a position, which is 3 float values
		inputElements[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;				// Most formats are described as color channels; really it just means "Three 32-bit floats"
		inputElements[0].SemanticName = "POSITION";							// This is "POSITION" - needs to match the semantics in our vertex shader input!
		inputElements[0].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;	// How far into the vertex is this?  Assume it's after the previous element

		// Set up the second element - a color, which is 4 more float values
		inputElements[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;			// 4x 32-bit floats
		inputElements[1].SemanticName = "COLOR";							// Match our vertex shader input!
		inputElements[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;	// After the previous element

		// Create the input layout, verifying our description against actual shader code
		//Graphics::Device->CreateInputLayout(
		//	inputElements,							// An array of descriptions
		//	2,										// How many elements in that array?
		//	vertexShaderBlob->GetBufferPointer(),	// Pointer to the code of a shader that uses this layout
		//	vertexShaderBlob->GetBufferSize(),		// Size of the shader code that uses this layout
		//	inputLayout.GetAddressOf());			// Address of the resulting ID3D11InputLayout pointer
	}
	*/
	
}


// --------------------------------------------------------
// Creates the geometry we're going to draw
// --------------------------------------------------------
void Game::CreateGeometry()
{
	std::shared_ptr<SimpleVertexShader> spVertexShader = std::make_shared<SimpleVertexShader>(
		Graphics::Device, Graphics::Context, FixPath(L"VertexShader.cso").c_str());
	std::shared_ptr<SimplePixelShader> spPixelShaderSolid = std::make_shared<SimplePixelShader>(
		Graphics::Device, Graphics::Context, FixPath(L"PixelShader.cso").c_str());
	/*std::shared_ptr<SimplePixelShader> spPixelShaderMultiTexture = std::make_shared<SimplePixelShader>(
		Graphics::Device, Graphics::Context, FixPath(L"PixelShaderMultiTexture.cso").c_str());
	std::shared_ptr<SimplePixelShader> spPixelShaderUV = std::make_shared<SimplePixelShader>(
		Graphics::Device, Graphics::Context, FixPath(L"DebugUVsPS.cso").c_str());
	std::shared_ptr<SimplePixelShader> spPixelShaderNormals = std::make_shared<SimplePixelShader>(
		Graphics::Device, Graphics::Context, FixPath(L"DebugNormalsPS.cso").c_str());
	std::shared_ptr<SimplePixelShader> spPixelShaderCustom = std::make_shared<SimplePixelShader>(
		Graphics::Device, Graphics::Context, FixPath(L"CustomPS.cso").c_str());*/

	// Create some temporary variables to represent colors
	// - Not necessary, just makes things more readable
	/*XMFLOAT4 red = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 green = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	XMFLOAT4 blue = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
	XMFLOAT4 black = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 white = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 grey = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.5f);*/
	XMFLOAT4 white = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	// load textures
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvMetal;
	DirectX::CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/metal_plate/metal_plate_02_diff_4k.jpg").c_str(), nullptr, srvMetal.GetAddressOf());
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvMetalNormal;
	DirectX::CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/metal_plate/metal_plate_02_nor_gl_4k.jpg").c_str(), nullptr, srvMetalNormal.GetAddressOf());
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvMetalRough;
	DirectX::CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/metal_plate/metal_plate_02_rough_4k.jpg").c_str(), nullptr, srvMetalRough.GetAddressOf());
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvMetalMetalness;
	DirectX::CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/metal_plate/metal_plate_02_metal_4k.jpg").c_str(), nullptr, srvMetalMetalness.GetAddressOf());
	
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvBrick; 
	DirectX::CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/stone_brick_wall/stone_brick_wall_001_diff_4k.jpg").c_str(), nullptr, srvBrick.GetAddressOf());
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvBrickNormal;
	DirectX::CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/stone_brick_wall/stone_brick_wall_001_nor_gl_4k.jpg").c_str(), nullptr, srvBrickNormal.GetAddressOf());
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvBrickRough;
	DirectX::CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/stone_brick_wall/stone_brick_wall_001_rough_4k.jpg").c_str(), nullptr, srvBrickRough.GetAddressOf());
	
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvMetalSafety;
	DirectX::CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/metal_plate_safety/metal_plate_diff_4k.jpg").c_str(), nullptr, srvMetalSafety.GetAddressOf());
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvMetalSafetyNormal;														
	DirectX::CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/metal_plate_safety/metal_plate_nor_gl_4k.jpg").c_str(), nullptr, srvMetalSafetyNormal.GetAddressOf());
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvMetalSafetyRough;														 
	DirectX::CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/metal_plate_safety/metal_plate_rough_4k.jpg").c_str(), nullptr, srvMetalSafetyRough.GetAddressOf());
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvMetalSafetyMetalness;													 
	DirectX::CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/metal_plate_safety/metal_plate_metal_4k.jpg").c_str(), nullptr, srvMetalSafetyMetalness.GetAddressOf());
	
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvNoMetalness;
	DirectX::CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/default_metalness.png").c_str(), nullptr, srvNoMetalness.GetAddressOf());
	
	Microsoft::WRL::ComPtr <ID3D11SamplerState> cpSamplerState;
	D3D11_SAMPLER_DESC samplerDesc = {};

	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	Graphics::Device.Get()->CreateSamplerState(&samplerDesc, cpSamplerState.GetAddressOf());

	//Material mMatSolid = Material(red, spVertexShader, spPixelShaderSolid);
	//Material mMatUV = Material(white, spVertexShader, spPixelShaderUV);
	//Material mMatNormals = Material(white, spVertexShader, spPixelShaderNormals);
	//std::shared_ptr<Material> spMatCustom = std::make_shared<Material>(blue, spVertexShader, spPixelShaderCustom, 1.0f);
	//Material mMatCustom = Material(blue, spVertexShader, spPixelShaderCustom);

	std::shared_ptr<Material> spMatMetal = std::make_shared<Material>(white, spVertexShader, spPixelShaderSolid, 0.5f);
	spMatMetal->AddTextureSRV("Albedo", srvMetal);
	spMatMetal->AddTextureSRV("NormalMap", srvMetalNormal);
	spMatMetal->AddTextureSRV("RoughnessMap", srvMetalRough);
	spMatMetal->AddTextureSRV("MetalnessMap", srvMetalNormal);
	spMatMetal->AddSampler("BasicSampler", cpSamplerState);
	//spMatMetal->SetUVScale(5.0f, 5.0f);
	//spMatMetal->SetUVOffset(0.75f, 0.0f);

	std::shared_ptr<Material> spMatBrick = std::make_shared<Material>(white, spVertexShader, spPixelShaderSolid, 1.0f);
	spMatBrick->AddTextureSRV("Albedo", srvBrick);
	spMatBrick->AddTextureSRV("NormalMap", srvBrickNormal);
	spMatBrick->AddTextureSRV("RoughnessMap", srvBrickRough);
	spMatBrick->AddTextureSRV("MetalnessMap", srvNoMetalness);
	spMatBrick->AddSampler("BasicSampler", cpSamplerState);

	std::shared_ptr<Material> spMatMetalSafety = std::make_shared<Material>(white, spVertexShader, spPixelShaderSolid, 0.5f);
	spMatMetalSafety->AddTextureSRV("Albedo", srvMetalSafety);
	spMatMetalSafety->AddTextureSRV("NormalMap", srvMetalSafetyNormal);
	spMatMetalSafety->AddTextureSRV("RoughnessMap", srvMetalSafetyRough);
	spMatMetalSafety->AddTextureSRV("MetalnessMap", srvMetalSafetyMetalness);
	spMatMetalSafety->AddSampler("BasicSampler", cpSamplerState);
	//
	//std::shared_ptr<Material> spMatWood = std::make_shared<Material>(white, spVertexShader, spPixelShaderSolid, 0.1);
	////Material mWood = Material(white, spVertexShader, spPixelShaderSolid);
	//spMatWood->AddTextureSRV("Albedo", srvWood);
	//spMatWood->AddTextureSRV("NormalMap", srvWoodNormal);
	//spMatWood->AddSampler("BasicSampler", cpSamplerState);

	//std::shared_ptr<Material> spMatCrackedBrick = std::make_shared<Material>(white, spVertexShader, spPixelShaderMultiTexture, 0.4);
	////Material mCrackedBrick = Material(white, spVertexShader, spPixelShaderMultiTexture);
	//spMatCrackedBrick->AddTextureSRV("SurfaceTexture", srvBrick);
	//spMatCrackedBrick->AddTextureSRV("Decal", srvCrack);
	//spMatCrackedBrick->AddSampler("BasicSampler", cpSamplerState);

	std::shared_ptr<Mesh> spMeshCube = std::make_shared<Mesh>(FixPath("../../Assets/Models/cube.obj").c_str());
	std::shared_ptr<Mesh> spMeshHelix = std::make_shared<Mesh>(FixPath("../../Assets/Models/helix.obj").c_str());
	std::shared_ptr<Mesh> spMeshSphere = std::make_shared<Mesh>(FixPath("../../Assets/Models/sphere.obj").c_str());
	std::shared_ptr<Mesh> spMeshCylinder = std::make_shared<Mesh>(FixPath("../../Assets/Models/cylinder.obj").c_str());
	std::shared_ptr<Mesh> spMeshTorus = std::make_shared<Mesh>(FixPath("../../Assets/Models/torus.obj").c_str());
	std::shared_ptr<Mesh> spMeshQuadDouble = std::make_shared<Mesh>(FixPath("../../Assets/Models/quad_double_sided.obj").c_str());
	std::shared_ptr<Mesh> spMeshQuad = std::make_shared<Mesh>(FixPath("../../Assets/Models/quad.obj").c_str());

	// create skybox
	m_spSkybox = std::make_shared<Sky>(spMeshCube, cpSamplerState,
		FixPath(L"../../Assets/Skies/Clouds_Pink/right.png").c_str(),
		FixPath(L"../../Assets/Skies/Clouds_Pink/left.png").c_str(),
		FixPath(L"../../Assets/Skies/Clouds_Pink/up.png").c_str(),
		FixPath(L"../../Assets/Skies/Clouds_Pink/down.png").c_str(),
		FixPath(L"../../Assets/Skies/Clouds_Pink/front.png").c_str(),
		FixPath(L"../../Assets/Skies/Clouds_Pink/back.png").c_str());

	m_vEntities.push_back(Entity(spMeshCube, spMatMetal));
	m_vEntities[0].GetTransform()->SetPosition(-7.5f, 0.0f, 0.0f);
	m_vEntities.push_back(Entity(spMeshHelix, spMatMetal));
	m_vEntities[1].GetTransform()->SetPosition(-4.5f, 0.0f, 0.0f);
	m_vEntities.push_back(Entity(spMeshSphere, spMatMetal));
	m_vEntities[2].GetTransform()->SetPosition(-1.5f, 0.0f, 0.0f);
	m_vEntities.push_back(Entity(spMeshCylinder, spMatMetalSafety));
	m_vEntities[3].GetTransform()->SetPosition(1.5f, 0.0f, 0.0f);
	m_vEntities.push_back(Entity(spMeshTorus, spMatMetalSafety));
	m_vEntities[4].GetTransform()->SetPosition(4.5f, 0.0f, 0.0f);
	m_vEntities.push_back(Entity(spMeshQuadDouble, spMatMetalSafety));
	m_vEntities[5].GetTransform()->SetPosition(7.5f, 0.0f, 0.0f);
	//m_vEntities.push_back(Entity(spMeshQuad, spMatMetal));
	//m_vEntities[6].GetTransform()->SetPosition(18.0f, 0.0f, 0.0f);

	//m_vEntities.push_back(Entity(spMeshCube, spMatBrick));
	//m_vEntities[7].GetTransform()->SetPosition(0.0f, -3.0f, 0.0f);
	//m_vEntities.push_back(Entity(spMeshHelix, spMatBrick));
	//m_vEntities[8].GetTransform()->SetPosition(3.0f, -3.0f, 0.0f);
	//m_vEntities.push_back(Entity(spMeshSphere, spMatBrick));
	//m_vEntities[9].GetTransform()->SetPosition(6.0f, -3.0f, 0.0f);
	//m_vEntities.push_back(Entity(spMeshCylinder, spMatBrick));
	//m_vEntities[10].GetTransform()->SetPosition(9.0f, -3.0f, 0.0f);
	//m_vEntities.push_back(Entity(spMeshTorus, spMatBrick));
	//m_vEntities[11].GetTransform()->SetPosition(12.0f, -3.0f, 0.0f);
	//m_vEntities.push_back(Entity(spMeshQuadDouble, spMatBrick));
	//m_vEntities[12].GetTransform()->SetPosition(15.0f, -3.0f, 0.0f);
	m_vEntities.push_back(Entity(spMeshQuadDouble, spMatBrick));
	m_vEntities[6].GetTransform()->SetPosition(0.0f, -3.0f, 0.0f);
	m_vEntities[6].GetTransform()->SetScale(10.0f, 1.0f, 20.0f);
	//
	//m_vEntities.push_back(Entity(spMeshCube, spMatMetalSafety));
	//m_vEntities[14].GetTransform()->SetPosition(0.0f, -6.0f, 0.0f);
	//m_vEntities.push_back(Entity(spMeshHelix, spMatMetalSafety));
	//m_vEntities[15].GetTransform()->SetPosition(3.0f, -6.0f, 0.0f);
	//m_vEntities.push_back(Entity(spMeshSphere, spMatMetalSafety));
	//m_vEntities[16].GetTransform()->SetPosition(6.0f, -6.0f, 0.0f);
	//m_vEntities.push_back(Entity(spMeshCylinder, spMatMetalSafety));
	//m_vEntities[17].GetTransform()->SetPosition(9.0f, -6.0f, 0.0f);
	//m_vEntities.push_back(Entity(spMeshTorus, spMatMetalSafety));
	//m_vEntities[18].GetTransform()->SetPosition(12.0f, -6.0f, 0.0f);
	//m_vEntities.push_back(Entity(spMeshQuadDouble, spMatMetalSafety));
	//m_vEntities[19].GetTransform()->SetPosition(15.0f, -6.0f, 0.0f);
	//m_vEntities.push_back(Entity(spMeshQuad, spMatMetalSafety));
	//m_vEntities[20].GetTransform()->SetPosition(18.0f, -6.0f, 0.0f);

}




// --------------------------------------------------------
// Handle resizing to match the new window size
//  - Eventually, we'll want to update our 3D camera
// --------------------------------------------------------
void Game::OnResize()
{
	// resize cameras
	for (int i = 0; i < m_vCameras.size(); i++)
	{
		if (m_vCameras[i] != nullptr)
		{
			m_vCameras[i]->UpdateProjectionMatrix(Window::AspectRatio());
		}
	}

	if (Graphics::Device != nullptr)
	{
		//remake the post-process render targets with new screen size
		MakePostProcessRenderTargets();
	}
}


// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	// update camera

	m_spActiveCamera->Update(deltaTime);

	// initialize ImGui frame
	InitializeNewUIFrame(deltaTime);

	//create UI
	BuildUI();

	// Example input checking: Quit if the escape key is pressed
	if (Input::KeyDown(VK_ESCAPE))
		Window::Quit();

	
	m_vEntities[0].GetTransform()->SetPosition(m_vEntities[0].GetTransform()->GetPosition().x, (float)sin(totalTime) * 5, m_vEntities[0].GetTransform()->GetPosition().z);
	m_vEntities[1].GetTransform()->SetRotation(totalTime, 0.5f, 0.0f);
	m_vEntities[2].GetTransform()->SetPosition(m_vEntities[2].GetTransform()->GetPosition().x, m_vEntities[2].GetTransform()->GetPosition().y, (float)sin(totalTime) * 5);
	m_vEntities[3].GetTransform()->SetScale(sinf(totalTime)/ 2 + 1, sinf(totalTime) / 2 + 1, sinf(totalTime) / 2 + 1);
	m_vEntities[4].GetTransform()->SetPosition(m_vEntities[4].GetTransform()->GetPosition().x, sinf(totalTime) * 2, m_vEntities[4].GetTransform()->GetPosition().z);
	m_vEntities[4].GetTransform()->SetRotation(totalTime, 0.0f, 0.0f);
	//m_vEntities[4].GetTransform()->SetPosition(-sinf(totalTime), -sinf(totalTime), 0.0f);
	//m_vEntities[4].GetTransform()->SetScale(sinf(totalTime) + 1.5, sinf(totalTime) + 1.5, 0.0f);
	
	
}

 
// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	// draw SHADOW MAP
	{
		// loop through all the shadow maps and draw them
		for (auto& e : m_vShadowMaps)
		{
			e.Draw(m_vEntities, m_cpShadowRasterizer);
		}

		// create a Texture2DArray SRV from the shadow maps
		std::vector<Microsoft::WRL::ComPtr<ID3D11Texture2D>> vShadowMapTextures;
		for (auto& e : m_vShadowMaps)
		{
			vShadowMapTextures.push_back(e.GetTexture());
		}

		m_cpShadowSRV = CreateSRVTextureArray(vShadowMapTextures);
	}

	// Frame START
	// - These things should happen ONCE PER FRAME
	// - At the beginning of Game::Draw() before drawing *anything*
	{
		// Clear the back buffer (erase what's on screen) and depth buffer
		//static float color[4] = { 0.4f, 0.6f, 0.75f, 0.0f };
		Graphics::Context->ClearRenderTargetView(Graphics::BackBufferRTV.Get(),	backgroundColor);
		Graphics::Context->ClearDepthStencilView(Graphics::DepthBufferDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}

	Graphics::Context->ClearRenderTargetView(m_cpBlurRenderTargetView.Get(), backgroundColor);
	Graphics::Context->OMSetRenderTargets(1, m_cpBlurRenderTargetView.GetAddressOf(), Graphics::DepthBufferDSV.Get());


	// DRAW geometry
	// - These steps are generally repeated for EACH object you draw
	// - Other Direct3D calls will also be necessary to do more complex things
	{
		//draw all entities
		for (int i = 0; i < m_vEntities.size(); i++)
		{
			std::vector<XMFLOAT4X4> vShadowViews;
			std::vector<XMFLOAT4X4> vShadowProjections;

			// send the light view and projection to the vertex shader
			for (auto& e : m_vShadowMaps)
			{
				vShadowViews.push_back(e.GetViewMatrix());
				vShadowProjections.push_back(e.GetProjectionMatrix());
			}

			m_vEntities[i].GetMaterial()->GetVertexShader()->SetData("lightViews", &vShadowViews[0], sizeof(XMFLOAT4X4) * (int)vShadowViews.size());
			m_vEntities[i].GetMaterial()->GetVertexShader()->SetData("lightProjections", &vShadowProjections[0], sizeof(XMFLOAT4X4) * (int)vShadowProjections.size());
			//m_vEntities[i].GetMaterial()->GetVertexShader()->SetMatrix4x4("lightView", m_m4LightView);
			//m_vEntities[i].GetMaterial()->GetVertexShader()->SetMatrix4x4("lightProjection", m_m4LightProjection);
			m_vEntities[i].GetMaterial()->GetVertexShader()->CopyAllBufferData();

			// send the shadow sampler to the pixel shader
			m_vEntities[i].GetMaterial()->GetPixelShader()->SetSamplerState("ShadowSampler", m_cpShadowSampler);

			// send the ambient light to the entity's pixel shader
			m_vEntities[i].GetMaterial()->GetPixelShader()->SetFloat3("ambient", m_f3AmbientLight);

			// send light to entity's pixel shader
			m_vEntities[i].GetMaterial()->GetPixelShader()->SetData("lights", &m_vLights[0], sizeof(Light) * (int)m_vLights.size());
			m_vEntities[i].GetMaterial()->GetPixelShader()->CopyAllBufferData();

			// send shadow map to entity's pixel shader
			m_vEntities[i].GetMaterial()->GetPixelShader()->SetShaderResourceView("ShadowMaps", m_cpShadowSRV);
			//m_vEntities[i].GetMaterial()->GetPixelShader()->SetShaderResourceView("ShadowMap", m_vShadowMaps[0].GetSRV());
			

			m_vEntities[i].Draw(m_spActiveCamera, totalTime);
		}

		// draw the skybox
		m_spSkybox->Draw(m_spActiveCamera);
	}

	// DRAW POST PROCESS ///////////////////////////////
	{
		Graphics::Context->OMSetRenderTargets(1, Graphics::BackBufferRTV.GetAddressOf(), 0);

		m_spPostProcessVertexShader->SetShader();
		m_spBlurPixelShader->SetShader();

		// Activate shaders and bind resources
		// Also set any required cbuffer data (not shown)
		m_spBlurPixelShader->SetShaderResourceView("Pixels", m_cpBlurShaderResourceView.Get());
		m_spBlurPixelShader->SetSamplerState("ClampSampler", m_cpPostProcessSampler.Get());

		m_spBlurPixelShader->SetInt("blurRadius", nBlurRadius);
		m_spBlurPixelShader->SetFloat("pixelWidth", 1.0f / Window::Width());
		m_spBlurPixelShader->SetFloat("pixelHeight", 1.0f / Window::Height());
		m_spBlurPixelShader->CopyAllBufferData();

		// Tell Direct 3D to draw
		Graphics::Context->Draw(3, 0); // Draw exactly 3 vertices (one triangle)

		//Graphics::Context->OMSetRenderTargets(
		//	1,
		//	Graphics::BackBufferRTV.GetAddressOf(),
		//	Graphics::DepthBufferDSV.Get());
		//Graphics::Context->RSSetState(0);
	}

	ImGui::Render(); // Turns this frame’s UI into renderable triangles
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData()); // Draws it to the screen

	// Frame END
	// - These should happen exactly ONCE PER FRAME
	// - At the very end of the frame (after drawing *everything*)
	{
		// Present at the end of the frame
		bool vsync = Graphics::VsyncState();
		Graphics::SwapChain->Present(
			vsync ? 1 : 0,
			vsync ? 0 : DXGI_PRESENT_ALLOW_TEARING);

		// Re-bind back buffer and depth buffer after presenting
		Graphics::Context->OMSetRenderTargets(
			1,
			Graphics::BackBufferRTV.GetAddressOf(),
			Graphics::DepthBufferDSV.Get());

		// unbind SRVs
		ID3D11ShaderResourceView* nullSRVs[128] = {};
		Graphics::Context->PSSetShaderResources(0, 128, nullSRVs);
	}
}

#pragma region Helper Functions
/// <summary>
/// Updates the ImGui window
/// </summary>
/// <param name="deltaTime">time passed since last frame</param>
void Game::InitializeNewUIFrame(float a_fDeltaTime)
{
	// Feed fresh data to ImGui
	ImGuiIO& io = ImGui::GetIO();
	io.DeltaTime = a_fDeltaTime;
	io.DisplaySize.x = (float)Window::Width();
	io.DisplaySize.y = (float)Window::Height();
	// Reset the frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	// Determine new input capture
	Input::SetKeyboardCapture(io.WantCaptureKeyboard);
	Input::SetMouseCapture(io.WantCaptureMouse);
	// Show the demo window
	if (demoWindowVisible)
	{
		ImGui::ShowDemoWindow();
	}
}

void Game::BuildUI()
{
	ImGui::Begin("Inspector");
	// display the framerate
	ImGui::Text("Framerate: %f", ImGui::GetIO().Framerate);
	// display the dimentions of the game window
	ImGui::Text("Window Dimentions: %ux%u", Window::Width(), Window::Height());

	// change the background color
	ImGui::ColorEdit4("Background color editor", backgroundColor);

	// button toggles the demo window on and off
	if (ImGui::Button("Show ImGui Demo Window"))
	{
		if (demoWindowVisible)
		{
			demoWindowVisible = false;
		}
		else
		{
			demoWindowVisible = true;
		}
	}

	static int e = 0;
	if (ImGui::CollapsingHeader("Cameras", ImGuiTreeNodeFlags_None))
	{
		for (int i = 0; i < m_vCameras.size(); i++)
		{
			std::string sCameraName = "Camera " + std::to_string(i); 
			ImGui::Indent(0);
			ImGui::RadioButton(sCameraName.c_str(), &e, i); 
			ImGui::Indent();
			ImGui::Text("Position: %f, %f, %f",
				m_vCameras[i]->GetTransform()->GetPosition().x,
				m_vCameras[i]->GetTransform()->GetPosition().y,
				m_vCameras[i]->GetTransform()->GetPosition().z);
			ImGui::Text("Orientation: %f, %f, %f",
				m_vCameras[i]->GetTransform()->GetPitchYawRoll().x,
				m_vCameras[i]->GetTransform()->GetPitchYawRoll().y,
				m_vCameras[i]->GetTransform()->GetPitchYawRoll().z);
			ImGui::Text("FOV: %f", m_vCameras[i]->GetFieldOfView());
			ImGui::Unindent(); ImGui::Unindent();
		}
	}

	m_spActiveCamera = m_vCameras[e];

	// display info about entities
	if(ImGui::CollapsingHeader("Entities", ImGuiTreeNodeFlags_None))
	{
		ImGui::Indent();
		for (int i = 0; i < m_vEntities.size(); i++)
		{
			//create unique header name
			std::string sHeaderName = "Entity " + std::to_string(i);
			if (ImGui::CollapsingHeader(sHeaderName.c_str(), ImGuiTreeNodeFlags_None))
			{
				ImGui::Indent();
				ImGui::Text("Verticies %u", m_vEntities[i].GetMesh()->GetVertexCount());
				ImGui::Text("Indicies %u", m_vEntities[i].GetMesh()->GetIndexCount());

				// create a local variable that can be passed into ImGui
				XMFLOAT3 f3EntityPosition = m_vEntities[i].GetTransform()->GetPosition();
				//create unique ID for position editor
				std::string sPositionID = "Entity Position##" + std::to_string(i);
				ImGui::DragFloat3(sPositionID.c_str(), &f3EntityPosition.x, 0.1f);
				// set the position equal to the edited ImGui position
				m_vEntities[i].GetTransform()->SetPosition(f3EntityPosition);

				// create a local variable that can be passed into ImGui
				XMFLOAT3 f3EntityRotation = m_vEntities[i].GetTransform()->GetPitchYawRoll();
				//create unique ID for rotation editor
				std::string sRotationID = "Entity Rotation##" + std::to_string(i);
				ImGui::DragFloat3(sRotationID.c_str(), &f3EntityRotation.x, 0.1f);
				// set the rotation equal to the edited ImGui position
				m_vEntities[i].GetTransform()->SetRotation(f3EntityRotation);

				// create a local variable that can be passed into ImGui
				XMFLOAT3 f3EntityScale = m_vEntities[i].GetTransform()->GetScale();
				//create unique ID for scale editor
				std::string sScaleID = "Entity Scale##" + std::to_string(i);
				ImGui::DragFloat3(sScaleID.c_str(), &f3EntityScale.x, 0.1f);
				// set the scale equal to the edited ImGui position
				m_vEntities[i].GetTransform()->SetScale(f3EntityScale);
				ImGui::Unindent();

				// create a local variable that can be passed into ImGui
				XMFLOAT4 f4EntityTint = m_vEntities[i].GetMaterial()->GetColorTint();
				//create unique ID for tint editor
				std::string sTintID = "Entity Tint##" + std::to_string(i);
				ImGui::ColorEdit4(sTintID.c_str(), &f4EntityTint.x);
				// set the tint equal to the edited ImGui tint
				m_vEntities[i].GetMaterial()->SetColorTint(f4EntityTint);

				// create a local variable that can be passed into ImGui
				XMFLOAT2 f2EntityUVScale = m_vEntities[i].GetMaterial()->GetUVScale();
				//create unique ID for UV Scale editor
				std::string sUVScaleID = "Entity UV Scale##" + std::to_string(i);
				ImGui::DragFloat2(sUVScaleID.c_str(), &f2EntityUVScale.x, 0.01f);
				// set the UV Scale equal to the edited ImGui UV Scale
				m_vEntities[i].GetMaterial()->SetUVScale(f2EntityUVScale);

				// create a local variable that can be passed into ImGui
				XMFLOAT2 f2EntityUVOffset = m_vEntities[i].GetMaterial()->GetUVOffset();
				//create unique ID for UV Scale editor
				std::string sUVOffsetID = "Entity UV Offset##" + std::to_string(i);
				ImGui::DragFloat2(sUVOffsetID.c_str(), &f2EntityUVOffset.x, 0.01f);
				// set the UV Scale equal to the edited ImGui UV Scale
				m_vEntities[i].GetMaterial()->SetUVOffset(XMFLOAT2(f2EntityUVOffset));

				float fRoughness = m_vEntities[i].GetMaterial()->GetRoughness();
				//create unique ID for roughness
				std::string sRoughnessID = "Entity Roughness Offset##" + std::to_string(i);
				ImGui::DragFloat(sRoughnessID.c_str(), &fRoughness, 0.01f);
				// set the UV Scale equal to the edited ImGui UV Scale
				m_vEntities[i].GetMaterial()->SetRoughness(fRoughness);

				ImGui::Indent();
				//create unique header name
				std::string sHeaderName = "Textures" + std::to_string(i);
				if (ImGui::CollapsingHeader(sHeaderName.c_str(), ImGuiTreeNodeFlags_None)) 
				{
					// loop through all texture SRVs and display them
					for (auto& t : m_vEntities[i].GetMaterial()->GetTextureSRVs())
					{
						ImGui::Image((ImTextureID)(intptr_t)t.second.Get(), ImVec2(512, 512));
					}
				}
				ImGui::Unindent();
			}
		}
		ImGui::Unindent();
	}

	if (ImGui::CollapsingHeader("Lights", ImGuiTreeNodeFlags_None))
	{
		ImGui::Indent();
		for (int i = 0; i < m_vLights.size(); i++)
		{
			//create unique header name
			std::string sHeaderName = "Light " + std::to_string(i);
			if (ImGui::CollapsingHeader(sHeaderName.c_str(), ImGuiTreeNodeFlags_None))
			{
				ImGui::Indent();

				//create unique ID for rotation editor
				std::string sDirectionID = "Light Direction##" + std::to_string(i);
				ImGui::DragFloat3(sDirectionID.c_str(), &m_vLights[i].Direction.x, 0.1f);

				//create unique ID for position editor
				std::string sPositionID = "Light Position##" + std::to_string(i);
				ImGui::DragFloat3(sPositionID.c_str(), &m_vLights[i].Position.x, 1.0f);

				//create unique ID for tint editor
				std::string sTintID = "Light Color##" + std::to_string(i);
				ImGui::ColorEdit4(sTintID.c_str(), &m_vLights[i].Color.x);
			}
		}
		ImGui::Unindent();
	}

	ImGui::DragInt("BlurRadius", &nBlurRadius, 1.0f, 0, 10);

	for (auto& e : m_vShadowMaps)
	{
		ImGui::Image((ImTextureID)(intptr_t)e.GetSRV().Get(), ImVec2(256, 256));
	}

	ImGui::End(); // Ends the current window
}

Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Game::CreateSRVTextureArray(std::vector<Microsoft::WRL::ComPtr<ID3D11Texture2D>> a_vTextures)
{
	// create the resource
	D3D11_TEXTURE2D_DESC texture2DDesc = {};
	texture2DDesc.Width = nShadowMapResolution;
	texture2DDesc.Height = nShadowMapResolution;
	texture2DDesc.MipLevels = 1;
	texture2DDesc.ArraySize = a_vTextures.size(); // Number of shadow maps
	texture2DDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	texture2DDesc.Usage = D3D11_USAGE_DEFAULT;
	texture2DDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
	texture2DDesc.SampleDesc.Count = 1;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> cpTextureArray = nullptr;
	Graphics::Device->CreateTexture2D(&texture2DDesc, nullptr, cpTextureArray.GetAddressOf());

	// Loop through the individual shadow maps and copy them,
	// one at a time, to the cube map texure
	for (int i = 0; i < a_vTextures.size(); i++)
	{
		// Calculate the subresource position to copy into
		unsigned int subresource = D3D11CalcSubresource(
			0,  // Which mip (zero, since there's only one)
			i,  // Which array element?
			1); // How many mip levels are in the texture?

		// Copy from one resource (texture) to another
		Graphics::Context->CopySubresourceRegion(
			cpTextureArray.Get(),  // Destination resource
			subresource,           // Dest subresource index (one of the array elements)
			0, 0, 0,               // XYZ location of copy
			a_vTextures[i].Get(),     // Source resource
			0,                     // Source subresource index (we're assuming there's only one)
			0);                    // Source subresource "box" of data to copy (zero means the whole thing)
	}

	// descrive the SRV
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
	shaderResourceViewDesc.Format = DXGI_FORMAT_R32_FLOAT;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY; 
	shaderResourceViewDesc.Texture2DArray.MostDetailedMip = 0; 
	shaderResourceViewDesc.Texture2DArray.MipLevels = 1; 
	shaderResourceViewDesc.Texture2DArray.FirstArraySlice = 0; 
	shaderResourceViewDesc.Texture2DArray.ArraySize = a_vTextures.size();

	// make the SRV
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cpTextureArraySRV;
	Graphics::Device->CreateShaderResourceView(cpTextureArray.Get(), &shaderResourceViewDesc, cpTextureArraySRV.GetAddressOf());

	// return the SRV
	return cpTextureArraySRV;
}

void Game::MakePostProcessRenderTargets()
{
	// reset the render target ComPtrs
	m_cpBlurRenderTargetView.Reset();
	m_cpBlurShaderResourceView.Reset();

	// Describe the texture we're creating
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = Window::Width();
	textureDesc.Height = Window::Height();
	textureDesc.ArraySize = 1;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.MipLevels = 1;
	textureDesc.MiscFlags = 0;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;

	// Create the resource (no need to track it after the views are created below)
	Microsoft::WRL::ComPtr<ID3D11Texture2D> ppTexture;
	Graphics::Device->CreateTexture2D(&textureDesc, 0, ppTexture.GetAddressOf());

	// Create the Render Target View
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = textureDesc.Format;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	Graphics::Device->CreateRenderTargetView(
		ppTexture.Get(),
		&rtvDesc,
		m_cpBlurRenderTargetView.ReleaseAndGetAddressOf());
	// Create the Shader Resource View
	// By passing it a null description for the SRV, we
	// get a "default" SRV that has access to the entire resource
	Graphics::Device->CreateShaderResourceView(
		ppTexture.Get(),
		0,
		m_cpBlurShaderResourceView.ReleaseAndGetAddressOf());
}
#pragma endregion


