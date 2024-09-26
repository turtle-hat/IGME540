#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <memory>
#include <vector>

#include "Mesh.h"
#include "Entity.h"
#include "BufferStructs.h"

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
	void InitializeSimulationParameters();
	void CreateConstantBuffers();
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
	// Stores translation offset for World transformation to be sent through constant buffer
	float pCBTfWorldTl[3];
	// Stores color tint to be sent through constant buffer
	float pCBColorTint[4];



	// MESHES
	std::vector<std::shared_ptr<Mesh>> meshes;
	
	
	
	// ENTITIES
	std::vector<std::shared_ptr<Entity>> entities;



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
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;

	// Primary constant buffer
	Microsoft::WRL::ComPtr<ID3D11Buffer> constBuffer;
};

