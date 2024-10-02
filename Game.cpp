#include "Game.h"
#include "Graphics.h"
#include "Vertex.h"
#include "Input.h"
#include "PathHelpers.h"
#include "Window.h"
#include "BufferStructs.h"

#include <DirectXMath.h>
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
	CreateGeometry();
	InitializeSimulationParameters();
	CreateConstantBuffers();

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
	XMFLOAT4 red			= XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 yellow			= XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);
	XMFLOAT4 green			= XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	XMFLOAT4 cyan			= XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 blue			= XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
	XMFLOAT4 red2			= XMFLOAT4(0.8f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 red3			= XMFLOAT4(0.6f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 red4			= XMFLOAT4(0.4f, 0.0f, 0.0f, 1.0f);

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
		{ XMFLOAT3(+0.0f, +0.5f, +0.0f), red },
		{ XMFLOAT3(+0.5f, -0.5f, +0.0f), blue },
		{ XMFLOAT3(-0.5f, -0.5f, +0.0f), green },
	};

	// Set up indices, which tell us which vertices to use and in which order
	// - This is redundant for just 3 vertices, but will be more useful later
	// - Indices are technically not required if the vertices are in the buffer 
	//    in the correct order and each one will be used exactly once
	// - But just to see how it's done...
	unsigned int iStarterTriangle[] = { 0, 1, 2 };

	meshes.push_back(std::make_shared<Mesh>(
		"M_StarterTriangle",
		vStarterTriangle,
		iStarterTriangle,
		ARRAYSIZE(vStarterTriangle),
		ARRAYSIZE(iStarterTriangle)
	));



	// Gradient Rectangle
	Vertex vGradientRectangle[] = {
		{ XMFLOAT3(-0.2f, +0.05f, +0.0f), red },
		{ XMFLOAT3(-0.2f, -0.05f, +0.0f), red },
		{ XMFLOAT3(-0.1f, +0.05f, +0.0f), yellow },
		{ XMFLOAT3(-0.1f, -0.05f, +0.0f), yellow },
		{ XMFLOAT3(+0.0f, +0.05f, +0.0f), green },
		{ XMFLOAT3(+0.0f, -0.05f, +0.0f), green },
		{ XMFLOAT3(+0.1f, +0.05f, +0.0f), cyan },
		{ XMFLOAT3(+0.1f, -0.05f, +0.0f), cyan },
		{ XMFLOAT3(+0.2f, +0.05f, +0.0f), blue },
		{ XMFLOAT3(+0.2f, -0.05f, +0.0f), blue },
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
		{ XMFLOAT3(-0.05f, +0.06f, +0.0f), red },
		{ XMFLOAT3(+0.01f, +0.06f, +0.0f), red },
		{ XMFLOAT3(-0.03f, +0.04f, +0.0f), red2 },
		{ XMFLOAT3(-0.01f, +0.02f, +0.0f), red3 },
		{ XMFLOAT3(+0.03f, +0.02f, +0.0f), red3 },
		{ XMFLOAT3(+0.05f, +0.02f, +0.0f), red3 },
		{ XMFLOAT3(-0.04f, +0.00f, +0.0f), red3 },
		{ XMFLOAT3(+0.01f, -0.06f, +0.0f), red4 },
	};

	unsigned int iMirrorsEdge[] = {
		0, 1, 2,
		2, 1, 3,
		3, 1, 4,
		4, 1, 5,
		2, 3, 6,
		3, 4, 7
	};

	meshes.push_back(std::make_shared<Mesh>(
		"M_MELogo",
		vMirrorsEdge,
		iMirrorsEdge,
		ARRAYSIZE(vMirrorsEdge),
		ARRAYSIZE(iMirrorsEdge)
	));



	// Create entities for these meshes
	entities.push_back(std::make_shared<Entity>("E_StarterTriangle", meshes[0]));
	entities.push_back(std::make_shared<Entity>("E_GradientRectangle", meshes[1]));
	entities.push_back(std::make_shared<Entity>("E_MELogo", meshes[2]));
	entities.push_back(std::make_shared<Entity>("E_BigTriangle", meshes[0]));
	entities.push_back(std::make_shared<Entity>("E_UpsideDownTriangle", meshes[0]));

	// Set special parameters for each mesh
	entities[1]->GetTransform()->SetPosition(XMFLOAT3(-0.5f, 0.5f, 0.0f));
	
	entities[2]->GetTransform()->SetPosition(XMFLOAT3(0.5f, 0.5f, 0.0f));
	
	entities[3]->GetTransform()->SetPosition(XMFLOAT3(0.0f, 0.8f, 0.0f));
	entities[3]->GetTransform()->SetScale(XMFLOAT3(2.0f, 0.5f, 1.0f));
	entities[3]->SetTint(XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f));
	
	entities[4]->GetTransform()->SetPosition(XMFLOAT3(0.7f, -0.5f, 0.0f));
	entities[4]->GetTransform()->SetRotation(XMFLOAT3(0.0f, 0.0f, XM_PI));
	entities[4]->GetTransform()->SetScale(XMFLOAT3(0.5f, 0.5f, 0.5f));
	entities[4]->SetTint(XMFLOAT4(0.5f, 0.0f, 0.0f, 1.0f));



	// Create cameras
	float aspect = (Window::Width() + 0.0f) / Window::Height();
	
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
	cameras[2]->GetTransform()->SetRotation(XM_PIDIV2, 0.0f, 0.0f);
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

	entities[0]->GetTransform()->SetPosition(sin(totalTime * 0.5f), cos(totalTime * 0.5f), 0.0f);

	entities[1]->GetTransform()->SetPosition(fmod(totalTime * 0.5f, 3.0f) - 1.5f, 0.5f, 0.5f);
	entities[1]->GetTransform()->SetScale(sin(totalTime * XM_PI) + 1.5f, 1.0f, 1.0f);

	entities[2]->GetTransform()->Rotate(0.0f, 0.0f, 25.0f * deltaTime);
	entities[2]->GetTransform()->SetScale(1.0f, (sin(totalTime * 0.5f) + 1) * 2, 1.0f);
	entities[2]->GetTransform()->SetScale(1.0f, (sin(totalTime * 0.5f) + 1) * 2, 1.0f);

	entities[3]->GetTransform()->Rotate(deltaTime * -5.0f, deltaTime * -5.0f, 0.0f);

	entities[4]->GetTransform()->SetScale(sin(totalTime) * 0.5f + 1.1f, cos(totalTime) * 0.5f + 1.1f, 1.0f);
	entities[4]->SetTint(XMFLOAT4(
		sin(totalTime) * 0.5f + 0.5f,
		sin(totalTime + (2.0f * XM_PI / 3.0f)) * 0.5f + 0.5f,
		sin(totalTime + (4.0f * XM_PI / 3.0f)) * 0.5f + 0.5f,
		1.0f
	));

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
		entities[i]->Draw(constBuffer, cameras[pCameraCurrent]);
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
	float color[4] = {0.4f, 0.6f, 0.75f, 1.0f};
	for (int i = 0; i < 4; i++) {
		pBackgroundColor[i] = color[i];
	}

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
// Creates all constant buffers used by our shaders
// --------------------------------------------------------
void Game::CreateConstantBuffers()
{
	// Find size of constant buffer struct
	unsigned int size = sizeof(VertexShaderData);
	// Get lowest multiple of 16 that is >= the size
	// of the buffer struct using integer division
	size = (size + 15) / 16 * 16;

	// Define constant buffer parameters
	D3D11_BUFFER_DESC bufferDesc	= {};
	bufferDesc.BindFlags			= D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.ByteWidth			= size;
	bufferDesc.CPUAccessFlags		= D3D11_CPU_ACCESS_WRITE;
	bufferDesc.Usage				= D3D11_USAGE_DYNAMIC;

	// Create the buffer itself
	Graphics::Device->CreateBuffer(&bufferDesc, 0, constBuffer.GetAddressOf());

	// Bind the buffer
	Graphics::Context->VSSetConstantBuffers(
		0,
		1,
		constBuffer.GetAddressOf()
	);
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
		if (ImGui::TreeNode("Performance")) {						// Stats about the app's performance
			ImGui::Spacing();
			
			ImGui::Text("Framerate:    %6dfps", (int)ImGui::GetIO().Framerate);

			ImGui::Text("Delta Time:   %6dus", (int)(ImGui::GetIO().DeltaTime * 1000000));
			ImGui::SetItemTooltip("Time between frames in microseconds\n(I didn't want to break things by trying to print the mu)");

			if (ImGui::TreeNode("Framerate Graph")) {					// Graph of framerate over time
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
		
		ImGui::ColorEdit4("Background Color", pBackgroundColor);
		
		ImGui::Spacing();
		ImGui::Text("NOTE: Tint is now applied per Entity!\nIt can be found in the Entities section.");
		
		ImGui::Spacing();
	}

	if (ImGui::CollapsingHeader("Meshes")) {					// Info about each mesh
		ImGui::Spacing();

		for (int i = 0; i < meshes.size(); i++) {

			// Each mesh gets its own Tree Node
			ImGui::PushID("MESH" + i);
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

		ImGui::Spacing();
	}

	if (ImGui::CollapsingHeader("Entities")) {				// Info about each entity
		ImGui::Spacing();

		// Store the position, rotation, scale, and tint of each entity as they're read in
		

		for (int i = 0; i < entities.size(); i++) {
			// Get position, rotation, scale, and tint
			XMFLOAT3 entityPos = entities[i]->GetTransform()->GetPosition();
			XMFLOAT3 entityRot = entities[i]->GetTransform()->GetRotation();
			XMFLOAT3 entitySca = entities[i]->GetTransform()->GetScale();
			XMFLOAT4 entityTin = entities[i]->GetTint();

			// Each entity gets its own Tree Node
			ImGui::PushID("ENTITY" + i);
			if (ImGui::TreeNode("", "(%06d) %s", i, entities[i]->GetName())) {
				ImGui::Spacing();

				ImGui::Text("Mesh:      %s", (entities[i]->GetMesh()->GetName()));
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
				
				ImGui::Spacing();
				if (ImGui::ColorEdit4("Tint", &entityTin.x)) {
					entities[i]->SetTint(entityTin);
				}

				ImGui::TreePop();
				ImGui::Spacing();
			}
			ImGui::PopID();
		}

		ImGui::Spacing();
	}

	if (ImGui::CollapsingHeader("Cameras")) {				// Info about each camera
		ImGui::Spacing();

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
			ImGui::PushID("CAMERA" + i);
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
					cameras[i]->SetNearClip(cameraNear);
				}
				if (ImGui::DragFloat("Far Clip", &cameraFar, 1.0f, 10.0f, 10000.0f, "%.0f", ImGuiSliderFlags_Logarithmic)) {
					cameras[i]->SetFarClip(cameraFar);
				}

				ImGui::TreePop();
				ImGui::Spacing();
			}
			ImGui::PopID();

		}

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
