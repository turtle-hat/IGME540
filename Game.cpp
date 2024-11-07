#include "Game.h"
#include "Graphics.h"
#include "Vertex.h"
#include "Input.h"
#include "PathHelpers.h"
#include "Window.h"
#include "WICTextureLoader.h"

#include <cmath>

// Needed for a helper function to load pre-compiled shader files
#pragma comment(lib, "d3dcompiler.lib")
#include <d3dcompiler.h>

// This code assumes files are in "ImGui" subfolder!
// Adjust as necessary for your own folder structure and project setup
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"

// For the DirectX Math library
using namespace std;
using namespace DirectX;

// --------------------------------------------------------
// Called once per program, after the window and graphics API
// are initialized but before the game loop begins
// --------------------------------------------------------
void Game::Initialize()
{
	// Helper methods for loading shaders, creating some basic
	// geometry to draw and some simple camera matrices.
	//  - You'll be expanding and/or replacing these later
	LoadShaders();
	CreateMaterials();
	CreateGeometry();
	CreateLights();
	CreateCameras();
	InitializeSimulationParameters();

	// Set initial graphics API state
	//  - These settings persist until we change them
	//  - Some of these, like the primitive topology & input layout, probably won't change
	//  - Others, like setting shaders, will need to be moved elsewhere later
	{
		// Tell the input assembler (IA) stage of the pipeline what kind of
		// geometric primitives (points, lines or triangles) we want to draw.  
		// Essentially: "What kind of shape should the GPU draw with our vertices?"
		Graphics::Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}

	// Initialize ImGui itself & platform/renderer backends
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplWin32_Init(Window::Handle());
	ImGui_ImplDX11_Init(Graphics::Device.Get(), Graphics::Context.Get());
	// Pick a style (uncomment one of these 3)
	//ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();
	ImGui::StyleColorsClassic();
}


// --------------------------------------------------------
// Clean up memory or objects created by this class
// 
// Note: Using smart pointers means there probably won't
//       be much to manually clean up here!
// --------------------------------------------------------
Game::~Game()
{
	// ImGui cleanup
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	// Cleanup other variables from helper methods
	CleanupSimulationParameters();
}


// --------------------------------------------------------
// Loads shaders from compiled shader object (.cso) files
// and creates the Input Layout that describes our vertex
// data to the rendering pipeline. 
// - Input Layout creation is done here because it must 
//    be verified against vertex shader byte code
// - We'll have that byte code already loaded below
// --------------------------------------------------------
void Game::LoadShaders()
{
	// Load shaders in with SimpleShader
	// VERTEX SHADERS 0-2
	AddVertexShader(L"VS_DiffuseSpecular.cso");
	AddVertexShader(L"VS_DiffuseNormal.cso");
	AddVertexShader(L"VS_Skybox.cso");
	// PIXEL SHADERS 0-1
	AddPixelShader(L"PS_DiffuseSpecular.cso");
	AddPixelShader(L"PS_DiffuseNormal.cso");
	// PIXEL SHADER 2
	AddPixelShader(L"PS_Skybox.cso");
	// PIXEL SHADERS 3-5
	AddPixelShader(L"PS_Normals.cso");
	AddPixelShader(L"PS_UVs.cso");
	AddPixelShader(L"PS_Custom.cso");
}

// --------------------------------------------------------
// Loads textures and creates materials
// --------------------------------------------------------
void Game::CreateMaterials()
{
	// Load textures
	// TEXTURES 0-2
	AddTexture(L"../../Assets/Textures/T_brokentiles_DS.png");
	AddTexture(L"../../Assets/Textures/T_rustymetal_DS.png");
	AddTexture(L"../../Assets/Textures/T_tiles_DS.png");
	// TEXTURES 3-9
	AddTexture(L"../../Assets/Textures/T_cobblestone_D.png");
	AddTexture(L"../../Assets/Textures/T_cobblestone_N.png");
	AddTexture(L"../../Assets/Textures/T_cushion_D.png");
	AddTexture(L"../../Assets/Textures/T_cushion_N.png");
	AddTexture(L"../../Assets/Textures/T_flat_N.png");
	AddTexture(L"../../Assets/Textures/T_rock_D.png");
	AddTexture(L"../../Assets/Textures/T_rock_N.png");

	// Set default sampler state settings
	pSamplerFilter = D3D11_FILTER_ANISOTROPIC;
	pAnisotropyPower = 4;

	// Create the sampler state
	SetGlobalSamplerState(pSamplerFilter, (int)pow(2, pAnisotropyPower));

	// Create materials
	// MATERIALS 0-2
	AddMaterial("Mat_Normals",		0, 3);
	AddMaterial("Mat_UVs",			0, 4);
	AddMaterial("Mat_Custom",		0, 5);

	// MATERIALS 3-5
	AddMaterial("Mat_BrokenTiles",	0, 0, 0.2f);
	materials[3]->AddTextureSRV("MapDiffuseSpecular", textures[0]);
	materials[3]->AddSampler("BasicSampler", samplerState);

	AddMaterial("Mat_RustyMetal",	0, 0, 0.8f);
	materials[4]->AddTextureSRV("MapDiffuseSpecular", textures[1]);
	materials[4]->AddSampler("BasicSampler", samplerState);

	AddMaterial("Mat_Tiles",		0, 0, 0.0f);
	materials[5]->AddTextureSRV("MapDiffuseSpecular", textures[2]);
	materials[5]->AddSampler("BasicSampler", samplerState);

	// MATERIALS 6-9
	AddMaterial("Mat_Cobblestone",	1, 1, 0.3f);
	materials[6]->AddTextureSRV("MapDiffuse", textures[3]);
	materials[6]->AddTextureSRV("MapNormal", textures[4]);
	materials[6]->AddSampler("BasicSampler", samplerState);

	AddMaterial("Mat_Cushion",		1, 1, 0.0f);
	materials[7]->AddTextureSRV("MapDiffuse", textures[5]);
	materials[7]->AddTextureSRV("MapNormal", textures[6]);
	materials[7]->AddSampler("BasicSampler", samplerState);
	materials[7]->SetUVScale(XMFLOAT2(3.0f, 3.0f));

	AddMaterial("Mat_Rock",			1, 1, 0.5f);
	materials[8]->AddTextureSRV("MapDiffuse", textures[8]);
	materials[8]->AddTextureSRV("MapNormal", textures[9]);
	materials[8]->AddSampler("BasicSampler", samplerState);

	AddMaterial("Mat_Flat",			1, 1, 0.0f);
	materials[9]->AddTextureSRV("MapDiffuse", textures[7]);	// Normal map used for diffuse
	materials[9]->AddTextureSRV("MapNormal", textures[7]);
	materials[9]->AddSampler("BasicSampler", samplerState);
}

// --------------------------------------------------------
// Creates the geometry we're going to draw and all
// entities in the scene
// --------------------------------------------------------
void Game::CreateGeometry()
{
	// Create meshes from OBJ models
	// MESHES 0-6
	meshes.push_back(std::make_shared<Mesh>("M_Cube", FixPath(L"../../Assets/Models/cube.obj").c_str()));
	meshes.push_back(std::make_shared<Mesh>("M_Cylinder", FixPath(L"../../Assets/Models/cylinder.obj").c_str()));
	meshes.push_back(std::make_shared<Mesh>("M_Helix", FixPath(L"../../Assets/Models/helix.obj").c_str()));
	meshes.push_back(std::make_shared<Mesh>("M_Quad-SingleSided", FixPath(L"../../Assets/Models/quad.obj").c_str()));
	meshes.push_back(std::make_shared<Mesh>("M_Quad-DoubleSided", FixPath(L"../../Assets/Models/quad_double_sided.obj").c_str()));
	meshes.push_back(std::make_shared<Mesh>("M_Sphere", FixPath(L"../../Assets/Models/sphere.obj").c_str()));
	meshes.push_back(std::make_shared<Mesh>("M_Torus", FixPath(L"../../Assets/Models/torus.obj").c_str()));

	// ENTITIES 0-6
	AddEntity("E_Normal_Cube",				0, 0, XMFLOAT3(-9.0f, 3.0f, 0.0f));
	AddEntity("E_Normal_Cylinder",			1, 0, XMFLOAT3(-6.0f, 3.0f, 0.0f));
	AddEntity("E_Normal_Helix",				2, 0, XMFLOAT3(-3.0f, 3.0f, 0.0f));
	AddEntity("E_Normal_Sphere",			5, 0, XMFLOAT3( 0.0f, 3.0f, 0.0f));
	AddEntity("E_Normal_Torus",				6, 0, XMFLOAT3( 3.0f, 3.0f, 0.0f));
	AddEntity("E_Normal_Quad-SingleSided",	3, 0, XMFLOAT3( 6.0f, 3.0f, 0.0f));
	AddEntity("E_Normal_Quad-DoubleSided",	4, 0, XMFLOAT3( 9.0f, 2.0f, 0.0f));

	// ENTITIES 7-13
	AddEntity("E_UV_Cube",				0, 1, XMFLOAT3(-9.0f,  0.0f, 0.0f));
	AddEntity("E_UV_Cylinder",			1, 1, XMFLOAT3(-6.0f,  0.0f, 0.0f));
	AddEntity("E_UV_Helix",				2, 1, XMFLOAT3(-3.0f,  0.0f, 0.0f));
	AddEntity("E_UV_Sphere",			5, 1, XMFLOAT3( 0.0f,  0.0f, 0.0f));
	AddEntity("E_UV_Torus",				6, 1, XMFLOAT3( 3.0f,  0.0f, 0.0f));
	AddEntity("E_UV_Quad-SingleSided",	3, 1, XMFLOAT3( 6.0f, -1.0f, 0.0f));
	AddEntity("E_UV_Quad-DoubleSided",	4, 1, XMFLOAT3( 9.0f, -1.0f, 0.0f));

	// ENTITIES 14-20
	AddEntity("E_Mat_Cube",				0, 6, XMFLOAT3(-9.0f, -3.0f, 0.0f));
	AddEntity("E_Mat_Cylinder",			1, 7, XMFLOAT3(-6.0f, -3.0f, 0.0f));
	AddEntity("E_Mat_Helix",			2, 8, XMFLOAT3(-3.0f, -3.0f, 0.0f));
	AddEntity("E_Mat_Sphere",			5, 8, XMFLOAT3( 0.0f, -3.0f, 0.0f));
	AddEntity("E_Mat_Torus",			6, 7, XMFLOAT3( 3.0f, -3.0f, 0.0f));
	AddEntity("E_Mat_Quad-SingleSided",	3, 6, XMFLOAT3( 6.0f, -4.0f, 0.0f));
	AddEntity("E_Mat_Quad-DoubleSided",	4, 9, XMFLOAT3( 9.0f, -4.0f, 0.0f));

	/*
	// ENTITY 21
	AddEntity("E_TestCanvas", 6, 2, XMFLOAT3(0.0f, 0.0f, -3.0f));
	entities[21]->GetTransform()->SetRotation(-XM_PIDIV2, 0.0f, 0.0f);
	*/
}

// --------------------------------------------------------
// Creates the lights to be rendered in the scene
// --------------------------------------------------------
void Game::CreateLights() {
	// Create lights
	// LIGHT 0
	AddLightDirectional(XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 1.0f, true);
	// LIGHTS 1-2
	AddLightDirectional(XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 1.0f, true);
	AddLightDirectional(XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 1.0f, true);
	// LIGHTS 3-4
	AddLightPoint(XMFLOAT3(-7.0f, -2.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 1.0f), 1.0f, 10.0f, true);
	AddLightPoint(XMFLOAT3(1.0f, -4.0f, 0.0f), XMFLOAT3(1.0f, 0.2f, 1.0f), 2.0f, 4.0f, true);
	// LIGHT 5
	AddLightSpot(XMFLOAT3(4.5f, 0.0f, 0.25f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), 1.0f, 30.0f, 0.35f, 0.4f, true);
}

// --------------------------------------------------------
// Creates the cameras we'll need in the scene
// --------------------------------------------------------
void Game::CreateCameras() {
	// Create cameras
	float aspect = (Window::Width() + 0.0f) / Window::Height();
	// CAMERAS 0-3
	AddCamera("C_Main",		XMFLOAT3(0.0f, 0.0f, -5.0f),	XMFLOAT3(0.0f, 0.0f, 0.0f),					aspect, false);
	AddCamera("C_OrthoYZ",	XMFLOAT3(100.0f, 0.0f, 0.0f),	XMFLOAT3(0.0f, -XM_PIDIV2, 0.0f),			aspect, true);
	cameras[1]->SetLookSpeed(1.0f);
	AddCamera("C_OrthoXZ",	XMFLOAT3(0.0f, 100.0f, 0.0f),	XMFLOAT3(XM_PIDIV2 - 0.001f, 0.0f, 0.0f),	aspect, true);
	cameras[2]->SetLookSpeed(1.0f);
	AddCamera("C_OrthoXY",	XMFLOAT3(0.0f, 0.0f, -100.0f),	XMFLOAT3(0.0f, 0.0f, 0.0f),					aspect, true);
	cameras[3]->SetLookSpeed(1.0f);
}

// --------------------------------------------------------
// Handle resizing to match the new window size
//  - Eventually, we'll want to update our 3D camera
// --------------------------------------------------------
void Game::OnResize()
{
	// If cameras exist, resize it
	if (cameras.size() > 0) {
		cameras[pCameraCurrent]->SetAspect((Window::Width() + 0.0f) / Window::Height());
	}
}


// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	// Update current camera
	cameras[pCameraCurrent]->Update(deltaTime);

	// Rotate OBJ-imported meshes
	for (int i = 0; i <= 20; i++) {
		entities[i]->GetTransform()->Rotate(0.0f, deltaTime * pObjectRotationSpeed, 0.0f);
	}

	// Scroll and scale UV of Mat_RustyMetal
	materials[4]->SetUVPosition(XMFLOAT2(sin(totalTime), cos(totalTime)));
	materials[4]->SetUVScale(XMFLOAT2(sin(totalTime * 0.5f) * 0.5f + 1.0f, cos(totalTime * 0.5f) * 0.5f + 1.0f));

	ImGuiUpdate(deltaTime);
	ImGuiBuild();

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
		Graphics::Context->ClearRenderTargetView(Graphics::BackBufferRTV.Get(),	pBackgroundColor);
		Graphics::Context->ClearDepthStencilView(Graphics::DepthBufferDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}
	
	// Loop through every entity and draw it
	for (int i = 0; i < entities.size(); i++) {
		//entities[i]->Draw(cameras[pCameraCurrent]);

		// Get entity material
		std::shared_ptr<Material> material = entities[i]->GetMaterial();
		// Prepare the material for drawing
		material->PrepareMaterial();

		// Get entity's shaders
		std::shared_ptr<SimpleVertexShader> vs = material->GetVertexShader();
		std::shared_ptr<SimplePixelShader> ps = material->GetPixelShader();


		// Set vertex and pixel shaders
		vs->SetShader();
		ps->SetShader();

		// Fill constant buffers with entity's data
		// VERTEX
		vs->SetMatrix4x4("tfWorld", entities[i]->GetTransform()->GetWorld());
		vs->SetMatrix4x4("tfView", cameras[pCameraCurrent]->GetViewMatrix());
		vs->SetMatrix4x4("tfProjection", cameras[pCameraCurrent]->GetProjectionMatrix());
		vs->SetMatrix4x4("tfWorldIT", entities[i]->GetTransform()->GetWorldInverseTranspose());
		// PIXEL
		ps->SetFloat4("colorTint", material->GetColorTint());
		ps->SetFloat("roughness", material->GetRoughness());
		ps->SetFloat3("cameraPosition", cameras[pCameraCurrent]->GetTransform()->GetPosition());

		ps->SetFloat2("uvPosition", material->GetUVPosition());
		ps->SetFloat2("uvScale", material->GetUVScale());

		// Set lights on pixel shader
		ps->SetData("lights", &lights[0], sizeof(Light) * (int)lights.size());
		// MATERIAL-SPECIFIC PIXEL SHADER CONSTANT BUFFER INPUTS
		if (material->GetName() == "Mat_Custom") {
			ps->SetFloat("totalTime", totalTime);
			ps->SetFloat2("imageCenter", pMatCustomImage);
			ps->SetFloat2("zoomCenter", pMatCustomZoom);
			ps->SetInt("maxIterations", pMatCustomIterations);
		}
		ps->SetFloat3("lightAmbient", pAmbientColor);

		// COPY DATA TO CONSTANT BUFFERS
		vs->CopyAllBufferData();
		ps->CopyAllBufferData();

		// Draw the entity's mesh
		entities[i]->GetMesh()->Draw();
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



// --------------------------------------------------------
// 
//                 CUSTOM HELPER METHODS
// 
// --------------------------------------------------------



// --------------------------------------------------------
// Initializes the all simulation variables
// to their default values
// --------------------------------------------------------
void Game::InitializeSimulationParameters() {
	
	// IMGUI PARAMETERS

	igShowDemo = false;
	float bgColor[4] = { 0.4f, 0.6f, 0.75f, 1.0f };
	memcpy(pBackgroundColor, bgColor, sizeof(float) * 4);
	pObjectRotationSpeed = 1.0f;

	pSelectedSamplerFilter = 5;
	pAmbientColor = XMFLOAT3(0.1f, 0.1f, 0.25f);

	pMatCustomIterations = 100;
	pMatCustomImage = XMFLOAT2(-1.77f, -0.02f);
	pMatCustomZoom = XMFLOAT2(-0.2f, -0.61f);

	pCameraCurrent = 0;

	// Framerate graph variables
	igFrameGraphSamples = new float[IG_FRAME_GRAPH_TOTAL_SAMPLES];
	// Zero out all of our sample array
	for (int i = 0; i < IG_FRAME_GRAPH_TOTAL_SAMPLES; i++) {
		igFrameGraphSamples[i] = 0;
	}
	igFrameGraphSampleCount = 240;
	igFrameGraphSampleRate = 60.0f;
	igFrameGraphNextSampleTime = 0.0;
	igFrameGraphSampleOffset = 0;
	igFrameGraphHighest = 0.0f;
	igFrameGraphDoAnimate = true;
}

// --------------------------------------------------------
// Adds a vertex shader to the list of vertex shaders
// --------------------------------------------------------
void Game::AddVertexShader(const wchar_t* _path)
{
	vertexShaders.push_back(std::make_shared<SimpleVertexShader>(
		Graphics::Device,
		Graphics::Context,
		FixPath(_path).c_str()
	));
}

// --------------------------------------------------------
// Adds a pixel shader to the list of pixel shaders
// --------------------------------------------------------
void Game::AddPixelShader(const wchar_t* _path)
{
	pixelShaders.push_back(std::make_shared<SimplePixelShader>(
		Graphics::Device,
		Graphics::Context,
		FixPath(_path).c_str()
	));
}

// --------------------------------------------------------
// Adds a texture to the list of textures
// --------------------------------------------------------
void Game::AddTexture(const wchar_t* _path)
{
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;

	CreateWICTextureFromFile(
		Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(_path).c_str(),
		nullptr,
		srv.GetAddressOf()
	);

	textures.push_back(srv);
}

// --------------------------------------------------------
// Adds a Material to the list of Materials
// --------------------------------------------------------
void Game::AddMaterial(const char* _name, unsigned int _vertexShaderIndex, unsigned int _pixelShaderIndex, DirectX::XMFLOAT4 _colorTint, float _roughness)
{
	materials.push_back(std::make_shared<Material>(
		_name,
		vertexShaders[_vertexShaderIndex],
		pixelShaders[_pixelShaderIndex],
		_colorTint,
		_roughness
	));
}

void Game::AddMaterial(const char* _name, unsigned int _vertexShaderIndex, unsigned int _pixelShaderIndex, DirectX::XMFLOAT4 _colorTint)
{
	AddMaterial(_name, _vertexShaderIndex, _pixelShaderIndex, _colorTint, 0.0f);
}

void Game::AddMaterial(const char* _name, unsigned int _vertexShaderIndex, unsigned int _pixelShaderIndex, float _roughness)
{
	AddMaterial(_name, _vertexShaderIndex, _pixelShaderIndex, XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), _roughness);
}

void Game::AddMaterial(const char* _name, unsigned int _vertexShaderIndex, unsigned int _pixelShaderIndex)
{
	AddMaterial(_name, _vertexShaderIndex, _pixelShaderIndex, XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), 0.0f);
}

// --------------------------------------------------------
// Adds an Entity to the list of Entities
// --------------------------------------------------------
void Game::AddEntity(const char* _name, unsigned int _meshIndex, unsigned int _materialIndex, DirectX::XMFLOAT3 _position)
{
	shared_ptr<Entity> entity = std::make_shared<Entity>(
		_name,
		meshes[_meshIndex],
		materials[_materialIndex]
	);

	entity->GetTransform()->SetPosition(_position);
	entities.push_back(entity);
}

// --------------------------------------------------------
// Adds a Light of a given type to the list of Lights
// --------------------------------------------------------
void Game::AddLightDirectional(DirectX::XMFLOAT3 _direction, DirectX::XMFLOAT3 _color, float _intensity, bool _isActive)
{
	Light light = {};
	light.Type = LIGHT_TYPE_DIRECTIONAL;
	light.Direction = _direction;
	light.Color = _color;
	light.Intensity = _intensity;
	light.Active = _isActive ? 1 : 0;
	lights.push_back(light);
}

void Game::AddLightPoint(DirectX::XMFLOAT3 _position, DirectX::XMFLOAT3 _color, float _intensity, float _range, bool _isActive)
{
	Light light = {};
	light.Type = LIGHT_TYPE_POINT;
	light.Position = _position;
	light.Color = _color;
	light.Intensity = _intensity;
	light.Range = _range;
	light.Active = _isActive ? 1 : 0;
	lights.push_back(light);
}

void Game::AddLightSpot(DirectX::XMFLOAT3 _position, DirectX::XMFLOAT3 _direction, DirectX::XMFLOAT3 _color, float _intensity, float _range, float _innerAngle, float _outerAngle, bool _isActive)
{
	Light light = {};
	light.Type = LIGHT_TYPE_SPOT;
	light.Position = _position;
	light.Direction = _direction;
	light.Color = _color;
	light.Intensity = _intensity;
	light.Range = _range;
	light.SpotInnerAngle = _innerAngle;
	light.SpotOuterAngle = _outerAngle;
	light.Active = _isActive ? 1 : 0;
	lights.push_back(light);
}

// --------------------------------------------------------
// Adds a Camera to the list of Cameras
// --------------------------------------------------------
void Game::AddCamera(const char* _name, DirectX::XMFLOAT3 _position, DirectX::XMFLOAT3 _rotation, float _aspect)
{
	shared_ptr<Camera> camera = std::make_shared<Camera>(
		_name,
		make_shared<Transform>(),
		_aspect
	);

	camera->GetTransform()->SetPosition(_position);
	camera->GetTransform()->SetRotation(_rotation);

	cameras.push_back(camera);
}

void Game::AddCamera(const char* _name, DirectX::XMFLOAT3 _position, DirectX::XMFLOAT3 _rotation, float _aspect, float _fov)
{
	shared_ptr<Camera> camera = std::make_shared<Camera>(
		_name,
		make_shared<Transform>(),
		_aspect,
		_fov
	);

	camera->GetTransform()->SetPosition(_position);
	camera->GetTransform()->SetRotation(_rotation);

	cameras.push_back(camera);
}

void Game::AddCamera(const char* _name, DirectX::XMFLOAT3 _position, DirectX::XMFLOAT3 _rotation, float _aspect, bool _isOrthographic)
{
	shared_ptr<Camera> camera = std::make_shared<Camera>(
		_name,
		make_shared<Transform>(),
		_aspect,
		_isOrthographic
	);

	camera->GetTransform()->SetPosition(_position);
	camera->GetTransform()->SetRotation(_rotation);

	cameras.push_back(camera);
}

void Game::AddCamera(const char* _name, DirectX::XMFLOAT3 _position, DirectX::XMFLOAT3 _rotation, float _aspect, bool _isOrthographic, float _orthoWidth)
{
	shared_ptr<Camera> camera = std::make_shared<Camera>(
		_name,
		make_shared<Transform>(),
		_aspect,
		_isOrthographic,
		_orthoWidth
	);

	camera->GetTransform()->SetPosition(_position);
	camera->GetTransform()->SetRotation(_rotation);

	cameras.push_back(camera);
}

// --------------------------------------------------------
// Sets the sampler state in the samplerState variable
// based on the given graphics settings
// --------------------------------------------------------
void Game::SetGlobalSamplerState(D3D11_FILTER _filter, int _anisotropyLevel) {
	// Reset sampler state if one exists
	samplerState = nullptr;

	// Sampler state description for changing sampler state with UI
	D3D11_SAMPLER_DESC samplerDescription = {};
	// Set default sampler state values
	samplerDescription.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDescription.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDescription.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDescription.Filter = _filter;
	samplerDescription.MaxAnisotropy = _anisotropyLevel;
	samplerDescription.MaxLOD = D3D11_FLOAT32_MAX;

	// Create sampler state
	Graphics::Device->CreateSamplerState(&samplerDescription, &samplerState);
}

// --------------------------------------------------------
// Sets the sampler states of each Material
// --------------------------------------------------------
void Game::SetMaterialSamplerStates() {
	for (int i = 0; i < materials.size(); i++) {
		materials[i]->AddSampler("BasicSampler", samplerState);
	}
}

// --------------------------------------------------------
// Prepares the ImGui UI window for being created
// --------------------------------------------------------
void Game::ImGuiUpdate(float deltaTime) {
	// Put this all in a helper method that is called from Game::Update()
	// Feed fresh data to ImGui
	ImGuiIO& io = ImGui::GetIO();
	io.DeltaTime = deltaTime;
	io.DisplaySize.x = (float)Window::Width();
	io.DisplaySize.y = (float)Window::Height();
	// Reset the frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	// Determine new input capture
	Input::SetKeyboardCapture(io.WantCaptureKeyboard);
	Input::SetMouseCapture(io.WantCaptureMouse);
	// Show the demo window if it's activated
	if (igShowDemo) {
		ImGui::ShowDemoWindow();
	}
}

// --------------------------------------------------------
// Builds the ImGui UI window structure each frame
// --------------------------------------------------------
void Game::ImGuiBuild() {
	ImGui::Begin("Inspector");
	if (ImGui::CollapsingHeader("App Details")) {				// Statistics about the app window and performance; no input elements
		if (ImGui::TreeNode("Window")) {							// Meta stats about the window, mouse, and other stuff outside the simulation
			ImGui::Spacing();
			ImVec2 mousePos = ImGui::GetIO().MousePos;
			
			ImGui::Text("Resolution:   %6dx %6d", Window::Width(), Window::Height());
			ImGui::SetItemTooltip("Window resolution in pixels");

			ImGui::Text("Mouse (px):  (%6d, %6d)", (int)mousePos.x, (int)mousePos.y);
			ImGui::SetItemTooltip("Mouse position in pixels,\nstarting at top-left corner");
			
			ImGui::Text("Mouse (NDC): (%+6.3f, %+6.3f)",
				2.0f * (mousePos.x - (Window::Width() * 0.5f)) / Window::Width(),
				-2.0f * (mousePos.y - (Window::Height() * 0.5f)) / Window::Height()
			);
			ImGui::SetItemTooltip("Mouse position in Normalized Device Coordinates\n(-1 to 1), starting at top-left corner");

			ImGui::Text("Aspect Ratio: %6.3f", ((Window::Width() + 0.0f) / Window::Height()));
			ImGui::SetItemTooltip("Window aspect ratio (width/height)");
			
			ImGui::TreePop();
			ImGui::Spacing();
		}
		if (ImGui::TreeNode("Performance")) {					// Stats about the app's performance
			ImGui::Spacing();
			
			ImGui::Text("Framerate:    %6dfps", (int)ImGui::GetIO().Framerate);

			ImGui::Text("Delta Time:   %6dus", (int)(ImGui::GetIO().DeltaTime * 1000000));
			ImGui::SetItemTooltip("Time between frames in microseconds\n(I didn't want to break things by trying to print the mu)");

			if (ImGui::TreeNode("Framerate Graph")) {				// Graph of framerate over time
				// Sets tooptip of enclosing TreeNode
				ImGui::SetItemTooltip("Records the framerate over time\n(Slows down performance in Debug build while open)");
				
				ImGui::Spacing();
			
				// Plotting code taken from ImGui demo

				// If not animating or just initialized
				if (!igFrameGraphDoAnimate || igFrameGraphNextSampleTime == 0.0) {
					// Reset refresh time
					igFrameGraphNextSampleTime = ImGui::GetTime();
				}
				// Record however many samples should have been captured within the elapsed time this frame
				while (igFrameGraphNextSampleTime < ImGui::GetTime()) {
					// Get the framerate for this frame
					float framerate = ImGui::GetIO().Framerate;
					// Record framerate in array at the current place
					igFrameGraphSamples[igFrameGraphSampleOffset] = framerate;
					// Step one sample further in the array, mod the current number of samples we're supposed to capture
					igFrameGraphSampleOffset = (igFrameGraphSampleOffset + 1) % (igFrameGraphSampleCount);
					// Set next sample time based on sample rate
					igFrameGraphNextSampleTime += 1.0f / igFrameGraphSampleRate;
					// If this framerate is the highest recorded, rescale graph so it's in view
					if (framerate > igFrameGraphHighest) {
						igFrameGraphHighest = framerate;
					}
				}
				// Draw the graph
				ImGui::PlotLines("Framerate", igFrameGraphSamples, igFrameGraphSampleCount, igFrameGraphSampleOffset, "", 0.0f, igFrameGraphHighest, ImVec2(0, 100.0f));
			
				// Pauses or resumes the graph
				if (ImGui::Button(igFrameGraphDoAnimate ? "Pause Framerate Graph" : "Resume Framerate Graph")) {
					igFrameGraphDoAnimate = !igFrameGraphDoAnimate;
				}

				ImGui::SliderFloat("Graph Rate", &igFrameGraphSampleRate, 0.5f, 120.0f, "%3.1fHz", ImGuiSliderFlags_Logarithmic);
				ImGui::SetItemTooltip("How often the graph updates per second\n(Rate will update after next sample)");
			
				ImGui::SliderInt("Graph Scale", &igFrameGraphSampleCount, 1, 1000, "%d samples", ImGuiSliderFlags_Logarithmic);
				ImGui::SetItemTooltip("How many samples are shown on the graph\n(WARNING: Changing will mess up the current graph!)");

				ImGui::TreePop();
				ImGui::Spacing();
			}
			else {
				// Sets tooptip of enclosing TreeNode if it's closed
				ImGui::SetItemTooltip("Slows down performance in Debug mode");
				// Resets sample time
				igFrameGraphNextSampleTime = 0.0;
			}

			ImGui::TreePop();
			ImGui::Spacing();
		}
	}
	
	if (ImGui::CollapsingHeader("Settings")) {					// General settings parameters for graphics and simulation
		ImGui::Spacing();
		
		ImGui::ColorEdit3("Background Color", pBackgroundColor);
		ImGui::Spacing();

		ImGui::SliderFloat("Object Rotation", &pObjectRotationSpeed, -2.0f, 2.0f, "%.1f");
		ImGui::Spacing();
		
		// Pass in the preview value visible before opening the combo
		const char* currentFilterName = SAMPLER_FILTER_STRINGS[pSelectedSamplerFilter];

		if (ImGui::BeginCombo("Filter", currentFilterName))
		{
			for (int i = 0; i < IM_ARRAYSIZE(SAMPLER_FILTER_STRINGS); i++)
			{
				const bool isSelected = (pSelectedSamplerFilter == i);
				// If option clicked,
				if (ImGui::Selectable(SAMPLER_FILTER_STRINGS[i], isSelected)) {
					// Set its index as the currently selected array item
					pSelectedSamplerFilter = i;
					// Set the filter itself
					pSamplerFilter = SAMPLER_FILTERS[i];
					// Set the sampler state of all materials
					SetGlobalSamplerState(pSamplerFilter, (int)pow(2, pAnisotropyPower));
					SetMaterialSamplerStates();
				}

				// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		// If anisotropic filtering is selecter, reveal anisotropy level slider
		if (pSamplerFilter == D3D11_FILTER_ANISOTROPIC) {
			if (ImGui::SliderInt("Anisotropy", &pAnisotropyPower, 0, 4)) {
				SetGlobalSamplerState(pSamplerFilter, (int)pow(2, pAnisotropyPower));
				SetMaterialSamplerStates();
			}
		}
		ImGui::SetItemTooltip("Sets anisotropy level to 2^n");
		
		ImGui::Spacing();
	}

	if (ImGui::CollapsingHeader("Meshes")) {					// Info about each mesh
		ImGui::Spacing();

		ImGui::PushID("MESH");
		for (int i = 0; i < meshes.size(); i++) {

			// Each mesh gets its own Tree Node
			ImGui::PushID(i);
			if (ImGui::TreeNode("", "(%06d) %s", i, meshes[i]->GetName())) {
				ImGui::Spacing();

				ImGui::Text("Triangles: %6d", meshes[i]->GetIndexCount() / 3);
				ImGui::Text("Vertices:  %6d", meshes[i]->GetVertexCount());
				ImGui::Text("Indices:   %6d", meshes[i]->GetIndexCount());

				ImGui::TreePop();
				ImGui::Spacing();
			}
			ImGui::PopID();
		}
		ImGui::PopID();

		ImGui::Spacing();
	}

	if (ImGui::CollapsingHeader("Materials")) {					// Info about each material
		ImGui::Spacing();

		ImGui::PushID("MATERIAL");
		for (int i = 0; i < materials.size(); i++) {

			// Each material gets its own Tree Node
			ImGui::PushID(i);
			if (ImGui::TreeNode("", "(%06d) %s", i, materials[i]->GetName())) {
				ImGui::Spacing();

				// Get material's tint as a float array
				XMFLOAT4 tint_xm = materials[i]->GetColorTint();
				float tint_f[4] = {tint_xm.x, tint_xm.y, tint_xm.z, tint_xm.w};
				float roughness = materials[i]->GetRoughness();
				vector<ID3D11ShaderResourceView*> textures = materials[i]->GetTextures();
				XMFLOAT2 uv_pos = materials[i]->GetUVPosition();
				XMFLOAT2 uv_sca = materials[i]->GetUVScale();

				// If the user has edited the tint this frame, change the material's tint
				if (ImGui::ColorEdit4("Tint", tint_f)) {
					materials[i]->SetColorTint(XMFLOAT4(tint_f));
				}
				// If the user has edited the material's roughness this frame, change the material's roughness
				if (ImGui::SliderFloat("Roughness", &roughness, 0.0f, 1.0f, "%.2f")) {
					materials[i]->SetRoughness(roughness);
				}

				// If any textures exist, include texture settings
				if (textures.size() > 0) {
					if (ImGui::DragFloat2("UV Position", (float*)&uv_pos, 0.01f, NULL, NULL, "%.2f")) {
						materials[i]->SetUVPosition(uv_pos);
					}
					if (ImGui::DragFloat2("UV Scale", (float*)&uv_sca, 0.01f, NULL, NULL, "%.2f")) {
						materials[i]->SetUVScale(uv_sca);
					}

					ImGui::Text("Textures:");
					for (ID3D11ShaderResourceView* texture : textures) {
						ImGui::Image(
							(void*)texture,
							ImVec2(240, 240),
							ImVec2(uv_pos.x, uv_pos.y),
							ImVec2(uv_pos.x + uv_sca.x, uv_pos.y + uv_sca.y)
						);
					}
				}

				// Custom settings for specific materials
				if (materials[i]->GetName() == "Mat_Custom") {
					float image[2] = { pMatCustomImage.x, pMatCustomImage.y };
					float zoom[2] = { pMatCustomZoom.x, pMatCustomZoom.y };

					ImGui::DragInt("Iterations", &pMatCustomIterations, 1, 0, 100);
					if (ImGui::DragFloat2("Image Center", image, 0.01f, -4.0f, 4.0f)) {
						pMatCustomImage = XMFLOAT2(image);
					}
					if (ImGui::DragFloat2("Zoom Center", zoom, 0.01f, -4.0f, 4.0f)) {
						pMatCustomZoom = XMFLOAT2(zoom);
					}
				}

				ImGui::TreePop();
				ImGui::Spacing();
			}
			ImGui::PopID();
		}
		ImGui::PopID();

		ImGui::Spacing();
	}

	if (ImGui::CollapsingHeader("Entities")) {					// Info about each entity
		ImGui::Spacing();

		// Store the position, rotation, scale, and tint of each entity as they're read in
		

		ImGui::PushID("ENTITY");
		for (int i = 0; i < entities.size(); i++) {
			// Get position, rotation, scale, and tint
			XMFLOAT3 entityPos = entities[i]->GetTransform()->GetPosition();
			XMFLOAT3 entityRot = entities[i]->GetTransform()->GetRotation();
			XMFLOAT3 entitySca = entities[i]->GetTransform()->GetScale();

			// Each entity gets its own Tree Node
			ImGui::PushID(i);
			if (ImGui::TreeNode("", "(%06d) %s", i, entities[i]->GetName())) {
				ImGui::Spacing();

				ImGui::Text("Mesh:      %s", (entities[i]->GetMesh()->GetName()));
				ImGui::Text("Material:  %s", (entities[i]->GetMaterial()->GetName()));
				ImGui::Spacing();

				if (ImGui::DragFloat3("Position", &entityPos.x, 0.01f)) {
					entities[i]->GetTransform()->SetPosition(entityPos);
				}
				if (ImGui::DragFloat3("Rotation", &entityRot.x, 0.01f)) {
					entities[i]->GetTransform()->SetRotation(entityRot);
				}
				ImGui::SetItemTooltip("In radians");
				if (ImGui::DragFloat3("Scale", &entitySca.x, 0.01f, 0.0f)) {
					entities[i]->GetTransform()->SetScale(entitySca);
				}
				// Clamp scale to 0
				if (entitySca.x < 0.0f) entitySca.x = 0.0f;
				if (entitySca.y < 0.0f) entitySca.y = 0.0f;
				if (entitySca.z < 0.0f) entitySca.z = 0.0f;

				ImGui::TreePop();
				ImGui::Spacing();
			}
			ImGui::PopID();
		}
		ImGui::PopID();

		ImGui::Spacing();
	}

	if (ImGui::CollapsingHeader("Lights")) {					// Info about each light
		ImGui::Spacing();

		float ambientColor[3] = { pAmbientColor.x, pAmbientColor.y, pAmbientColor.z };
		if (ImGui::ColorEdit3("Ambient Light", ambientColor)) {
			pAmbientColor = XMFLOAT3(ambientColor);
		}
		ImGui::Spacing();

		ImGui::PushID("LIGHT");
		for (int i = 0; i < lights.size(); i++) {					// List of lights in the scene

			// Each light gets its own Tree Node
			ImGui::PushID(i);

			bool active = lights[i].Active == 1;
			ImGui::AlignTextToFramePadding();
			if (ImGui::Checkbox("", &active)) {
				lights[i].Active = active;
			}
			ImGui::SetItemTooltip("Toggle whether light is active");

			ImGui::SameLine();
			if (ImGui::TreeNode("", "(%06d) %s", i, LIGHT_TYPE_STRINGS[lights[i].Type])) {
				ImGui::Spacing();

				ImGui::ColorEdit3("Color", &lights[i].Color.x);
				if (ImGui::DragFloat("Intensity", &lights[i].Intensity, 0.1f, 0.0f, NULL, "%.1f")) {
					lights[i].Intensity = max(lights[i].Intensity, 0.0f);
				}
				
				ImGui::Spacing();
				ImGui::Text("Type:");
				ImGui::RadioButton("Directional", &lights[i].Type, LIGHT_TYPE_DIRECTIONAL);
				ImGui::SameLine();
				ImGui::RadioButton("Point", &lights[i].Type, LIGHT_TYPE_POINT);
				ImGui::SameLine();
				ImGui::RadioButton("Spot", &lights[i].Type, LIGHT_TYPE_SPOT);

				ImGui::Spacing();
				if (lights[i].Type != LIGHT_TYPE_DIRECTIONAL) {
					ImGui::DragFloat3("Position", &lights[i].Position.x, 0.01f);
				}
				if (lights[i].Type != LIGHT_TYPE_POINT) {
					ImGui::DragFloat3("Direction", &lights[i].Direction.x, 0.01f);
				}
				if (lights[i].Type != LIGHT_TYPE_DIRECTIONAL) {
					if (ImGui::DragFloat("Range", &lights[i].Range, 0.1f, 0.0f, NULL, "%.1f")) {
						lights[i].Range = max(lights[i].Range, 0.0f);
					}
				}
				if (lights[i].Type == LIGHT_TYPE_SPOT) {
					if (ImGui::DragFloat("Spot Inner Angle", &lights[i].SpotInnerAngle, 0.01f, 0.0f, XM_PIDIV2, "%.2f")) {
						if (lights[i].SpotOuterAngle <= lights[i].SpotInnerAngle) {
							lights[i].SpotOuterAngle = lights[i].SpotInnerAngle + 0.01f;
						}
					}
					ImGui::SetItemTooltip("In radians");
					if (ImGui::DragFloat("Spot Outer Angle", &lights[i].SpotOuterAngle, 0.01f, 0.01f, XM_PIDIV2, "%.2f")) {
						if (lights[i].SpotOuterAngle <= lights[i].SpotInnerAngle) {
							lights[i].SpotInnerAngle = lights[i].SpotOuterAngle - 0.01f;
						}
					}
					ImGui::SetItemTooltip("In radians");
				}

				ImGui::TreePop();
				ImGui::Spacing();
			}
			ImGui::PopID();
		}
		ImGui::PopID();
	}

	if (ImGui::CollapsingHeader("Cameras")) {					// Info about each camera
		ImGui::Spacing();

		ImGui::PushID("CAMERA");
		for (int i = 0; i < cameras.size(); i++) {
			// Get position, rotation, scale, and tint
			XMFLOAT3 cameraPos = cameras[i]->GetTransform()->GetPosition();
			XMFLOAT3 cameraRot = cameras[i]->GetTransform()->GetRotation();
			XMFLOAT3 cameraRight = cameras[i]->GetTransform()->GetRight();
			XMFLOAT3 cameraUp = cameras[i]->GetTransform()->GetUp();
			XMFLOAT3 cameraFwd = cameras[i]->GetTransform()->GetForward();
			bool cameraMode = cameras[i]->GetProjectionMode();
			float cameraMove = cameras[i]->GetMoveSpeed();
			float cameraLook = cameras[i]->GetLookSpeed();
			float cameraNear = cameras[i]->GetNearClip();
			float cameraFar = cameras[i]->GetFarClip();

			// Each entity gets its own Tree Node
			ImGui::PushID(i);
			ImGui::AlignTextToFramePadding();
			ImGui::RadioButton("", &pCameraCurrent, i);
			ImGui::SetItemTooltip("Set as active camera");

			ImGui::SameLine();
			if (ImGui::TreeNode("", "(%06d) %s", i, cameras[i]->GetName())) {
				ImGui::Spacing();

				if (ImGui::Button(cameraMode ? "Mode: Orthographic" : "Mode: Perspective")) {
					cameras[i]->ToggleProjectionMode();
				}
				if (cameraMode) {
					float cameraWidth = cameras[i]->GetOrthographicWidth();
					if (ImGui::DragFloat("Width", &cameraWidth, 1.0f, 1.0f, 1000.0f, "%.0f")) {
						cameras[i]->SetOrthographicWidth(cameraWidth);
					}
					ImGui::SetItemTooltip("In world units");
				}
				else {
					float cameraFov = (cameras[i]->GetFov() * 180 * XM_1DIVPI);
					if (ImGui::DragFloat("Field of View", &cameraFov, 1.0f, 1.0f, 179.0f, "%.0f")) {
						cameras[i]->SetFov(cameraFov * XM_PI / 180);
					}
					ImGui::SetItemTooltip("In degrees (stored as radians)");
				}
				ImGui::Spacing();
				
				if (ImGui::DragFloat3("Position", &cameraPos.x, 0.01f)) {
					cameras[i]->GetTransform()->SetPosition(cameraPos);
				}
				if (ImGui::DragFloat3("Rotation", &cameraRot.x, 0.01f)) {
					cameras[i]->GetTransform()->SetRotation(cameraRot);
				}
				ImGui::SetItemTooltip("In radians");
				ImGui::Text("Right:       (%+6.3f, %+6.3f, %+6.3f)", cameraRight.x, cameraRight.y, cameraRight.z);
				ImGui::Text("Up:          (%+6.3f, %+6.3f, %+6.3f)", cameraUp.x, cameraUp.y, cameraUp.z);
				ImGui::Text("Forward:     (%+6.3f, %+6.3f, %+6.3f)", cameraFwd.x, cameraFwd.y, cameraFwd.z);
				ImGui::Spacing();

				if (ImGui::DragFloat("Move Speed", &cameraMove, 0.1f, 0.1f, 100.0f, "%.1f", ImGuiSliderFlags_Logarithmic)) {
					cameras[i]->SetMoveSpeed(cameraMove);
				}
				ImGui::SetItemTooltip("In units per second");
				if (ImGui::DragFloat("Look Speed", &cameraLook, 0.01f, 0.01f, 10.0f, "%.2f", ImGuiSliderFlags_Logarithmic)) {
					cameras[i]->SetLookSpeed(cameraLook);
				}
				ImGui::SetItemTooltip("In milliradians per pixel\nof mouse movement");
				ImGui::Spacing();

				if (ImGui::DragFloat("Near Clip", &cameraNear, 0.01f, 0.001f, 10.0f, "%.3f", ImGuiSliderFlags_Logarithmic)) {
					if (cameraFar > cameraNear) {
						cameras[i]->SetNearClip(cameraNear);
					}
					else {
						cameras[i]->SetNearClip(cameraFar - 0.001f);
					}
				}
				if (ImGui::DragFloat("Far Clip", &cameraFar, 1.0f, 11.0f, 10000.0f, "%.0f", ImGuiSliderFlags_Logarithmic)) {
					if (cameraFar > cameraNear) {
						cameras[i]->SetFarClip(cameraFar);
					}
					else {
						cameras[i]->SetFarClip(floor(cameraNear) + 1.0f);
					}
				}

				ImGui::TreePop();
				ImGui::Spacing();
			}
			ImGui::PopID();

		}
		ImGui::PopID();

		ImGui::Spacing();
	}

	if (ImGui::CollapsingHeader("Testing (Unused)")) {			// Miscellaneous UI inputs/elements for testing and debugging
		ImGui::Spacing();

		ImGui::Text("N/A");

		ImGui::Spacing();
	}

	if (ImGui::CollapsingHeader("Dear ImGui")) {				// Settings related to ImGui itself
		ImGui::Spacing();

		if (ImGui::Button("Toggle Dear ImGui Demo")) {				// Toggles the ImGui Demo window
			igShowDemo = !igShowDemo;
		}

		ImGui::Spacing();
	}

	ImGui::End();
}

// --------------------------------------------------------
// Called by destructor, cleans up pointers used by helper functions
// --------------------------------------------------------
void Game::CleanupSimulationParameters() {
	if (igFrameGraphSamples != nullptr) {
		delete[] igFrameGraphSamples;
		igFrameGraphSamples = nullptr;
	}
}
