#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <memory>
#include <vector>
#include <DirectXMath.h>

#include "Mesh.h"
#include "Material.h"
#include "Entity.h"
#include "Camera.h"
#include "Lights.h"
#include "SimpleShader.h"

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
	void CreateMaterials();
	void CreateGeometry();
	void CreateLights();
	void CreateCameras();
	void InitializeSimulationParameters();
	void AddVertexShader(const wchar_t* _path);
	void AddPixelShader(const wchar_t* _path);
	void AddTexture(const wchar_t* _path);
	void AddMaterial(const char* _name, unsigned int _vertexShaderIndex, unsigned int _pixelShaderIndex, DirectX::XMFLOAT4 _colorTint, float _roughness);
	void AddMaterial(const char* _name, unsigned int _vertexShaderIndex, unsigned int _pixelShaderIndex, DirectX::XMFLOAT4 _colorTint);
	void AddMaterial(const char* _name, unsigned int _vertexShaderIndex, unsigned int _pixelShaderIndex, float _roughness);
	void AddMaterial(const char* _name, unsigned int _vertexShaderIndex, unsigned int _pixelShaderIndex);
	void AddEntity(const char* _name, unsigned int _meshIndex, unsigned int _materialIndex, DirectX::XMFLOAT3 _position);
	void AddLightDirectional(DirectX::XMFLOAT3 _direction, DirectX::XMFLOAT3 _color, float _intensity, bool _isActive);
	void AddLightPoint(DirectX::XMFLOAT3 _position, DirectX::XMFLOAT3 _color, float _intensity, float _range, bool _isActive);
	void AddLightSpot(DirectX::XMFLOAT3 _position, DirectX::XMFLOAT3 _direction, DirectX::XMFLOAT3 _color, float _intensity, float _range, float _innerAngle, float _outerAngle, bool _isActive);
	void AddCamera(const char* _name, DirectX::XMFLOAT3 _position, DirectX::XMFLOAT3 _rotation, float _aspect);
	void AddCamera(const char* _name, DirectX::XMFLOAT3 _position, DirectX::XMFLOAT3 _rotation, float _aspect, float _fov);
	void AddCamera(const char* _name, DirectX::XMFLOAT3 _position, DirectX::XMFLOAT3 _rotation, float _aspect, bool _isOrthographic);
	void AddCamera(const char* _name, DirectX::XMFLOAT3 _position, DirectX::XMFLOAT3 _rotation, float _aspect, bool _isOrthographic, float _orthoWidth);
	void SetGlobalSamplerState(D3D11_FILTER _filter, int _anisotropyLevel);
	void SetMaterialSamplerStates();
	void ImGuiUpdate(float deltaTime);

	// Update helper methods
	void ImGuiBuild();

	// Draw helper methods

	// Destructor helper methods
	void CleanupSimulationParameters();




	// SIMULATION PARAMETERS
	// All start with a "p" for "parameter"
	
	// The background color used by Direct3D
	float pBackgroundColor[4];
	// How fast to rotate the objects
	float pObjectRotationSpeed;

	// Filtering mode
	D3D11_FILTER pSamplerFilter;
	// Names for each filtering mode
	const char* SAMPLER_FILTER_STRINGS[6] = { "Point", "Linear Magnification", "Linear Minification", "Bilinear", "Trilinear", "Anisotropic" };
	// Corresponding filtering modes
	const D3D11_FILTER SAMPLER_FILTERS[6] = {
		D3D11_FILTER_MIN_MAG_MIP_POINT,
		D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT,
		D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT,
		D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT,
		D3D11_FILTER_MIN_MAG_MIP_LINEAR,
		D3D11_FILTER_ANISOTROPIC
	};
	// Selected array item
	int pSelectedSamplerFilter;
	// Level of anisotropy is 2 to this power
	int pAnisotropyPower;

	// Ambient light color
	DirectX::XMFLOAT3 pAmbientColor;
	// How many iterations the custom material should go through
	int pMatCustomIterations;
	// Where to center the image and the zoom-in on it
	DirectX::XMFLOAT2 pMatCustomImage;
	DirectX::XMFLOAT2 pMatCustomZoom;

	const char* LIGHT_TYPE_STRINGS[3] = { "Directional", "Point", "Spot" };

	// MESHES
	std::vector<std::shared_ptr<Mesh>> meshes;
	
	// TEXTURES
	std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> textures;
	// Pointer to the general sampler state
	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;

	// MATERIALS
	std::vector<std::shared_ptr<Material>> materials;

	// ENTITIES
	std::vector<std::shared_ptr<Entity>> entities;

	// LIGHTS
	std::vector<Light> lights;

	// CAMERAS
	std::vector<std::shared_ptr<Camera>> cameras;
	// Index of the current camera
	int pCameraCurrent;





	// IMGUI-SPECIFIC VARIABLES
	// All start with a "ig" for "ImGui"
	
	// Whether to show the ImGui demo
	bool igShowDemo;

	// Framerate Graph Variables

	// How many framerate samples are recorded for the graph
	const int IG_FRAME_GRAPH_TOTAL_SAMPLES = 1000;
	// Array of recorded framerate samples
	float* igFrameGraphSamples;
	// How many framerate samples are displayed on the graph
	int igFrameGraphSampleCount;
	// The rate at which new framerate samples should be added to the graph
	float igFrameGraphSampleRate;
	// The timestamp at which to add new framerate samples
	double igFrameGraphNextSampleTime;
	// The index of pFrameTimes to write the next sample to
	int igFrameGraphSampleOffset;
	// The highest recorded framerate; sets the scale of the framerate graph
	float igFrameGraphHighest;
	// Whether to continue sampling framerate for the graph
	bool igFrameGraphDoAnimate;


	// Note the usage of ComPtr below
	//  - This is a smart pointer for objects that abide by the
	//     Component Object Model, which DirectX objects do
	//  - More info here: https://github.com/Microsoft/DirectXTK/wiki/ComPtr

	// Shaders and shader-related constructs
	std::vector<std::shared_ptr<SimpleVertexShader>> vertexShaders;
	std::vector<std::shared_ptr<SimplePixelShader>> pixelShaders;
};

