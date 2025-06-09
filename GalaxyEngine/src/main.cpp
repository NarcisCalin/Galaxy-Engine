#include "globalLogic.h"

int main(int argc, char** argv) {

	// SPH Materials initialization
	SPHMaterials::Init();

	SetConfigFlags(FLAG_MSAA_4X_HINT);

	SetConfigFlags(FLAG_WINDOW_RESIZABLE);

	int threadsAvailable = std::thread::hardware_concurrency();

	myVar.threadsAmount = static_cast<int>(threadsAvailable * 0.5f);

	std::cout << "Threads available: " << threadsAvailable << std::endl;
	std::cout << "Thread amount set to: " << myVar.threadsAmount << std::endl;

	InitWindow(myVar.screenWidth, myVar.screenHeight, "Galaxy Engine");

	Image icon = LoadImage("Textures/WindowIcon.png");
	SetWindowIcon(icon);

	rlImGuiSetup(true);

	Texture2D particleBlurTex = LoadTexture("Textures/ParticleBlur.png");

	Shader myBloom = LoadShader(nullptr, "Shaders/bloom.fs");

	RenderTexture2D myParticlesTexture = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
	RenderTexture2D myUITexture = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());

	SetTargetFPS(myVar.targetFPS);

	int lastScreenWidth = GetScreenWidth();
	int lastScreenHeight = GetScreenHeight();
	bool wasFullscreen = IsWindowFullscreen();

	bool lastScreenState = false;


	// If "Saves" directory doesn't exist, then create one. This is done here to store the default parameters
	if (!std::filesystem::exists("Saves")) {
		std::filesystem::create_directory("Saves");
	}
	save.saveFlag = true;
	save.saveSystem("Saves/DefaultSettings.bin", myVar, myParam, sph);
	save.saveFlag = false;

	ImGuiStyle& style = ImGui::GetStyle();
	ImVec4* colors = style.Colors;

	// Button and window colors
	colors[ImGuiCol_WindowBg] = myVar.colWindowBg;
	colors[ImGuiCol_Button] = myVar.colButton;
	colors[ImGuiCol_ButtonHovered] = myVar.colButtonHover;
	colors[ImGuiCol_ButtonActive] = myVar.colButtonPress;

	// Slider colors
	style.Colors[ImGuiCol_SliderGrab] = myVar.colSliderGrab;        // Bright cyan-ish knob
	style.Colors[ImGuiCol_SliderGrabActive] = myVar.colSliderGrabActive;  // Darker when dragging

	style.Colors[ImGuiCol_FrameBg] = myVar.colSliderBg;           // Dark track when idle
	style.Colors[ImGuiCol_FrameBgHovered] = myVar.colSliderBgHover;    // Lighter track on hover
	style.Colors[ImGuiCol_FrameBgActive] = myVar.colSliderBgActive;     // Even lighter on active

	// Tab colors
	style.Colors[ImGuiCol_Tab] = myVar.colButton;
	style.Colors[ImGuiCol_TabHovered] = myVar.colButtonHover;
	style.Colors[ImGuiCol_TabActive] = myVar.colButtonPress;

	ImGuiIO& io = ImGui::GetIO();

	ImFontConfig config;
	config.SizePixels = 14.0f;        // Base font size in pixels
	config.RasterizerDensity = 2.0f;  // Improves rendering at small sizes
	config.OversampleH = 4;           // Horizontal anti-aliasing
	config.OversampleV = 4;           // Vertical anti-aliasing
	config.PixelSnapH = false;        // Disable pixel snapping for smoother text
	config.RasterizerMultiply = 0.9f; // Slightly boosts brightness

	myVar.robotoMediumFont = io.Fonts->AddFontFromFileTTF(
		"fonts/Roboto-Medium.ttf",
		config.SizePixels,
		&config,
		io.Fonts->GetGlyphRangesDefault()
	);

	if (!myVar.robotoMediumFont) {
		std::cerr << "Failed to load special font!\n";
	}
	else {
		std::cout << "Special font loaded successfully\n";
	}

	io.Fonts->Build();
	rlImGuiReloadFonts();
	ImPlot::CreateContext();


	while (!WindowShouldClose()) {

		fullscreenToggle(lastScreenWidth, lastScreenHeight, wasFullscreen, lastScreenState, myParticlesTexture, myUITexture);

		BeginTextureMode(myParticlesTexture);

		ClearBackground(BLACK);

		BeginBlendMode(myParam.colorVisuals.blendMode);

		BeginMode2D(myParam.myCamera.cameraLogic(save.loadFlag, myVar.isMouseNotHoveringUI));

		rlImGuiBegin();

		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 2.0f);

		updateScene();

		drawScene(particleBlurTex, myUITexture);

		EndMode2D();

		EndBlendMode();


		//------------------------ RENDER TEXTURES BELOW ------------------------//

		if (myVar.isGlowEnabled) {
			BeginShaderMode(myBloom);
		}

		//ClearBackground(BLACK);

		//BeginBlendMode(BLEND_ALPHA_PREMULTIPLY);

		DrawTextureRec(
			myParticlesTexture.texture,
			Rectangle{ 0, 0, static_cast<float>(GetScreenWidth()), -static_cast<float>(GetScreenHeight()) },
			Vector2{ 0, 0 },
			WHITE
		);

		if (myVar.isGlowEnabled) {
			EndShaderMode();
		}

		DrawTextureRec(
			myUITexture.texture,
			Rectangle{ 0, 0, static_cast<float>(GetScreenWidth()), -static_cast<float>(GetScreenHeight()) },
			Vector2{ 0, 0 },
			WHITE
		);

		EndBlendMode();

		// Detects if the user is recording the screen
		myVar.isRecording = myParam.screenCapture.screenGrab(myParticlesTexture, myVar, myParam);

		if (myVar.isRecording) {
			DrawRectangleLinesEx({ 0,0, static_cast<float>(GetScreenWidth()), static_cast<float>(GetScreenHeight()) }, 3, RED);
		}

		ImGui::PopStyleVar();

		rlImGuiEnd();


		EndDrawing();

		enableMultiThreading();
	}

	rlImGuiShutdown();
	ImPlot::DestroyContext();

	UnloadShader(myBloom);
	UnloadTexture(particleBlurTex);
	UnloadRenderTexture(myParticlesTexture);
	UnloadRenderTexture(myUITexture);
	UnloadImage(icon);

	CloseWindow();



	return 0;
}