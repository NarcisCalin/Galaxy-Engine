#include "globalLogic.h"

#if defined(PLATFORM_DESKTOP)
#define GLSL_VERSION            330
#else   // PLATFORM_ANDROID, PLATFORM_WEB
#define GLSL_VERSION            100
#endif

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

	RenderTexture2D myParticlesTexture = CreateFloatRenderTexture(GetScreenWidth(), GetScreenHeight());
	RenderTexture2D myRayTracingTexture = CreateFloatRenderTexture(GetScreenWidth(), GetScreenHeight());
	RenderTexture2D myUITexture = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
	RenderTexture2D myMiscTexture = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());

	SetTargetFPS(myVar.targetFPS);

	// ---- Fullscreen ---- //

	int lastScreenWidth = GetScreenWidth();
	int lastScreenHeight = GetScreenHeight();
	bool wasFullscreen = IsWindowMaximized();

	bool lastScreenState = false;

	// ---- Save ---- //

	// If "Saves" directory doesn't exist, then create one. This is done here to store the default parameters
	if (!std::filesystem::exists("Saves")) {
		std::filesystem::create_directory("Saves");
	}

	save.saveFlag = true;
	save.saveSystem("Saves/DefaultSettings.bin", myVar, myParam, sph, physics, physics3D, lighting, field);
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
	style.Colors[ImGuiCol_FrameBgHovered] = myVar.colSliderBgHover;     // Lighter track on hover
	style.Colors[ImGuiCol_FrameBgActive] = myVar.colSliderBgActive;      // Even lighter on active

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
	ImPlot::CreateContext();

	// ---- Intro ---- //

	bool fadeActive = false;
	bool introActive = false;

	myVar.customFont = LoadFontEx("fonts/Unispace Bd.otf", myVar.introFontSize, 0, 250);

	SetTextureFilter(myVar.customFont.texture, TEXTURE_FILTER_BILINEAR);

	if (myVar.customFont.texture.id == 0) {
		TraceLog(LOG_WARNING, "Failed to load font! Using default font");
	}

	if (myVar.fullscreenState) {
		myVar.screenWidth = GetMonitorWidth(GetCurrentMonitor()) * 0.5f;
		myVar.screenHeight = GetMonitorHeight(GetCurrentMonitor()) * 0.5f;
	}

	// ---- Ray Tracing and Long Exposure ---- //

	const char* accumulationVs = R"(
    #version 330 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec2 vertexTexCoord;
layout(location = 3) in vec4 vertexColor;

out vec2 fragTexCoord;
out vec4 fragColor;

uniform mat4 mvp;

void main()
{

    gl_Position = mvp * vec4(vertexPosition, 1.0);


    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;

}
    )";

	const char* accumulationFs = R"(

#version 330

precision highp float;
precision highp sampler2D;

in vec2 fragTexCoord;
out vec4 finalColor;

uniform sampler2D currentFrame;
uniform sampler2D accumulatedFrame;
uniform float sampleCount;

void main() {

    highp vec4 newColor = texture(currentFrame, fragTexCoord);
    highp vec4 oldColor = texture(accumulatedFrame, fragTexCoord);

    finalColor = (oldColor * (sampleCount - 1.0) + newColor) / sampleCount;

    finalColor = clamp(finalColor, 0.0, 1.0);
}
)";

	Shader accumulationShader = LoadShaderFromMemory(accumulationVs, accumulationFs);

	int screenSizeLoc = GetShaderLocation(accumulationShader, "screenSize");
	float screenSize[2] = {
		(float)myVar.screenWidth,
		(float)myVar.screenHeight
	};
	SetShaderValue(accumulationShader, screenSizeLoc, screenSize, SHADER_UNIFORM_VEC2);

	int rayTextureLoc = GetShaderLocation(accumulationShader, "rayTexture");

	RenderTexture2D accumulatedTexture = CreateFloatRenderTexture(GetScreenWidth(), GetScreenHeight());
	RenderTexture2D pingPongTexture = CreateFloatRenderTexture(GetScreenWidth(), GetScreenHeight());

	int currentFrameLoc = GetShaderLocation(accumulationShader, "currentFrame");
	int accumulatedFrameLoc = GetShaderLocation(accumulationShader, "accumulatedFrame");
	int sampleCountLoc = GetShaderLocation(accumulationShader, "sampleCount");
	RenderTexture2D testSampleTexture = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());

	int prevScreenWidth = GetScreenWidth();
	int prevScreenHeight = GetScreenHeight();

	bool prevLongExpFlag = false;

	bool accumulationCondition = false;

	buildKernels();

	myVar.hasAVX2 = hasAVX2Support();

	rlSetClipPlanes(1.0f, 50000.0f);

	// ================= SKYBOX INITIALIZATION ================= //

	Mesh cubeSky = GenMeshCube(10000.0f, 10000.0f, 10000.0f);
	Model skybox = LoadModelFromMesh(cubeSky);

	// Load skybox shader
	skybox.materials[0].shader = LoadShader(TextFormat("Shaders/skybox.vs", GLSL_VERSION),
		TextFormat("Shaders/skybox.fs", GLSL_VERSION));

	int environmentMap = MATERIAL_MAP_CUBEMAP;
	SetShaderValue(skybox.materials[0].shader, GetShaderLocation(skybox.materials[0].shader, "environmentMap"), &environmentMap, SHADER_UNIFORM_INT);

	int doGamma = 0;
	SetShaderValue(skybox.materials[0].shader, GetShaderLocation(skybox.materials[0].shader, "doGamma"), &doGamma, SHADER_UNIFORM_INT);

	int vflipped = 0;
	SetShaderValue(skybox.materials[0].shader, GetShaderLocation(skybox.materials[0].shader, "vflipped"), &vflipped, SHADER_UNIFORM_INT);


	Image faces[6] = {
		LoadImage("Textures/sky_pos_x.png"),   // +X
		LoadImage("Textures/sky_neg_x.png"),    // -X
		LoadImage("Textures/sky_pos_y.png"),     // +Y
		LoadImage("Textures/sky_neg_y.png"),  // -Y
		LoadImage("Textures/sky_pos_z.png"),   // +Z
		LoadImage("Textures/sky_neg_z.png")     // -Z
	};

	int width = faces[0].width;
	int height = faces[0].height;

	Image verticalStrip = GenImageColor(width, height * 6, BLACK);

	for (int i = 0; i < 6; i++) {

		ImageDraw(&verticalStrip, faces[i],
			(Rectangle) {
			0, 0, (float)width, (float)height
		},
			(Rectangle) {
			0, (float)(i * height), (float)width, (float)height
		},
			WHITE);

		UnloadImage(faces[i]);
	}

	skybox.materials[0].maps[MATERIAL_MAP_CUBEMAP].texture = LoadTextureCubemap(verticalStrip, CUBEMAP_LAYOUT_LINE_VERTICAL);

	UnloadImage(verticalStrip);

	while (!WindowShouldClose()) {

		if (myVar.exitGame) {
			CloseWindow();
			break;
		}

		fullscreenToggle(lastScreenWidth, lastScreenHeight, wasFullscreen, lastScreenState, myParticlesTexture, myUITexture);

		if (myVar.is3DMode && myParam.colorVisuals.blendMode == 0) {
			ClearBackground({ 0,0,0,0 });
		}

		BeginTextureMode(myParticlesTexture);

		ClearBackground(BLACK);

		BeginBlendMode(myParam.colorVisuals.blendMode);

		if (myVar.is3DMode) {

			BeginMode3D(myParam.myCamera3D.cameraLogic(save.loadFlag, myVar.isMouseNotHoveringUI, myVar.firstPerson, ship.isShipEnabled));

			rlDisableBackfaceCulling();
			rlDisableDepthMask();
			DrawModel(skybox, (Vector3) { 0, 0, 0 }, 1.0f, WHITE);
			rlEnableBackfaceCulling();
			rlEnableDepthMask();

			mode3D();

			drawMode3DRecording(particleBlurTex);

			if (!myVar.isRecording) {
				drawMode3DNonRecording();
			}

			EndMode3D();
		}

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

		drawScene(particleBlurTex, myRayTracingTexture, myUITexture, myMiscTexture, fadeActive, introActive);

		EndMode2D();

		EndBlendMode();


		//------------------------ RENDER TEXTURES BELOW ------------------------//

		if (myVar.isGlowEnabled) {
			BeginShaderMode(myBloom);
		}

		if (myVar.isGlowEnabled) {
			EndShaderMode();
		}

		if (myParam.myCamera.cameraChangedThisFrame) {
			lighting.shouldRender = true;
		}

		if (myVar.longExposureFlag != prevLongExpFlag) {
			myVar.longExposureCurrent = 1;

			prevLongExpFlag = myVar.longExposureFlag;
		}

		if (myVar.isOpticsEnabled) {
			accumulationCondition = lighting.currentSamples <= lighting.maxSamples;
		}
		else if (myVar.longExposureFlag) {
			accumulationCondition = myVar.longExposureCurrent <= myVar.longExposureDuration;
		}
		else {
			accumulationCondition = true;
		}

		if (GetScreenWidth() != prevScreenWidth || GetScreenHeight() != prevScreenHeight) {

			UnloadRenderTexture(accumulatedTexture);
			UnloadRenderTexture(pingPongTexture);
			UnloadRenderTexture(testSampleTexture);

			accumulatedTexture = CreateFloatRenderTexture(GetScreenWidth(), GetScreenHeight());
			pingPongTexture = CreateFloatRenderTexture(GetScreenWidth(), GetScreenHeight());
			testSampleTexture = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());

			screenSize[0] = (float)GetScreenWidth();
			screenSize[1] = (float)GetScreenHeight();
			SetShaderValue(accumulationShader, screenSizeLoc, screenSize, SHADER_UNIFORM_VEC2);

			prevScreenWidth = GetScreenWidth();
			prevScreenHeight = GetScreenHeight();

			if (myVar.isOpticsEnabled) {
				lighting.shouldRender = true;
			}

			if (myVar.longExposureFlag) {
				myVar.longExposureFlag = false;
			}
		}

		// Ray Tracing and Long Exposure
		if (accumulationCondition) {

			BeginTextureMode(pingPongTexture);

			BeginShaderMode(accumulationShader);

			SetShaderValueTexture(accumulationShader, currentFrameLoc, myParticlesTexture.texture);
			SetShaderValueTexture(accumulationShader, accumulatedFrameLoc, accumulatedTexture.texture);

			float sampleCount = 1.0f;

			if (myVar.isOpticsEnabled) {
				sampleCount = static_cast<float>(lighting.currentSamples);
			}
			else {
				lighting.currentSamples = 0;
			}

			if (myVar.longExposureFlag) {
				sampleCount = static_cast<float>(myVar.longExposureCurrent);
			}
			else {
				myVar.longExposureCurrent = 0;
			}

			SetShaderValue(accumulationShader, sampleCountLoc, &sampleCount, SHADER_UNIFORM_FLOAT);

			DrawTextureRec(
				accumulatedTexture.texture,
				Rectangle{ 0, 0, (float)GetScreenWidth(), -((float)GetScreenHeight()) },
				Vector2{ 0, 0 },
				WHITE
			);

			EndShaderMode();

			EndTextureMode();

			std::swap(accumulatedTexture, pingPongTexture);


			if (myVar.longExposureFlag) {
				myVar.longExposureCurrent++;
			}
		}

		DrawTextureRec(
			accumulatedTexture.texture,
			Rectangle{ 0, 0, (float)GetScreenWidth(), -((float)GetScreenHeight()) },
			Vector2{ 0, 0 },
			WHITE
		);


		DrawTextureRec(
			myUITexture.texture,
			Rectangle{ 0, 0, static_cast<float>(GetScreenWidth()), -static_cast<float>(GetScreenHeight()) },
			Vector2{ 0, 0 },
			WHITE
		);

		EndBlendMode();


		// Detects if the user is recording the screen
		myVar.isRecording = myParam.screenCapture.screenGrab(accumulatedTexture, myVar, myParam);

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
	UnloadRenderTexture(myRayTracingTexture);
	UnloadRenderTexture(myUITexture);
	UnloadRenderTexture(myMiscTexture);

	UnloadShader(skybox.materials[0].shader);
	UnloadTexture(skybox.materials[0].maps[MATERIAL_MAP_CUBEMAP].texture);

	UnloadImage(icon);

	geSound.unloadSounds();

	// Unload accumulation shader
	UnloadShader(accumulationShader);

	// Free compute shader memory
	freeGPUMemory();

	if (std::filesystem::exists(myVar.playbackPath)) {
		std::filesystem::remove(myVar.playbackPath);
	}

	CloseWindow();



	return 0;
}