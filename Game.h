#pragma once

#include <d3d11.h>
#include <wrl/client.h>

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
	void ImGuiUpdate(float deltaTime);

	// Update helper methods
	void ImGuiBuild();

	// Draw helper methods

	// Destructor helper methods
	void CleanupSimulationParameters();

	// Simulation Parameter Variables
	// All start with a "p" for "parameter"
	// The background color used by Direct3D
	float pBackgroundColor[4];
	// An arbitrary slider
	float pTripleSlider[3];

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

	// Buffers to hold actual geometry data
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;

	// Shaders and shader-related constructs
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
};

