#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include "Mesh.h"
#include <memory>
#include "Entity.h"
#include <vector>
#include "Camera.h"
#include "Lights.h"
#include "Sky.h"
#include "ShadowMap.h"

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
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> CreateSRVTextureArray(std::vector<Microsoft::WRL::ComPtr<ID3D11Texture2D>> a_vTextures);

	// Note the usage of ComPtr below
	//  - This is a smart pointer for objects that abide by the
	//     Component Object Model, which DirectX objects do
	//  - More info here: https://github.com/Microsoft/DirectXTK/wiki/ComPtr

	// Buffers to hold actual geometry data
	//Microsoft::WRL::ComPtr<ID3D11Buffer> m_cpConstantBuffer;

	// Shaders and shader-related constructs
	/*Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;*/

	DirectX::XMFLOAT3 m_f3AmbientLight;

#pragma region Shadow
	//Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_cpShadowDSV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_cpShadowSRV;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_cpShadowRasterizer;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_cpShadowSampler;
	//DirectX::XMFLOAT4X4 m_m4LightViewMatrix;
	//DirectX::XMFLOAT4X4 m_m4LightProjectionMatrix;
	//std::shared_ptr<SimpleVertexShader> m_spShadowVertexShader;
	//DirectX::XMFLOAT4X4 m_m4LightView;
	//DirectX::XMFLOAT4X4 m_m4LightProjection;
	std::vector<ShadowMap> m_vShadowMaps;
#pragma endregion

	


#pragma region Helper Functions
	void InitializeNewUIFrame(float a_fDeltaTime);
	void BuildUI();
#pragma endregion

#pragma region Game Objects
	std::vector<Entity> m_vEntities;
	std::vector<std::shared_ptr<Camera>> m_vCameras;
	std::shared_ptr<Camera> m_spActiveCamera;
	std::vector<Light> m_vLights;
	std::shared_ptr<Sky> m_spSkybox;
#pragma endregion
};

