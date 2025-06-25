#include "globalLogic.h"

int main(int argc, char** argv) {

	// SPH Materials initialization
	SPHMaterials::Init();

	SetConfigFlags(FLAG_MSAA_4X_HINT);

	SetConfigFlags(FLAG_WINDOW_RESIZABLE);

	SetConfigFlags(FLAG_WINDOW_ALWAYS_RUN);

	int threadsAvailable = std::thread::hardware_concurrency();

	myVar.threadsAmount = static_cast<int>(threadsAvailable * 0.5f);

	std::cout << "Threads available: " << threadsAvailable << std::endl;
	std::cout << "Thread amount set to: " << myVar.threadsAmount << std::endl;

	if (myVar.fullscreenState) {
		myVar.screenWidth = GetMonitorWidth(GetCurrentMonitor());
		myVar.screenHeight = GetMonitorHeight(GetCurrentMonitor());
	}

	InitWindow(myVar.screenWidth, myVar.screenHeight, "Galaxy Engine");

	// ---- Config ---- //

	if (!std::filesystem::exists("Config")) {
		std::filesystem::create_directory("Config");
	}

	if (!std::filesystem::exists("Config/config.txt")) {
		saveConfig();
	}
	else {
		loadConfig();
	}

	// ---- Audio ---- //

	geSound.loadSounds();

	// ---- Icon ---- //

	Image icon = LoadImage("Textures/WindowIcon.png");
	SetWindowIcon(icon);

	// ---- Textures & rlImGui ---- //

	rlImGuiSetup(true);

	Texture2D particleBlurTex = LoadTexture("Textures/ParticleBlur.png");

	Shader myBloom = LoadShader(nullptr, "Shaders/bloom.fs");

	RenderTexture2D myParticlesTexture = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
	RenderTexture2D myUITexture = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
	RenderTexture2D myMiscTexture = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());

	SetTargetFPS(myVar.targetFPS);

	// ---- Fullscreen ---- //

	int lastScreenWidth = GetScreenWidth();
	int lastScreenHeight = GetScreenHeight();
	bool wasFullscreen = IsWindowFullscreen();

	bool lastScreenState = false;

	// ---- Save ---- //

	// If "Saves" directory doesn't exist, then create one. This is done here to store the default parameters
	if (!std::filesystem::exists("Saves")) {
		std::filesystem::create_directory("Saves");
	}

	save.saveFlag = true;
	save.saveSystem("Saves/DefaultSettings.bin", myVar, myParam, sph, physics);
	save.saveFlag = false;

	// ---- ImGui ---- //

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

	// ---- Intro ---- //

	bool fadeActive = true;
	bool introActive = true;

	myVar.customFont = LoadFontEx("fonts/Unispace Bd.otf", myVar.introFontSize, 0, 250);

	SetTextureFilter(myVar.customFont.texture, TEXTURE_FILTER_BILINEAR);

	if (myVar.customFont.texture.id == 0) {
		TraceLog(LOG_WARNING, "Failed to load font! Using default font");
	}

	if (myVar.fullscreenState) {
		myVar.screenWidth = GetMonitorWidth(GetCurrentMonitor()) * 0.5f;
		myVar.screenHeight = GetMonitorHeight(GetCurrentMonitor()) * 0.5f;
	}

	while (!WindowShouldClose()) {

		fullscreenToggle(lastScreenWidth, lastScreenHeight, wasFullscreen, lastScreenState, myParticlesTexture, myUITexture);

		BeginTextureMode(myParticlesTexture);

		ClearBackground(BLACK);

		BeginBlendMode(myParam.colorVisuals.blendMode);

		BeginMode2D(myParam.myCamera.cameraLogic(save.loadFlag, myVar.isMouseNotHoveringUI));

		rlImGuiBegin();

		if (introActive) {
			ImGuiIO& io = ImGui::GetIO();
			io.WantCaptureMouse = true;
			io.WantCaptureKeyboard = true;
			io.WantTextInput = true;

			if (myParam.pParticles.size() > 0) {
				myParam.pParticles.clear();
				myParam.rParticles.clear();
			}
		}
		else {
			geSound.soundtrackLogic();
		}

		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 2.0f);

		saveConfigIfChanged();

		updateScene();

		drawScene(particleBlurTex, myUITexture, myMiscTexture, fadeActive, introActive);

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

		DrawTextureRec(
			myMiscTexture.texture,
			Rectangle{ 0, 0, static_cast<float>(GetScreenWidth()), -static_cast<float>(GetScreenHeight()) },
			Vector2{ 0, 0 },
			WHITE
		);


		EndDrawing();

		enableMultiThreading();
	}

	rlImGuiShutdown();
	ImPlot::DestroyContext();

	UnloadShader(myBloom);
	UnloadTexture(particleBlurTex);

	UnloadRenderTexture(myParticlesTexture);
	UnloadRenderTexture(myUITexture);
	UnloadRenderTexture(myMiscTexture);

	UnloadImage(icon);

	geSound.unloadSounds();

	CloseWindow();



	return 0;
}