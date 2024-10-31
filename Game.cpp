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
	// VERTEX SHADER 0
	vertexShaders.push_back(std::make_shared<SimpleVertexShader>(
		Graphics::Device,
		Graphics::Context,
		FixPath(L"VertexShader.cso").c_str()
	));
	// PIXEL SHADERS 0-3
	pixelShaders.push_back(std::make_shared<SimplePixelShader>(
		Graphics::Device,
		Graphics::Context,
		FixPath(L"PixelShader.cso").c_str()
	));
	pixelShaders.push_back(std::make_shared<SimplePixelShader>(
		Graphics::Device,
		Graphics::Context,
		FixPath(L"normalPS.cso").c_str()
	));
	pixelShaders.push_back(std::make_shared<SimplePixelShader>(
		Graphics::Device,
		Graphics::Context,
		FixPath(L"uvPS.cso").c_str()
	));
	pixelShaders.push_back(std::make_shared<SimplePixelShader>(
		Graphics::Device,
		Graphics::Context,
		FixPath(L"customPS.cso").c_str()
	));
}

// --------------------------------------------------------
// Loads textures and creates materials
// --------------------------------------------------------
void Game::CreateMaterials()
{
	// Declare SRVs 0-2
	// I didn't wanna put these in a vector since I
	// didn't know how to push_back a new ComPtr for each SRV
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv0;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv1;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv2;

	// Load textures
	CreateWICTextureFromFile(
		Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/T_brokentiles_DS.png").c_str(),
		nullptr,
		srv0.GetAddressOf()
	);
	// For some reason creating more than one texture spits out a ton of warnings when the app closes:
	// D3D11 WARNING: Process is terminating. Using simple reporting. Please call ReportLiveObjects() at runtime for standard reporting. [ STATE_CREATION WARNING #0: UNKNOWN]
	// D3D11 WARNING : Live Producer at 0x0000029023D63430, Refcount : 2.[STATE_CREATION WARNING #0: UNKNOWN]
	// D3D11 WARNING : Live Object at 0x0000029023D67390, Refcount : 0.[STATE_CREATION WARNING #0: UNKNOWN]
	CreateWICTextureFromFile(
		Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/T_rustymetal_DS.png").c_str(),
		nullptr,
		srv1.GetAddressOf()
	);
	CreateWICTextureFromFile(
		Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/T_tiles_DS.png").c_str(),
		nullptr,
		srv2.GetAddressOf()
	);

	// Sampler state description for changing sampler state with UI
	D3D11_SAMPLER_DESC samplerDescription = {};
	// Set default sampler state values
	samplerDescription.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDescription.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDescription.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDescription.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDescription.MaxAnisotropy = 16;
	samplerDescription.MaxLOD = D3D11_FLOAT32_MAX;

	// Create sampler state
	Graphics::Device->CreateSamplerState(&samplerDescription, &samplerState);

	// Create materials
	// MATERIALS 0-2
	materials.push_back(std::make_shared<Material>(
		"Mat_BrokenTiles",
		vertexShaders[0],
		pixelShaders[0],
		XMFLOAT4(1.0f, 1.0f, 0.1f, 1.0f),
		0.2f
	));
	materials[0]->AddTextureSRV("SurfaceTexture", srv0);
	materials[0]->AddSampler("BasicSampler", samplerState);
	materials.push_back(std::make_shared<Material>(
		"Mat_RustyMetal",
		vertexShaders[0],
		pixelShaders[0],
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		0.8f
	));
	materials[1]->AddTextureSRV("SurfaceTexture", srv1);
	materials[1]->AddSampler("BasicSampler", samplerState);
	materials.push_back(std::make_shared<Material>(
		"Mat_Tiles",
		vertexShaders[0],
		pixelShaders[0],
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		0.0f
	));
	materials[2]->AddTextureSRV("SurfaceTexture", srv2);
	materials[2]->AddSampler("BasicSampler", samplerState);
	// MATERIALS 3-5
	materials.push_back(std::make_shared<Material>(
		"Mat_Normals",
		vertexShaders[0],
		pixelShaders[1],
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		1.0f
	));
	materials.push_back(std::make_shared<Material>(
		"Mat_UVs",
		vertexShaders[0],
		pixelShaders[2],
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		1.0f
	));
	materials.push_back(std::make_shared<Material>(
		"Mat_Custom",
		vertexShaders[0],
		pixelShaders[3],
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		1.0f
	));
}

// --------------------------------------------------------
// Creates the geometry we're going to draw and all
// entities in the scene
// --------------------------------------------------------
void Game::CreateGeometry()
{
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
	Vertex vStarterTriangle[] =
	{
		{ XMFLOAT3(+0.0f, +0.5f, +0.0f), XMFLOAT3(+0.0f, +0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(+0.5f, -0.5f, +0.0f), XMFLOAT3(+0.0f, +0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.5f, -0.5f, +0.0f), XMFLOAT3(+0.0f, +0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
	};

	// Set up indices, which tell us which vertices to use and in which order
	// - This is redundant for just 3 vertices, but will be more useful later
	// - Indices are technically not required if the vertices are in the buffer 
	//    in the correct order and each one will be used exactly once
	// - But just to see how it's done...
	unsigned int iStarterTriangle[] = { 0, 1, 2 };

	// MESH 0
	meshes.push_back(std::make_shared<Mesh>(
		"M_StarterTriangle",
		vStarterTriangle,
		iStarterTriangle,
		ARRAYSIZE(vStarterTriangle),
		ARRAYSIZE(iStarterTriangle)
	));



	// Gradient Rectangle
	Vertex vGradientRectangle[] = {
		{ XMFLOAT3(-0.2f, +0.05f, +0.0f), XMFLOAT3(+0.0f, +0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.2f, -0.05f, +0.0f), XMFLOAT3(+0.0f, +0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.1f, +0.05f, +0.0f), XMFLOAT3(+0.0f, +0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.1f, -0.05f, +0.0f), XMFLOAT3(+0.0f, +0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(+0.0f, +0.05f, +0.0f), XMFLOAT3(+0.0f, +0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(+0.0f, -0.05f, +0.0f), XMFLOAT3(+0.0f, +0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(+0.1f, +0.05f, +0.0f), XMFLOAT3(+0.0f, +0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(+0.1f, -0.05f, +0.0f), XMFLOAT3(+0.0f, +0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(+0.2f, +0.05f, +0.0f), XMFLOAT3(+0.0f, +0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(+0.2f, -0.05f, +0.0f), XMFLOAT3(+0.0f, +0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
	};

	unsigned int iGradientRectangle[] = {
		0, 2, 1,
		1, 2, 3,
		2, 4, 5,
		2, 5, 3,
		4, 6, 7,
		4, 7, 5,
		6, 8, 9,
		6, 9, 7
	};

	// MESH 1
	meshes.push_back(std::make_shared<Mesh>(
		"M_GradientRectangle",
		vGradientRectangle,
		iGradientRectangle,
		ARRAYSIZE(vGradientRectangle),
		ARRAYSIZE(iGradientRectangle)
	));
	

	// Mirror's Edge Logo
	// (I think it looks cool)
	Vertex vMirrorsEdge[] = {
		{ XMFLOAT3(-0.05f, +0.06f, +0.0f), XMFLOAT3(+0.0f, +0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(+0.01f, +0.06f, +0.0f), XMFLOAT3(+0.0f, +0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.03f, +0.04f, +0.0f), XMFLOAT3(+0.0f, +0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.01f, +0.02f, +0.0f), XMFLOAT3(+0.0f, +0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(+0.03f, +0.02f, +0.0f), XMFLOAT3(+0.0f, +0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(+0.05f, +0.02f, +0.0f), XMFLOAT3(+0.0f, +0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-0.04f, +0.00f, +0.0f), XMFLOAT3(+0.0f, +0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(+0.01f, -0.06f, +0.0f), XMFLOAT3(+0.0f, +0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
	};

	unsigned int iMirrorsEdge[] = {
		0, 1, 2,
		2, 1, 3,
		3, 1, 4,
		4, 1, 5,
		2, 3, 6,
		3, 4, 7
	};

	// MESH 2
	meshes.push_back(std::make_shared<Mesh>(
		"M_MELogo",
		vMirrorsEdge,
		iMirrorsEdge,
		ARRAYSIZE(vMirrorsEdge),
		ARRAYSIZE(iMirrorsEdge)
	));





	// Create entities for these meshes
	// ENTITIES 0-4
	entities.push_back(std::make_shared<Entity>("E_StarterTriangle", meshes[0], materials[0]));
	entities.push_back(std::make_shared<Entity>("E_GradientRectangle", meshes[1], materials[0]));
	entities.push_back(std::make_shared<Entity>("E_MELogo", meshes[2], materials[1]));
	entities.push_back(std::make_shared<Entity>("E_BigTriangle", meshes[0], materials[1]));
	entities.push_back(std::make_shared<Entity>("E_UpsideDownTriangle", meshes[0], materials[2]));

	// Set special parameters for each mesh
	entities[1]->GetTransform()->SetPosition(XMFLOAT3(-0.5f, -9.5f, 0.0f));
	
	entities[2]->GetTransform()->SetPosition(XMFLOAT3(0.5f, -9.5f, 0.0f));
	
	entities[3]->GetTransform()->SetPosition(XMFLOAT3(0.0f, -9.2f, 0.0f));
	entities[3]->GetTransform()->SetScale(XMFLOAT3(2.0f, 0.5f, 1.0f));
	
	entities[4]->GetTransform()->SetPosition(XMFLOAT3(0.7f, -10.5f, 0.0f));
	entities[4]->GetTransform()->SetRotation(XMFLOAT3(0.0f, 0.0f, XM_PI));
	entities[4]->GetTransform()->SetScale(XMFLOAT3(0.5f, 0.5f, 0.5f));





	// Create meshes from OBJ models
	// MESHES 3-9
	meshes.push_back(std::make_shared<Mesh>("M_Cube", FixPath(L"../../Assets/Models/cube.obj").c_str()));
	meshes.push_back(std::make_shared<Mesh>("M_Cylinder", FixPath(L"../../Assets/Models/cylinder.obj").c_str()));
	meshes.push_back(std::make_shared<Mesh>("M_Helix", FixPath(L"../../Assets/Models/helix.obj").c_str()));
	meshes.push_back(std::make_shared<Mesh>("M_Quad-SingleSided", FixPath(L"../../Assets/Models/quad.obj").c_str()));
	meshes.push_back(std::make_shared<Mesh>("M_Quad-DoubleSided", FixPath(L"../../Assets/Models/quad_double_sided.obj").c_str()));
	meshes.push_back(std::make_shared<Mesh>("M_Sphere", FixPath(L"../../Assets/Models/sphere.obj").c_str()));
	meshes.push_back(std::make_shared<Mesh>("M_Torus", FixPath(L"../../Assets/Models/torus.obj").c_str()));

	// ENTITIES 5-11
	entities.push_back(std::make_shared<Entity>("E_Normal_Cube",				meshes[3], materials[3]));
	entities.push_back(std::make_shared<Entity>("E_Normal_Cylinder",			meshes[4], materials[3]));
	entities.push_back(std::make_shared<Entity>("E_Normal_Helix",				meshes[5], materials[3]));
	entities.push_back(std::make_shared<Entity>("E_Normal_Sphere",				meshes[8], materials[3]));
	entities.push_back(std::make_shared<Entity>("E_Normal_Torus",				meshes[9], materials[3]));
	entities.push_back(std::make_shared<Entity>("E_Normal_Quad-SingleSided",	meshes[6], materials[3]));
	entities.push_back(std::make_shared<Entity>("E_Normal_Quad-DoubleSided",	meshes[7], materials[3]));

	entities[5]->GetTransform()->SetPosition(-9.0f, 3.0f, 0.0f);
	entities[6]->GetTransform()->SetPosition(-6.0f, 3.0f, 0.0f);
	entities[7]->GetTransform()->SetPosition(-3.0f, 3.0f, 0.0f);
	entities[8]->GetTransform()->SetPosition(0.0f,  3.0f, 0.0f);
	entities[9]->GetTransform()->SetPosition(3.0f,  3.0f, 0.0f);
	entities[10]->GetTransform()->SetPosition(6.0f, 3.0f, 0.0f);
	entities[11]->GetTransform()->SetPosition(9.0f, 2.0f, 0.0f);

	// ENTITIES 12-18
	entities.push_back(std::make_shared<Entity>("E_UV_Cube",				meshes[3], materials[4]));
	entities.push_back(std::make_shared<Entity>("E_UV_Cylinder",			meshes[4], materials[4]));
	entities.push_back(std::make_shared<Entity>("E_UV_Helix",				meshes[5], materials[4]));
	entities.push_back(std::make_shared<Entity>("E_UV_Sphere",				meshes[8], materials[4]));
	entities.push_back(std::make_shared<Entity>("E_UV_Torus",				meshes[9], materials[4]));
	entities.push_back(std::make_shared<Entity>("E_UV_Quad-SingleSided",	meshes[6], materials[4]));
	entities.push_back(std::make_shared<Entity>("E_UV_Quad-DoubleSided",	meshes[7], materials[4]));

	entities[12]->GetTransform()->SetPosition(-9.0f, 0.0f, 0.0f);
	entities[13]->GetTransform()->SetPosition(-6.0f, 0.0f, 0.0f);
	entities[14]->GetTransform()->SetPosition(-3.0f, 0.0f, 0.0f);
	entities[15]->GetTransform()->SetPosition(0.0f, 0.0f, 0.0f);
	entities[16]->GetTransform()->SetPosition(3.0f, 0.0f, 0.0f);
	entities[17]->GetTransform()->SetPosition(6.0f, -1.0f, 0.0f);
	entities[18]->GetTransform()->SetPosition(9.0f, -1.0f, 0.0f);

	// ENTITIES 19-25
	entities.push_back(std::make_shared<Entity>("E_Tint_Cube",				meshes[3], materials[0]));
	entities.push_back(std::make_shared<Entity>("E_Tint_Cylinder",			meshes[4], materials[1]));
	entities.push_back(std::make_shared<Entity>("E_Tint_Helix",				meshes[5], materials[2]));
	entities.push_back(std::make_shared<Entity>("E_Custom_Sphere",			meshes[8], materials[2]));
	entities.push_back(std::make_shared<Entity>("E_Tint_Torus",				meshes[9], materials[2]));
	entities.push_back(std::make_shared<Entity>("E_Tint_Quad-SingleSided",	meshes[6], materials[1]));
	entities.push_back(std::make_shared<Entity>("E_Tint_Quad-DoubleSided",	meshes[7], materials[0]));

	entities[19]->GetTransform()->SetPosition(-9.0f, -3.0f, 0.0f);
	entities[20]->GetTransform()->SetPosition(-6.0f, -3.0f, 0.0f);
	entities[21]->GetTransform()->SetPosition(-3.0f, -3.0f, 0.0f);
	entities[22]->GetTransform()->SetPosition(0.0f,  -3.0f, 0.0f);
	entities[23]->GetTransform()->SetPosition(3.0f,  -3.0f, 0.0f);
	entities[24]->GetTransform()->SetPosition(6.0f,  -4.0f, 0.0f);
	entities[25]->GetTransform()->SetPosition(9.0f,  -4.0f, 0.0f);

	/*
	// ENTITY 26
	entities.push_back(std::make_shared<Entity>("E_TestCanvas",	meshes[6], materials[5]));
	entities[26]->GetTransform()->SetPosition(0.0f, 0.0f, -3.0f);
	entities[26]->GetTransform()->SetRotation(-XM_PIDIV2, 0.0f, 0.0f);
	*/
}

// --------------------------------------------------------
// Creates the lights to be rendered in the scene
// --------------------------------------------------------
void Game::CreateLights() {
	// Create lights
	// LIGHT 0
	Light lightDir0 = {};
	lightDir0.Type = LIGHT_TYPE_DIRECTIONAL;
	lightDir0.Direction = XMFLOAT3(1.0f, 0.0f, 0.0f);
	lightDir0.Color = XMFLOAT3(1.0f, 0.0f, 0.0f);
	lightDir0.Intensity = 1.0f;
	lightDir0.Active = 1;
	lights.push_back(lightDir0);

	// LIGHTS 1-2
	Light lightDir1 = {};
	lightDir1.Type = LIGHT_TYPE_DIRECTIONAL;
	lightDir1.Direction = XMFLOAT3(0.0f, -1.0f, 0.0f);
	lightDir1.Color = XMFLOAT3(0.0f, 1.0f, 0.0f);
	lightDir1.Intensity = 1.0f;
	lightDir1.Active = 1;
	lights.push_back(lightDir1);

	Light lightDir2 = {};
	lightDir2.Type = LIGHT_TYPE_DIRECTIONAL;
	lightDir2.Direction = XMFLOAT3(0.0f, 0.0f, 1.0f);
	lightDir2.Color = XMFLOAT3(0.0f, 0.0f, 1.0f);
	lightDir2.Intensity = 1.0f;
	lightDir2.Active = 1;
	lights.push_back(lightDir2);

	// LIGHTS 3-4
	Light lightPt0 = {};
	lightPt0.Type = LIGHT_TYPE_POINT;
	lightPt0.Position = XMFLOAT3(-7.0f, -2.0f, 1.0f);
	lightPt0.Color = XMFLOAT3(0.0f, 1.0f, 1.0f);
	lightPt0.Intensity = 1.0f;
	lightPt0.Range = 10.0f;
	lightPt0.Active = 1;
	lights.push_back(lightPt0);

	Light lightPt1 = {};
	lightPt1.Type = LIGHT_TYPE_POINT;
	lightPt1.Position = XMFLOAT3(1.0f, -4.0f, 0.0f);
	lightPt1.Color = XMFLOAT3(1.0f, 0.8f, 1.0f);
	lightPt1.Intensity = 2.0f;
	lightPt1.Range = 4.0f;
	lightPt1.Active = 1;
	lights.push_back(lightPt1);

	// LIGHT 5
	Light lightSpt0 = {};
	lightSpt0.Type = LIGHT_TYPE_SPOT;
	lightSpt0.Position = XMFLOAT3(4.5f, 0.0f, 0.25f);
	lightSpt0.Direction = XMFLOAT3(0.0f, -1.0f, 0.0f);
	lightSpt0.Color = XMFLOAT3(1.0f, 1.0f, 1.0f);
	lightSpt0.Intensity = 1.0f;
	lightSpt0.Range = 30.0f;
	lightSpt0.SpotInnerAngle = 0.05f;
	lightSpt0.SpotOuterAngle = 0.5f;
	lightSpt0.Active = 1;
	lights.push_back(lightSpt0);
}

// --------------------------------------------------------
// Creates the cameras we'll need in the scene
// --------------------------------------------------------
void Game::CreateCameras() {
	// Create cameras
	float aspect = (Window::Width() + 0.0f) / Window::Height();
	// CAMERAS 0-3
	cameras.push_back(std::make_shared<Camera>(
		"C_Main",
		make_shared<Transform>(),
		aspect
	));
	cameras.push_back(std::make_shared<Camera>(
		"C_OrthoYZ",
		make_shared<Transform>(),
		aspect,
		true
	));
	cameras.push_back(std::make_shared<Camera>(
		"C_OrthoXZ",
		make_shared<Transform>(),
		aspect,
		true
	));
	cameras.push_back(std::make_shared<Camera>(
		"C_OrthoXY",
		make_shared<Transform>(),
		aspect,
		true
	));

	cameras[0]->GetTransform()->SetPosition(0.0f, 0.0f, -5.0f);

	cameras[1]->GetTransform()->SetPosition(100.0f, 0.0f, 0.0f);
	cameras[1]->GetTransform()->SetRotation(0.0f, -XM_PIDIV2, 0.0f);
	cameras[1]->SetLookSpeed(1.0f);

	cameras[2]->GetTransform()->SetPosition(0.0f, 100.0f, 0.0f);
	cameras[2]->GetTransform()->SetRotation(XM_PIDIV2 - 0.001f, 0.0f, 0.0f);
	cameras[2]->SetLookSpeed(1.0f);

	cameras[3]->GetTransform()->SetPosition(0.0f, 0.0f, -100.0f);
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

	// Update test meshes
	entities[0]->GetTransform()->SetPosition(sin(totalTime * 0.5f), cos(totalTime * 0.5f) - 10.0f, 0.0f);

	entities[1]->GetTransform()->SetPosition(fmod(totalTime * 0.5f, 3.0f) - 1.5f, -9.5f, 0.5f);
	entities[1]->GetTransform()->SetScale(sin(totalTime * XM_PI) + 1.5f, 1.0f, 1.0f);

	entities[2]->GetTransform()->Rotate(0.0f, 0.0f, 25.0f * deltaTime);
	entities[2]->GetTransform()->SetScale(1.0f, (sin(totalTime * 0.5f) + 1) * 2, 1.0f);
	entities[2]->GetTransform()->SetScale(1.0f, (sin(totalTime * 0.5f) + 1) * 2, 1.0f);

	entities[3]->GetTransform()->Rotate(deltaTime * -5.0f, deltaTime * -5.0f, 0.0f);

	entities[4]->GetTransform()->SetScale(sin(totalTime) * 0.5f + 1.1f, cos(totalTime) * 0.5f + 1.1f, 1.0f);

	// Rotate OBJ-imported meshes
	for (int i = 5; i <= 25; i++) {
		entities[i]->GetTransform()->Rotate(0.0f, deltaTime, 0.0f);
	}

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
		ps->SetFloat3("lightAmbient", pAmbientColor);

		// Set lights on pixel shader
		ps->SetData("lights", &lights[0], sizeof(Light) * (int)lights.size());
		// MATERIAL-SPECIFIC PIXEL SHADER CONSTANT BUFFER INPUTS
		if (material->GetName() == "Mat_Custom") {
			ps->SetFloat("totalTime", totalTime);
			ps->SetFloat2("imageCenter", pMatCustomImage);
			ps->SetFloat2("zoomCenter", pMatCustomZoom);
			ps->SetInt("maxIterations", pMatCustomIterations);
		}

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
	
	if (ImGui::CollapsingHeader("Basics")) {					// Basic parameters (e.g. colors and positions)
		ImGui::Spacing();
		
		ImGui::ColorEdit3("Background Color", pBackgroundColor);
		
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
					if (ImGui::DragFloat2("UV Position", (float*)&uv_pos, 0.01f, -1.0f, 1.0f, "%.2f")) {
						materials[i]->SetUVPosition(uv_pos);
					}
					if (ImGui::DragFloat2("UV Scale", (float*)&uv_sca, 0.01f, 0.001f, 10.0f, "%.2f")) {
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
