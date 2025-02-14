#include "Game.h"
#include "Graphics.h"
#include "Vertex.h"
#include "Input.h"
#include "PathHelpers.h"
#include "Window.h"
#include "Mesh.h"
#include "BufferStructs.h"

#include <DirectXMath.h>

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
static float offset[3] = { 0.0f, 0.0f, 0.0f };
static float tint[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
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
		Graphics::Context->IASetInputLayout(inputLayout.Get());

		// Set the active vertex and pixel shaders
		//  - Once you start applying different shaders to different objects,
		//    these calls will need to happen multiple times per frame
		Graphics::Context->VSSetShader(vertexShader.Get(), 0, 0);
		Graphics::Context->PSSetShader(pixelShader.Get(), 0, 0);
	}

	//calcualate the amount of space our struct needs (next multiple of 16)
	//unsigned int size = sizeof(VertexShaderData);
	//size = (size + 15) / 16 * 16;
	//
	//// Describe the constant buffer
	//D3D11_BUFFER_DESC cbDesc = {}; // Sets struct to all zeros
	//cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	//cbDesc.ByteWidth = size; // Must be a multiple of 16
	//cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	//cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	//Graphics::Device->CreateBuffer(&cbDesc, 0, constantBuffer.GetAddressOf());
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
		Graphics::Device->CreatePixelShader(
			pixelShaderBlob->GetBufferPointer(),	// Pointer to blob's contents
			pixelShaderBlob->GetBufferSize(),		// How big is that data?
			0,										// No classes in this shader
			pixelShader.GetAddressOf());			// Address of the ID3D11PixelShader pointer

		Graphics::Device->CreateVertexShader(
			vertexShaderBlob->GetBufferPointer(),	// Get a pointer to the blob's contents
			vertexShaderBlob->GetBufferSize(),		// How big is that data?
			0,										// No classes in this shader
			vertexShader.GetAddressOf());			// The address of the ID3D11VertexShader pointer
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
		Graphics::Device->CreateInputLayout(
			inputElements,							// An array of descriptions
			2,										// How many elements in that array?
			vertexShaderBlob->GetBufferPointer(),	// Pointer to the code of a shader that uses this layout
			vertexShaderBlob->GetBufferSize(),		// Size of the shader code that uses this layout
			inputLayout.GetAddressOf());			// Address of the resulting ID3D11InputLayout pointer
	}
}


// --------------------------------------------------------
// Creates the geometry we're going to draw
// --------------------------------------------------------
void Game::CreateGeometry()
{
	// Create some temporary variables to represent colors
	// - Not necessary, just makes things more readable
	XMFLOAT4 red = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 green = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	XMFLOAT4 blue = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
	XMFLOAT4 black = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 white = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

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
	Vertex defaultTriangleVertices[] =
	{
		{ XMFLOAT3(+0.0f, +0.5f, +0.0f), red },
		{ XMFLOAT3(+0.5f, -0.5f, +0.0f), blue },
		{ XMFLOAT3(-0.5f, -0.5f, +0.0f), green },
	};

	Vertex squareVertices[] =
	{
		{ XMFLOAT3(+0.55f, +0.35f, +0.0f), red },
		{ XMFLOAT3(+0.85f, +0.35f, +0.0f), blue },
		{ XMFLOAT3(+0.85f, +0.85f, +0.0f), green },
		{ XMFLOAT3(+0.55f, +0.85f, +0.0f), blue },
	};

	Vertex diamondVertices[] =
	{
		{ XMFLOAT3(-0.50f, +0.85f, +0.0f), black },
		{ XMFLOAT3(-0.35f, +0.70f, +0.0f), white },
		{ XMFLOAT3(-0.50f, +0.55f, +0.0f), black },
		{ XMFLOAT3(-0.65f, +0.70f, +0.0f), white },
	};

	// Set up indices, which tell us which vertices to use and in which order
	// - This is redundant for just 3 vertices, but will be more useful later
	// - Indices are technically not required if the vertices are in the buffer 
	//    in the correct order and each one will be used exactly once
	// - But just to see how it's done...
	unsigned int defaultTriangleIndices[] = { 0, 1, 2 };
	unsigned int squareIndices[] = { 0, 3, 1, 3, 2 ,1 };
	unsigned int diamondIndices[] = { 0, 1, 3, 1, 2 ,3 };
	
	//create some meshes
	m_spDefaultTriangle = std::make_shared<Mesh>(defaultTriangleVertices, 3, defaultTriangleIndices, 3);
	m_spSquare = std::make_shared<Mesh>(squareVertices, 4, squareIndices, 6);
	m_spDiamond = std::make_shared<Mesh>(diamondVertices, 4, diamondIndices, 6);

	Mesh defaultTriangle = Mesh(defaultTriangleVertices, 3, defaultTriangleIndices, 3);

	m_vEntities.push_back(Entity(defaultTriangle)); 
}


// --------------------------------------------------------
// Handle resizing to match the new window size
//  - Eventually, we'll want to update our 3D camera
// --------------------------------------------------------
void Game::OnResize()
{
}


// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	// initialize ImGui frame
	InitializeNewUIFrame(deltaTime);

	//create UI
	BuildUI();

	// Example input checking: Quit if the escape key is pressed
	if (Input::KeyDown(VK_ESCAPE))
		Window::Quit();
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

	// Edit Constant buffer
	//VertexShaderData vsdData;
	//vsdData.colorTint = XMFLOAT4(tint);
	////vsdData.offset = XMFLOAT3(offset);
	//
	//D3D11_MAPPED_SUBRESOURCE mappedBuffer = {};
	//Graphics::Context->Map(constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer);
	//
	//memcpy(mappedBuffer.pData, &vsdData, sizeof(vsdData));
	//
	//Graphics::Context->Unmap(constantBuffer.Get(), 0);
	//
	//Graphics::Context->VSSetConstantBuffers(
	//	0, // the slot (register) to bind the buffer to
	//	1, // number of buffers to set right now
	//	constantBuffer.GetAddressOf()); // buffer adress

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
			m_vEntities[i].Draw(constantBuffer, XMFLOAT4(tint));
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

	// allows the user to select a number and doubles it
	static int number = 0;
	ImGui::SliderInt("Choose a number", &number, 0, 100);
	ImGui::Text("That number doubled is %i", number * 2);

	static bool notARobot = false;
	ImGui::Checkbox("I am not a robot", &notARobot);
	if (notARobot)
	{
		ImGui::Text("Thats just what a robot would say... ");
	}

	// allows the user to select their favorite color from ROYGBIV, and will judge their choice
	const char* colors[] = { "Red", "Orange", "Yellow", "Green", "Blue", "Indigo", "Violet" };
	static int selectedColorIndex = 0;
	if (ImGui::BeginListBox("Choose your favorite color"))
	{
		for (int i = 0; i < IM_ARRAYSIZE(colors); i++)
		{
			const bool is_selected = (selectedColorIndex == i);
			if (ImGui::Selectable(colors[i], is_selected))
				selectedColorIndex = i;

			/*
			if (item_highlight && ImGui::IsItemHovered())
				item_highlighted_idx = n;
			*/

			// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndListBox();
	}
	ImGui::Text("You selected: %s", colors[selectedColorIndex]);

	switch (selectedColorIndex)
	{
	case 0: ImGui::Text("Red is pretty cool"); break;
	case 1: ImGui::Text("Oh come on, no one's THAT into school spirit"); break;
	case 2: ImGui::Text("Not valid"); break;
	case 3: ImGui::Text("Bad choice"); break;
	case 4: ImGui::Text("Blue is incorrect"); break;
	case 5: ImGui::Text("That's not a real color"); break;
	case 6: ImGui::Text("Just say purple"); break;
	}

	// allow user to edit mesh offset/color
	ImGui::DragFloat3("Mesh Offset", offset, 0.1f, -1.0f, 1.0f);
	ImGui::ColorEdit4("Mesh Tint", tint);

	// display info about meshes
	if(ImGui::CollapsingHeader("Meshes", ImGuiTreeNodeFlags_None))
	{
		if (ImGui::CollapsingHeader("Triangle", ImGuiTreeNodeFlags_None))
		{
			ImGui::Text("Verticies %u", m_spDefaultTriangle->GetVertexCount());
			ImGui::Text("Indicies %u", m_spDefaultTriangle->GetIndexCount());
		}
		if (ImGui::CollapsingHeader("Square", ImGuiTreeNodeFlags_None))
		{
			ImGui::Text("Verticies %u", m_spSquare->GetVertexCount());
			ImGui::Text("Indicies %u", m_spSquare->GetIndexCount());
		}
		if (ImGui::CollapsingHeader("Diamond", ImGuiTreeNodeFlags_None))
		{
			ImGui::Text("Verticies %u", m_spDiamond->GetVertexCount());
			ImGui::Text("Indicies %u", m_spDiamond->GetIndexCount());
		}
	}
	ImGui::End(); // Ends the current window
}
#pragma endregion


