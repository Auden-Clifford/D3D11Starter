#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include "Mesh.h"
#include <memory>
#include "Entity.h"
#include <vector>
#include "Camera.h"

class Game
{
public:
	// Basic OOP setup
	Game() = default;
	~Game();
	Game(const Game&) = delete; // Remove copy constructor
	Game& operator=(const Game&) = delete; // Remove copy-assignment operator

	// Primary functions
	void Initialize();
	void Update(float deltaTime, float totalTime);
	void Draw(float deltaTime, float totalTime);
	void OnResize();

private:

	// Initialization helper methods - feel free to customize, combine, remove, etc.
	void LoadShaders();
	void CreateGeometry();

	// Note the usage of ComPtr below
	//  - This is a smart pointer for objects that abide by the
	//     Component Object Model, which DirectX objects do
	//  - More info here: https://github.com/Microsoft/DirectXTK/wiki/ComPtr

	// Buffers to hold actual geometry data
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_cpConstantBuffer;

	// Shaders and shader-related constructs
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;


#pragma region Helper Functions
	void InitializeNewUIFrame(float a_fDeltaTime);
	void BuildUI();
#pragma endregion

#pragma region Game Objects
	std::vector<Entity> m_vEntities;
	std::vector<std::shared_ptr<Camera>> m_vCameras;
	std::shared_ptr<Camera> m_spActiveCamera;
#pragma endregion
};

