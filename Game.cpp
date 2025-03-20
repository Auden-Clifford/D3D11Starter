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
static float backgroundColor[4] = { 0.4f, 0.6f, 0.75f, 0.0f };
static float demoWindowVisible = false;
// static float tint[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
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
	m_vCameras.push_back(std::make_shared<Camera>(Window::AspectRatio(), XMFLOAT3(10.0f, 5.0f, -10.0f), XMFLOAT3(XM_PI / 6, -XM_PI / 6, 0.0f), 20.0f, 0.1f, 500.0f, 10.0f, 0.01f));

	// set the first camera as active
	m_spActiveCamera = m_vCameras[0];

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
	std::shared_ptr<SimplePixelShader> spPixelShaderUV = std::make_shared<SimplePixelShader>(
		Graphics::Device, Graphics::Context, FixPath(L"DebugUVsPS.cso").c_str());
	std::shared_ptr<SimplePixelShader> spPixelShaderNormals = std::make_shared<SimplePixelShader>(
		Graphics::Device, Graphics::Context, FixPath(L"DebugNormalsPS.cso").c_str());
	std::shared_ptr<SimplePixelShader> spPixelShaderCustom = std::make_shared<SimplePixelShader>(
		Graphics::Device, Graphics::Context, FixPath(L"CustomPS.cso").c_str());

	// Create some temporary variables to represent colors
	// - Not necessary, just makes things more readable
	XMFLOAT4 red = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 green = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	XMFLOAT4 blue = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
	XMFLOAT4 black = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 white = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 grey = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.5f);

	// load textures
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cpSRV;

	HRESULT loadedTexture = DirectX::CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(L"../../Assets/Textures/metal_rusty_grid/rusty_metal_grid_diff_4k.jpg").c_str(), nullptr, cpSRV.GetAddressOf());

	Material mMatSolid = Material(red, spVertexShader, spPixelShaderSolid);
	Material mMatUV = Material(white, spVertexShader, spPixelShaderUV);
	Material mMatNormals = Material(white, spVertexShader, spPixelShaderNormals);
	Material mMatCustom = Material(blue, spVertexShader, spPixelShaderCustom);

	m_vEntities.push_back(Entity(Mesh(FixPath("../../Assets/Models/cube.obj").c_str()), mMatUV));
	m_vEntities.push_back(Entity(Mesh(FixPath("../../Assets/Models/helix.obj").c_str()), mMatUV));
	m_vEntities[1].GetTransform()->SetPosition(3.0f, 0.0f, 0.0f);
	m_vEntities.push_back(Entity(Mesh(FixPath("../../Assets/Models/sphere.obj").c_str()), mMatUV));
	m_vEntities[2].GetTransform()->SetPosition(6.0f, 0.0f, 0.0f);
	m_vEntities.push_back(Entity(Mesh(FixPath("../../Assets/Models/cylinder.obj").c_str()), mMatUV));
	m_vEntities[3].GetTransform()->SetPosition(9.0f, 0.0f, 0.0f);
	m_vEntities.push_back(Entity(Mesh(FixPath("../../Assets/Models/torus.obj").c_str()), mMatUV));
	m_vEntities[4].GetTransform()->SetPosition(12.0f, 0.0f, 0.0f);
	m_vEntities.push_back(Entity(Mesh(FixPath("../../Assets/Models/quad_double_sided.obj").c_str()), mMatUV));
	m_vEntities[5].GetTransform()->SetPosition(15.0f, 0.0f, 0.0f);
	m_vEntities.push_back(Entity(Mesh(FixPath("../../Assets/Models/quad.obj").c_str()), mMatUV));
	m_vEntities[6].GetTransform()->SetPosition(18.0f, 0.0f, 0.0f);

	m_vEntities.push_back(Entity(Mesh(FixPath("../../Assets/Models/cube.obj").c_str()), mMatNormals));
	m_vEntities[7].GetTransform()->SetPosition(0.0f, -3.0f, 0.0f);
	m_vEntities.push_back(Entity(Mesh(FixPath("../../Assets/Models/helix.obj").c_str()), mMatNormals));
	m_vEntities[8].GetTransform()->SetPosition(3.0f, -3.0f, 0.0f);
	m_vEntities.push_back(Entity(Mesh(FixPath("../../Assets/Models/sphere.obj").c_str()), mMatNormals));
	m_vEntities[9].GetTransform()->SetPosition(6.0f, -3.0f, 0.0f);
	m_vEntities.push_back(Entity(Mesh(FixPath("../../Assets/Models/cylinder.obj").c_str()), mMatNormals));
	m_vEntities[10].GetTransform()->SetPosition(9.0f, -3.0f, 0.0f);
	m_vEntities.push_back(Entity(Mesh(FixPath("../../Assets/Models/torus.obj").c_str()), mMatNormals));
	m_vEntities[11].GetTransform()->SetPosition(12.0f, -3.0f, 0.0f);
	m_vEntities.push_back(Entity(Mesh(FixPath("../../Assets/Models/quad_double_sided.obj").c_str()), mMatNormals));
	m_vEntities[12].GetTransform()->SetPosition(15.0f, -3.0f, 0.0f);
	m_vEntities.push_back(Entity(Mesh(FixPath("../../Assets/Models/quad.obj").c_str()), mMatNormals));
	m_vEntities[13].GetTransform()->SetPosition(18.0f, -3.0f, 0.0f);

	m_vEntities.push_back(Entity(Mesh(FixPath("../../Assets/Models/cube.obj").c_str()), mMatSolid));
	m_vEntities[14].GetTransform()->SetPosition(0.0f, -6.0f, 0.0f);
	m_vEntities.push_back(Entity(Mesh(FixPath("../../Assets/Models/helix.obj").c_str()), mMatSolid));
	m_vEntities[15].GetTransform()->SetPosition(3.0f, -6.0f, 0.0f);
	m_vEntities.push_back(Entity(Mesh(FixPath("../../Assets/Models/sphere.obj").c_str()), mMatSolid));
	m_vEntities[16].GetTransform()->SetPosition(6.0f, -6.0f, 0.0f);
	m_vEntities.push_back(Entity(Mesh(FixPath("../../Assets/Models/cylinder.obj").c_str()), mMatSolid));
	m_vEntities[17].GetTransform()->SetPosition(9.0f, -6.0f, 0.0f);
	m_vEntities.push_back(Entity(Mesh(FixPath("../../Assets/Models/torus.obj").c_str()), mMatSolid));
	m_vEntities[18].GetTransform()->SetPosition(12.0f, -6.0f, 0.0f);
	m_vEntities.push_back(Entity(Mesh(FixPath("../../Assets/Models/quad_double_sided.obj").c_str()), mMatSolid));
	m_vEntities[19].GetTransform()->SetPosition(15.0f, -6.0f, 0.0f);
	m_vEntities.push_back(Entity(Mesh(FixPath("../../Assets/Models/quad.obj").c_str()), mMatSolid));
	m_vEntities[20].GetTransform()->SetPosition(18.0f, -6.0f, 0.0f);

	m_vEntities.push_back(Entity(Mesh(FixPath("../../Assets/Models/cube.obj").c_str()), mMatCustom));
	m_vEntities[21].GetTransform()->SetPosition(0.0f, -9.0f, 0.0f);
	m_vEntities.push_back(Entity(Mesh(FixPath("../../Assets/Models/helix.obj").c_str()), mMatCustom));
	m_vEntities[22].GetTransform()->SetPosition(3.0f, -9.0f, 0.0f);
	m_vEntities.push_back(Entity(Mesh(FixPath("../../Assets/Models/sphere.obj").c_str()), mMatCustom));
	m_vEntities[23].GetTransform()->SetPosition(6.0f, -9.0f, 0.0f);
	m_vEntities.push_back(Entity(Mesh(FixPath("../../Assets/Models/cylinder.obj").c_str()), mMatCustom));
	m_vEntities[24].GetTransform()->SetPosition(9.0f, -9.0f, 0.0f);
	m_vEntities.push_back(Entity(Mesh(FixPath("../../Assets/Models/torus.obj").c_str()), mMatCustom));
	m_vEntities[25].GetTransform()->SetPosition(12.0f, -9.0f, 0.0f);
	m_vEntities.push_back(Entity(Mesh(FixPath("../../Assets/Models/quad_double_sided.obj").c_str()), mMatCustom));
	m_vEntities[26].GetTransform()->SetPosition(15.0f, -9.0f, 0.0f);
	m_vEntities.push_back(Entity(Mesh(FixPath("../../Assets/Models/quad.obj").c_str()), mMatCustom));
	m_vEntities[27].GetTransform()->SetPosition(18.0f, -9.0f, 0.0f);
	
	// Set up the vertices of the triangle we would like to draw
	// - We're going to copy this array, exactly as it exists in CPU memory
	//    over to a Direct3D-controlled data structure on the GPU (the vertex buffer)
	// - Note: Since we don't have a camera or really any concept of
	//    a "3d world" yet, we're simply describing positions within the
	//    bounds of how the rasterizer sees our screen: [-1 to +1] on X and Y
	// - This means (0,0) is at the very center of the screen.
	// - These are known as "Normalized Device Coordinates" or "Homogeneous 
	//    Screen Coords", which are ways to describe a position without
	//    knowing the exact size (in pixels) of the image/window/etc.  
	// - Long story short: Resizing the window also resizes the triangle,
	//    since we're describing the triangle in terms of the window itself
	/*
	Vertex defaultTriangleVertices[] =
	{
		{ XMFLOAT3(+0.0f, +0.5f, +0.0f), red },
		{ XMFLOAT3(+0.5f, -0.5f, +0.0f), blue },
		{ XMFLOAT3(-0.5f, -0.5f, +0.0f), green },
	};

	Vertex squareVertices[] =
	{
		{ XMFLOAT3(-0.15f, -0.15f, +0.0f), red },
		{ XMFLOAT3(+0.15f, -0.15f, +0.0f), blue },
		{ XMFLOAT3(+0.15f, +0.15f, +0.0f), green },
		{ XMFLOAT3(-0.15f, +0.15f, +0.0f), blue },
	};

	Vertex diamondVertices[] =
	{
		{ XMFLOAT3(+0.0f, +0.15f, +0.0f), black },
		{ XMFLOAT3(+0.15f, +0.0f, +0.0f), white },
		{ XMFLOAT3(+0.0f, -0.15f, +0.0f), black },
		{ XMFLOAT3(-0.15f, +0.0f, +0.0f), white },
	};
	*/

	// Set up indices, which tell us which vertices to use and in which order
	// - This is redundant for just 3 vertices, but will be more useful later
	// - Indices are technically not required if the vertices are in the buffer 
	//    in the correct order and each one will be used exactly once
	// - But just to see how it's done...
	/*
	unsigned int defaultTriangleIndices[] = { 0, 1, 2 };
	unsigned int squareIndices[] = { 0, 3, 1, 3, 2 ,1 };
	unsigned int diamondIndices[] = { 0, 1, 3, 1, 2 ,3 };
	
	//create some meshes
	
	Mesh defaultTriangle = Mesh(defaultTriangleVertices, 3, defaultTriangleIndices, 3);
	Mesh Square = Mesh(squareVertices, 4, squareIndices, 6);
	Mesh diamond = Mesh(diamondVertices, 4, diamondIndices, 6);

	m_vEntities.push_back(Entity(defaultTriangle, mMat1));  
	m_vEntities.push_back(Entity(Square, mMat2));
	m_vEntities.push_back(Entity(diamond, mMat3));
	m_vEntities.push_back(Entity(Square, mMat1));
	m_vEntities.push_back(Entity(diamond, mMat2));
	*/

}


// --------------------------------------------------------
// Handle resizing to match the new window size
//  - Eventually, we'll want to update our 3D camera
// --------------------------------------------------------
void Game::OnResize()
{
	for (int i = 0; i < m_vCameras.size(); i++)
	{
		if (m_vCameras[i] != nullptr)
		{
			m_vCameras[i]->UpdateProjectionMatrix(Window::AspectRatio());
		}
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

	/*
	//m_vEntities[0].GetTransform()->SetPosition((float)sin(totalTime), 0.0f, 0.0f);
	m_vEntities[1].GetTransform()->SetPosition(0.5f, 0.5f, 0.0f);
	m_vEntities[2].GetTransform()->SetRotation(0.0f, 0.0f, totalTime);
	m_vEntities[2].GetTransform()->SetPosition(sinf(totalTime), sinf(totalTime), 0.0f);
	m_vEntities[3].GetTransform()->SetPosition(-sinf(totalTime), sinf(totalTime), 0.0f);
	m_vEntities[4].GetTransform()->SetPosition(-sinf(totalTime), -sinf(totalTime), 0.0f);
	m_vEntities[4].GetTransform()->SetScale(sinf(totalTime) + 1.5, sinf(totalTime) + 1.5, 0.0f);
	*/
	
}

 
// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	// Frame START
	// - These things should happen ONCE PER FRAME
	// - At the beginning of Game::Draw() before drawing *anything*
	{
		// Clear the back buffer (erase what's on screen) and depth buffer
		//static float color[4] = { 0.4f, 0.6f, 0.75f, 0.0f };
		Graphics::Context->ClearRenderTargetView(Graphics::BackBufferRTV.Get(),	backgroundColor);
		Graphics::Context->ClearDepthStencilView(Graphics::DepthBufferDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}

	// DRAW geometry
	// - These steps are generally repeated for EACH object you draw
	// - Other Direct3D calls will also be necessary to do more complex things
	{
		//m_spDefaultTriangle->Draw();
		//m_spSquare->Draw();
		//m_spDiamond->Draw();

		//draw all entities
		for (int i = 0; i < m_vEntities.size(); i++)
		{
			m_vEntities[i].Draw(m_spActiveCamera, totalTime);
		}
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

	// display info about meshes
	if(ImGui::CollapsingHeader("Entities", ImGuiTreeNodeFlags_None))
	{
		ImGui::Indent();
		for (int i = 0; i < m_vEntities.size(); i++)
		{
			//create unique header name
			std::string sHeaderName = "Entity " + std::to_string(i);
			if (ImGui::CollapsingHeader(sHeaderName.c_str(), ImGuiTreeNodeFlags_None))
			{
				ImGui::Indent();;
				ImGui::Text("Verticies %u", m_vEntities[i].GetMesh()->GetVertexCount());
				ImGui::Text("Indicies %u", m_vEntities[i].GetMesh()->GetIndexCount());

				// turn the transform position into float[3] so it can be passed into ImGui
				float f3EntityPosition[3] = {
					m_vEntities[i].GetTransform()->GetPosition().x,
					m_vEntities[i].GetTransform()->GetPosition().y,
					m_vEntities[i].GetTransform()->GetPosition().z };

				//create unique ID for position editor
				std::string sPositionID = "Entity Position##" + std::to_string(i);
				ImGui::DragFloat3(sPositionID.c_str(), f3EntityPosition, 0.01f, -1.0f, 1.0f);

				// set the position equal to the edited ImGui position
				m_vEntities[i].GetTransform()->SetPosition(XMFLOAT3(f3EntityPosition));

				// turn the transform rotation into float[3] so it can be passed into ImGui
				float f3EntityRotation[3] = {
					m_vEntities[i].GetTransform()->GetPitchYawRoll().x,
					m_vEntities[i].GetTransform()->GetPitchYawRoll().y,
					m_vEntities[i].GetTransform()->GetPitchYawRoll().z };

				//create unique ID for rotation editor
				std::string sRotationID = "Entity Rotation##" + std::to_string(i);
				ImGui::DragFloat3(sRotationID.c_str(), f3EntityRotation, 0.1f);

				// set the position equal to the edited ImGui position
				m_vEntities[i].GetTransform()->SetRotation(XMFLOAT3(f3EntityRotation));

				// turn the transform scale into float[3] so it can be passed into ImGui
				float f3EntityScale[3] = {
					m_vEntities[i].GetTransform()->GetScale().x,
					m_vEntities[i].GetTransform()->GetScale().y,
					m_vEntities[i].GetTransform()->GetScale().z };

				//create unique ID for scale editor
				std::string sScaleID = "Entity Scale##" + std::to_string(i);
				ImGui::DragFloat3(sScaleID.c_str(), f3EntityScale, 0.1f);

				// set the position equal to the edited ImGui position
				m_vEntities[i].GetTransform()->SetScale(XMFLOAT3(f3EntityScale));
				ImGui::Unindent();

				// turn the tint color into float[4] so it can be passed into ImGui
				float f4EntityTint[4] = {
					m_vEntities[i].GetMaterial()->GetColorTint().x,
					m_vEntities[i].GetMaterial()->GetColorTint().y,
					m_vEntities[i].GetMaterial()->GetColorTint().z,
					m_vEntities[i].GetMaterial()->GetColorTint().w };

				//create unique ID for position editor
				std::string sTintID = "Entity Tint##" + std::to_string(i);
				ImGui::ColorEdit4(sTintID.c_str(), f4EntityTint);

				// set the position equal to the edited ImGui position
				m_vEntities[i].GetMaterial()->SetColorTint(XMFLOAT4(f4EntityTint));
			}
		}
		ImGui::Unindent();
	}
	ImGui::End(); // Ends the current window
}
#pragma endregion


