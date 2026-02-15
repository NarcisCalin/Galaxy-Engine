#include "UI/UI.h"

void UI::uiLogic(UpdateParameters& myParam, UpdateVariables& myVar, SPH& sph, SaveSystem& save, GESound& geSound, 
	Lighting& lighting, Field& field, ParticleSpaceship& ship) {

	if (IO::shortcutPress(KEY_U)) {
		showSettings = !showSettings;
	}

	if (myVar.timeFactor == 0.0f) {
		DrawRectangleV({ GetScreenWidth() - 700.0f, 20.0f }, { 10.0f, 30.0f }, WHITE);
		DrawRectangleV({ GetScreenWidth() - 720.0f, 20.0f }, { 10.0f, 30.0f }, WHITE);
	}

	if (ImGui::IsAnyItemHovered() || ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow)) {
		myVar.isMouseNotHoveringUI = false;
		myVar.isDragging = false;
	}
	else {
		myVar.isMouseNotHoveringUI = true;
	}

	ImGui::GetIO().IniFilename = nullptr;

	float screenX = static_cast<float>(GetScreenWidth());
	float screenY = static_cast<float>(GetScreenHeight());

	float buttonsWindowX = 220.0f;
	float buttonsWindowY = screenY - 30.0f;

	float settingsButtonX = 250.0f;
	float settingsButtonY = 25.0f;

	float parametersSliderX = 200.0f;
	float parametersSliderY = 30.0f;

	bool enabled = true;

	ImGui::SetNextWindowSize(ImVec2(buttonsWindowX, buttonsWindowY), ImGuiCond_Once);
	ImGui::SetNextWindowSizeConstraints(ImVec2(buttonsWindowX, buttonsWindowY), ImVec2(buttonsWindowX, buttonsWindowY));
	ImGui::SetNextWindowPos(ImVec2(screenX - buttonsWindowX, 0.0f), ImGuiCond_Always);
	ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_NoResize);

	float contentRegionWidth = ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x;
	float buttonX = (contentRegionWidth - settingsButtonX) * 0.5f;

	float oldSpacingY = ImGui::GetStyle().ItemSpacing.y;
	ImGui::GetStyle().ItemSpacing.y = 5.0f; // Set the spacing only for the settings buttons

	std::vector<SimilarTypeButton::Mode> controlsAndInfo{
{ "Controls", "Open controls panel", &myParam.controls.isShowControlsEnabled },
{ "Information", "Open information panel", &myParam.controls.isInformationEnabled }
	};

	std::vector<SimilarTypeButton::Mode> trails{
{ "Global Trails", "Enables trails for all particles", &myVar.isGlobalTrailsEnabled },
{ "Selected Trails", "Enables trails for selected particles", &myVar.isSelectedTrailsEnabled }
	};

	std::vector<SimilarTypeButton::Mode> size{
{ "Density Size", "Maps particle neighbor amount to size", &myVar.isDensitySizeEnabled },
{ "Force Size", "Maps particle acceleration to size", &myVar.isForceSizeEnabled }
	};

	std::vector<SimilarTypeButton::Mode> gpuSimd{
{ "GPU (Beta)", "Simulates gravity on the GPU", &myVar.isGPUEnabled },
{ "Naive SIMD", "Simulates gravity with a Naive algorithm with SIMD vectorization (Recommended to leave turned off)", &myVar.naiveSIMD }
	};

	ImGui::Spacing();
	ImGui::Separator();

	ImGui::TextColored(UpdateVariables::colMenuInformation, "General");

	ImGui::Separator();
	ImGui::Spacing();

	buttonHelper("Fullscreen", "Toggles fulscreen", myVar.fullscreenState, -1.0f, settingsButtonY, true, enabled);

	SimilarTypeButton::buttonIterator(controlsAndInfo, -1.0f, settingsButtonY, true, enabled);

	buttonHelper("Multi-Threading", "Distributes the simulation across multiple threads", myVar.isMultiThreadingEnabled, -1.0f, settingsButtonY, true, enabled);

	SimilarTypeButton::buttonIterator(gpuSimd, -1.0f, settingsButtonY, true, enabled);

	if (myVar.is3DMode) {
		myVar.isGPUEnabled = false;
	}

	if (buttonHelper("3D Mode (BETA)", "Enables 3D simulation", myVar.is3DMode, -1.0f, settingsButtonY, true, enabled)) {
		myParam.pParticles.clear();
		myParam.rParticles.clear();

		myParam.pParticlesSelected.clear();
		myParam.rParticlesSelected.clear();

		myParam.pParticles3D.clear();
		myParam.rParticles3D.clear();

		myParam.pParticlesSelected3D.clear();
		myParam.rParticlesSelected3D.clear();

		myParam.trails.segments.clear();
		myParam.trails.segments3D.clear();

	}

	ImGui::Spacing();
	ImGui::Separator();

	ImGui::TextColored(UpdateVariables::colMenuInformation, "Exit");

	ImGui::Separator();
	ImGui::Spacing();

	buttonHelper("Exit Galaxy Engine", "Are you sure you don't want to play a little more?", myVar.exitGame, -1.0f, settingsButtonY, true, enabled);

	ImGui::Spacing();
	ImGui::Separator();

	ImGui::TextColored(UpdateVariables::colMenuInformation, "Save/Load");

	ImGui::Separator();
	ImGui::Spacing();

	buttonHelper("Save Scene", "Save current scene to disk", save.saveFlag, -1.0f, settingsButtonY, true, enabled);
	buttonHelper("Load Scene", "Load a scene from disk", save.loadFlag, -1.0f, settingsButtonY, true, enabled);

	ImGui::Spacing();
	ImGui::Separator();

	ImGui::TextColored(UpdateVariables::colMenuInformation, "Trails");

	ImGui::Separator();
	ImGui::Spacing();

	SimilarTypeButton::buttonIterator(trails, -1.0f, settingsButtonY, true, enabled);

	buttonHelper("Local Trails", "Enables trails moving relative to particles average position", myVar.isLocalTrailsEnabled, -1.0f, settingsButtonY, true, enabled);
	buttonHelper("White Trails", "Makes all trails white", myParam.trails.whiteTrails, -1.0f, settingsButtonY, true, enabled);

	ImGui::Spacing();
	ImGui::Separator();

	ImGui::TextColored(UpdateVariables::colMenuInformation, "Visuals");

	ImGui::Separator();
	ImGui::Spacing();

	bool* colorModesArray[] = {
		&myParam.colorVisuals.solidColor,
		&myParam.colorVisuals.densityColor,
		&myParam.colorVisuals.forceColor,
		&myParam.colorVisuals.velocityColor,
		&myParam.colorVisuals.shockwaveColor,
		&myParam.colorVisuals.turbulenceColor,
		&myParam.colorVisuals.pressureColor,
		&myParam.colorVisuals.temperatureColor,
		&myParam.colorVisuals.gasTempColor,
		&myParam.colorVisuals.SPHColor
	};

	const char* colorModes[] = { "Solid Color", "Density Color", "Force Color", "Velocity Color", "Shockwave Color", "Turbulence Color", "Pressure Color", "Temperature Color",
	"Temperature Gas Color", "Material Color" };

	const char* colorModeTips[] = {
		"Particles will only use the primary color",
		"Maps particle neighbor amount to primary and secondary colors",
		"Maps particle acceleration to primary and secondary colors",
		"Maps particle velocity to color",
		"Maps particle acceleration to color",
		"Maps particle turbulence to primary and secondary colors",
		"Maps particle pressure to color",
		"Maps particle temperature to color",
		"Maps particle temperature to primary and secondary colors",
		"Uses materials colors",
	};
	static int currentColorMode = 1;

	if (myVar.loadDropDownMenus) {
		for (int i = 0; i < IM_ARRAYSIZE(colorModesArray); i++) {

			if (*colorModesArray[i]) {
				currentColorMode = i;
				break;
			}
		}
	}

	ImGui::PushItemWidth(-FLT_MIN);

	if (ImGui::BeginCombo("##Color", colorModes[currentColorMode])) {
		for (int i = 0; i < IM_ARRAYSIZE(colorModes); i++) {

			bool isSelected = (currentColorMode == i);

			if (ImGui::Selectable(colorModes[i], isSelected)) {
				currentColorMode = i;
			}

			if (isSelected) {
				ImGui::SetItemDefaultFocus();
			}

			if (ImGui::IsItemHovered()) {
				ImGui::BeginTooltip();
				ImGui::TextUnformatted(colorModeTips[i]);
				ImGui::EndTooltip();
			}


		}
		ImGui::EndCombo();

		for (int i = 0; i < IM_ARRAYSIZE(colorModesArray); ++i) {
			*colorModesArray[i] = false;
		}

		*colorModesArray[currentColorMode] = true;
	}

	ImGui::Spacing();

	SimilarTypeButton::buttonIterator(size, -1.0f, settingsButtonY, true, enabled);

	ImGui::PopItemWidth();

	ImGui::Spacing();
	ImGui::Separator();

	ImGui::TextColored(UpdateVariables::colMenuInformation, "Simulation");

	ImGui::Separator();
	ImGui::Spacing();

	static bool galaxyModeDummy = false;

	bool* simModesArray[] = {
		&galaxyModeDummy,
		&myVar.isSPHEnabled,
		&myVar.isMergerEnabled
	};

	const char* simModes[] = { "Galaxy Mode", "Fluid Mode", "Merger" };

	const char* simModeTips[] = {
		"Default simulation mode. Used for very large scale objects like galaxies or the Big Bang",
		"Enables SPH fluids. Used for planets or small scale simulations",
		"Colliding particles will merge together"
	};
	static int currentSimMode = 0;

	bool foundSimMode = false;
	if (myVar.loadDropDownMenus) {
		bool anyEnabled = false;

		for (int i = 1; i < IM_ARRAYSIZE(simModesArray); i++) {
			if (*simModesArray[i]) {
				currentSimMode = i;
				anyEnabled = true;
				break;
			}
		}

		galaxyModeDummy = !anyEnabled;
		if (!anyEnabled) {
			currentSimMode = 0;
		}
	}

	ImGui::PushItemWidth(-FLT_MIN);

	bool wasSPHEnabled = myVar.isSPHEnabled;
	bool wasMergerEnabled = myVar.isMergerEnabled;

	if (ImGui::BeginCombo("##Simulation", simModes[currentSimMode])) {
		for (int i = 0; i < IM_ARRAYSIZE(simModes); i++) {

			bool isSelected = (currentSimMode == i);

			if (ImGui::Selectable(simModes[i], isSelected)) {
				currentSimMode = i;
			}

			if (isSelected) {
				ImGui::SetItemDefaultFocus();
			}

			if (ImGui::IsItemHovered()) {
				ImGui::BeginTooltip();
				ImGui::TextUnformatted(simModeTips[i]);
				ImGui::EndTooltip();
			}


		}
		ImGui::EndCombo();

		for (int i = 0; i < IM_ARRAYSIZE(simModesArray); ++i) {
			*simModesArray[i] = false;
		}

		*simModesArray[currentSimMode] = true;

		bool anyModeActive = false;
		for (int i = 0; i < IM_ARRAYSIZE(simModesArray); ++i) {
			if (*simModesArray[i]) {
				anyModeActive = true;
				break;
			}
		}
		if (!anyModeActive) {
			galaxyModeDummy = true;
			currentSimMode = 0;
		}

		if (!wasSPHEnabled && myVar.isSPHEnabled) {
			for (size_t i = 0; i < IM_ARRAYSIZE(colorModesArray); i++) {
				*colorModesArray[i] = (colorModesArray[i] == &myParam.colorVisuals.SPHColor);
				if (colorModesArray[i] == &myParam.colorVisuals.SPHColor) {
					currentColorMode = i;

					myVar.SPHWater = true;
				}
			}
		}

		if (!wasMergerEnabled && myVar.isMergerEnabled) {
			for (size_t i = 0; i < IM_ARRAYSIZE(colorModesArray); i++) {
				*colorModesArray[i] = (colorModesArray[i] == &myParam.colorVisuals.solidColor);
				if (colorModesArray[i] == &myParam.colorVisuals.solidColor) {
					currentColorMode = i;

					myParam.colorVisuals.pColor = { 255,255,255,255 };
				}
			}
		}

		foundSimMode = false;
		for (int i = 0; i < IM_ARRAYSIZE(simModesArray); i++) {
			if (*simModesArray[i]) {
				currentSimMode = i;
				foundSimMode = true;
				break;
			}
		}
		if (!foundSimMode) {
			galaxyModeDummy = true;
			currentSimMode = 0;
		}
	}

	ImGui::PopItemWidth();

	ImGui::Spacing();
	ImGui::Separator();

	ImGui::TextColored(UpdateVariables::colMenuInformation, "Fluid Mode Material");

	ImGui::Separator();
	ImGui::Spacing();

	bool* materialsArray[] = {
		&myVar.SPHWater,
		&myVar.SPHRock,
		&myVar.SPHIron,
		&myVar.SPHSand,
		&myVar.SPHSoil,
		&myVar.SPHIce,
		&myVar.SPHMud,
		&myVar.SPHRubber,
		&myVar.SPHGas
	};

	const char* materials[] = { "Water", "Rock", "Iron", "Sand", "Soil", "Ice", "Mud", "Rubber", "Gas" };

	static int currentMat = 0;

	if (myVar.loadDropDownMenus) {
		for (int i = 0; i < IM_ARRAYSIZE(materialsArray); i++) {

			if (*materialsArray[i]) {
				currentMat = i;
				break;
			}
		}
	}

	ImGui::PushItemWidth(-FLT_MIN);

	ImGui::BeginDisabled(!myVar.isSPHEnabled);

	if (ImGui::BeginCombo("##Materials", materials[currentMat])) {
		for (int i = 0; i < IM_ARRAYSIZE(materials); i++) {

			bool isSelected = (currentMat == i);

			if (ImGui::Selectable(materials[i], isSelected)) {
				currentMat = i;
			}

			if (isSelected) {
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();

		for (int i = 0; i < IM_ARRAYSIZE(materialsArray); ++i) {
			*materialsArray[i] = false;
		}

		*materialsArray[currentMat] = true;
	}


	ImGui::PopItemWidth();

	ImGui::EndDisabled();

	ImGui::Spacing();
	ImGui::Separator();

	ImGui::TextColored(UpdateVariables::colMenuInformation, "Dark Matter");

	ImGui::Separator();
	ImGui::Spacing();

	buttonHelper("Dark Matter", "Enables dark matter particles. This works for galaxies and Big Bang", myVar.isDarkMatterEnabled, -1.0f, settingsButtonY, true, enabled);
	buttonHelper("Show Dark Matter", "Unhides dark matter particles", myParam.colorVisuals.showDarkMatterEnabled, -1.0f, settingsButtonY, true, enabled);

	ImGui::Spacing();
	ImGui::Separator();

	ImGui::TextColored(UpdateVariables::colMenuInformation, "Space Modifiers");

	ImGui::Separator();
	ImGui::Spacing();

	buttonHelper("Fluid Ground Mode", "Adds vertical gravity and makes particles collide with the domain walls", myVar.sphGround, -1.0f, settingsButtonY, true, myVar.isSPHEnabled);
	buttonHelper("Looping Space", "Particles disappearing on one side will appear on the other side", myVar.isPeriodicBoundaryEnabled, -1.0f, settingsButtonY, true, enabled);
	buttonHelper("Infinite Domain", "Enables or disables the domain boundaries that contain the simulation", myVar.infiniteDomain, -1.0f, settingsButtonY, true, enabled);

	ImGui::Spacing();
	ImGui::Separator();

	ImGui::TextColored(UpdateVariables::colMenuInformation, "Temperature");

	ImGui::Separator();
	ImGui::Spacing();

	buttonHelper("Temperature Simulation", "Enables temperature simulation", myVar.isTempEnabled, -1.0f, settingsButtonY, true, enabled);

	ImGui::Spacing();
	ImGui::Separator();

	ImGui::TextColored(UpdateVariables::colMenuInformation, "Constraints");

	ImGui::Separator();
	ImGui::Spacing();

	buttonHelper("Particle Constraints", "Enables particles constraints for solids and soft bodies simulation. Works best with Fluid Mode enabled", myVar.constraintsEnabled, -1.0f, settingsButtonY, true, enabled);
	buttonHelper("Unbreakable Constraints", "Makes all constraints unbreakable", myVar.unbreakableConstraints, -1.0f, settingsButtonY, true, myVar.constraintsEnabled);
	buttonHelper("Constraint After Drawing", "Creates constraints in between particles right after drawing them", myVar.constraintAfterDrawing, -1.0f, settingsButtonY, true, myVar.constraintsEnabled);

	if (buttonHelper("Visualize Constraints", "Draws all existing constraints", myVar.drawConstraints, -1.0f, settingsButtonY, true, myVar.constraintsEnabled)) {
		myVar.visualizeMesh = false;
	}
	if (buttonHelper("Visualize Mesh", "Draws a mesh that connect particles", myVar.visualizeMesh, -1.0f, settingsButtonY, true, enabled)) {
		myVar.drawConstraints = false;
	}

	buttonHelper("Constraint Stress Color", "Maps the constraints stress to an RGB color", myVar.constraintStressColor, -1.0f, settingsButtonY, true, myVar.drawConstraints);

	ImGui::Spacing();
	ImGui::Separator();

	ImGui::TextColored(UpdateVariables::colMenuInformation, "Optics");

	ImGui::Separator();
	ImGui::Spacing();

	buttonHelper("Optics", "Enables light simulation with ray tracing. Simulate light from the Optics tab", myVar.isOpticsEnabled, -1.0f, settingsButtonY, true, enabled);

	ImGui::Spacing();
	ImGui::Separator();

	ImGui::TextColored(UpdateVariables::colMenuInformation, "Fields");

	ImGui::Separator();
	ImGui::Spacing();

	bool isNot3DMode = !myVar.is3DMode;

	if (isNot3DMode) {
		myVar.isGravityFieldEnabled = false;
		myVar.gravityFieldDMParticles = false;
	}

	if (buttonHelper("Gravity Field", "Enables the gravity field visualization mode (IT IS RECOMMENDED TO USE SMALLER DOMAIN SIZES)", myVar.isGravityFieldEnabled, -1.0f, settingsButtonY, true, isNot3DMode)) {
		field.computeField = true;
	}
	buttonHelper("DM Particles", "Enables ignores Dark Matter particles for the gravity field", myVar.gravityFieldDMParticles, -1.0f, settingsButtonY, true, isNot3DMode);

	ImGui::Spacing();
	ImGui::Separator();

	ImGui::Spacing();
	ImGui::TextColored(UpdateVariables::colMenuInformation, "Misc.");

	ImGui::Separator();
	ImGui::Spacing();

	buttonHelper("Highlight Selected", "Highlight selected particles", myParam.colorVisuals.selectedColor, -1.0f, settingsButtonY, true, enabled);
	buttonHelper("Glow", "Enables glow shader", myVar.isGlowEnabled, -1.0f, settingsButtonY, true, enabled);
	buttonHelper("Predict Path", "Predicts the trajectory of heavy particles before launching them", myVar.enablePathPrediction, -1.0f, settingsButtonY, true, enabled);

	ImGui::GetStyle().ItemSpacing.y = oldSpacingY; // End the settings buttons spacing

	ImGui::End();

	// Start of settings sliders

	float parametersWindowSizeX = 400.0f;
	float parametersWindowSizeY = screenY - 30.0f;

	ImGui::SetNextWindowSize(ImVec2(parametersWindowSizeX, parametersWindowSizeY), ImGuiCond_Once);
	ImGui::SetNextWindowSizeConstraints(ImVec2(parametersWindowSizeX, parametersWindowSizeY), ImVec2(parametersWindowSizeX, parametersWindowSizeY));
	ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
	ImGui::Begin("Parameters", nullptr, ImGuiWindowFlags_NoResize);
	float totalWidth = ImGui::GetContentRegionAvail().x;
	float halfButtonWidth = totalWidth * 0.4f;

	if (ImGui::BeginTabBar("##MainTabBar", ImGuiTabBarFlags_NoTabListScrollingButtons)) {

		if (ImGui::BeginTabItem("Visuals")) {

			bVisualsSliders = true;
			bPhysicsSliders = false;
			bStatsWindow = false;
			bRecordingSettings = false;
			bSoundWindow = false;
			bLightingWindow = false;

			// Initialize all tabs for sliders defaults
			if (loadSettings) {
				bVisualsSliders = true;
				bPhysicsSliders = true;
				bStatsWindow = true;
				bRecordingSettings = true;
				bSoundWindow = true;
				bLightingWindow = true;

				loadSettings = false;
			}

			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Physics")) {

			bVisualsSliders = false;
			bPhysicsSliders = true;
			bStatsWindow = false;
			bRecordingSettings = false;
			bSoundWindow = false;
			bLightingWindow = false;

			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Advanced Stats")) {

			bVisualsSliders = false;
			bPhysicsSliders = false;
			bStatsWindow = true;
			bRecordingSettings = false;
			bSoundWindow = false;
			bLightingWindow = false;

			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Optics")) {

			bVisualsSliders = false;
			bPhysicsSliders = false;
			bStatsWindow = false;
			bRecordingSettings = false;
			bSoundWindow = false;
			bLightingWindow = true;

			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Sound")) {

			bVisualsSliders = false;
			bPhysicsSliders = false;
			bStatsWindow = false;
			bRecordingSettings = false;
			bSoundWindow = true;
			bLightingWindow = false;

			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Recording")) {

			bVisualsSliders = false;
			bPhysicsSliders = false;
			bStatsWindow = false;
			bRecordingSettings = true;
			bSoundWindow = false;
			bLightingWindow = false;

			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}

	ImGui::BeginChild("##ContentRegion", ImVec2(0, 0), true); {

		if (bVisualsSliders) {

			ImGui::Spacing();
			ImGui::Separator();

			ImGui::TextColored(UpdateVariables::colMenuInformation, "Colors");

			ImGui::Separator();
			ImGui::Spacing();

			Color primaryColors = {
				static_cast<unsigned char>(myParam.colorVisuals.pColor.r),
				static_cast<unsigned char>(myParam.colorVisuals.pColor.g),
				static_cast<unsigned char>(myParam.colorVisuals.pColor.b),
				static_cast<unsigned char>(myParam.colorVisuals.pColor.a) };

			ImVec4 imguiPColor = rlImGuiColors::Convert(primaryColors);
			static Color originalPColor = primaryColors;

			bool placeholderP = false;

			if (buttonHelper("Reset Primary Color", "Resets the secondary color picker", placeholderP, 240.0f, 30.0f, true, enabled)) {
				myParam.colorVisuals.pColor.r = originalPColor.r;
				myParam.colorVisuals.pColor.g = originalPColor.g;
				myParam.colorVisuals.pColor.b = originalPColor.b;
				myParam.colorVisuals.pColor.a = originalPColor.a;
			}

			if (ImGui::ColorPicker4("Primary Color", (float*)&imguiPColor, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_DisplayRGB)) {
				primaryColors = rlImGuiColors::Convert(imguiPColor);
				myParam.colorVisuals.pColor.r = primaryColors.r;
				myParam.colorVisuals.pColor.g = primaryColors.g;
				myParam.colorVisuals.pColor.b = primaryColors.b;
				myParam.colorVisuals.pColor.a = primaryColors.a;
			}

			Color secondaryColors = {
				static_cast<unsigned char>(myParam.colorVisuals.sColor.r),
				static_cast<unsigned char>(myParam.colorVisuals.sColor.g),
				static_cast<unsigned char>(myParam.colorVisuals.sColor.b),
				static_cast<unsigned char>(myParam.colorVisuals.sColor.a) };

			ImVec4 imguiSColor = rlImGuiColors::Convert(secondaryColors);
			static Color originalSColor = secondaryColors;

			bool placeholderS = false;

			if (buttonHelper("Reset Secondary Col.", "Resets the primary color picker", placeholderS, 240.0f, 30.0f, true, enabled)) {
				myParam.colorVisuals.sColor.r = originalSColor.r;
				myParam.colorVisuals.sColor.g = originalSColor.g;
				myParam.colorVisuals.sColor.b = originalSColor.b;
				myParam.colorVisuals.sColor.a = originalSColor.a;
			}

			if (ImGui::ColorPicker4("Secondary Col.", (float*)&imguiSColor, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_DisplayRGB)) {
				secondaryColors = rlImGuiColors::Convert(imguiSColor);
				myParam.colorVisuals.sColor.r = secondaryColors.r;
				myParam.colorVisuals.sColor.g = secondaryColors.g;
				myParam.colorVisuals.sColor.b = secondaryColors.b;
				myParam.colorVisuals.sColor.a = secondaryColors.a;
			}

			ImGui::Spacing();
			ImGui::Separator();

			ImGui::TextColored(UpdateVariables::colMenuInformation, "Camera");

			ImGui::Separator();
			ImGui::Spacing();

			buttonHelper("First Person Camera", "Enables first person mode. Use the arrow buttons to move", myVar.firstPerson, 240.0f, 30.0f, true, enabled);

			sliderHelper("Camera Arrows Speed", "Controls the speed of the camera when moving with arrows", myParam.myCamera3D.arrowMoveSpeed, 0.001f, 5000.0f, parametersSliderX, parametersSliderY, enabled, LogSlider);

			ImGui::Spacing();
			ImGui::Separator();

			ImGui::TextColored(UpdateVariables::colMenuInformation, "Particle Clipping");

			ImGui::Separator();
			ImGui::Spacing();

			buttonHelper("Clip Selected X", "Hides half of the selected particles on the X axis", myVar.clipSelectedX, 240.0f, 30.0f, true, enabled);
			ImGui::SameLine();
			buttonHelper("Invert X", "Inverts X half", myVar.clipSelectedXInv, 75.0f, 30.0f, true, enabled);

			buttonHelper("Clip Selected Y", "Hides half of the selected particles on the Y axis", myVar.clipSelectedY, 240.0f, 30.0f, true, enabled);
			ImGui::SameLine();
			buttonHelper("Invert Y", "Inverts Y half", myVar.clipSelectedYInv, 75.0f, 30.0f, true, enabled);

			buttonHelper("Clip Selected Z", "Hides half of the selected particles on the Z axis", myVar.clipSelectedZ, 240.0f, 30.0f, true, enabled);
			ImGui::SameLine();
			buttonHelper("Invert Z", "Inverts Z half", myVar.clipSelectedZInv, 75.0f, 30.0f, true, enabled);

			ImGui::Spacing();
			ImGui::Separator();

			ImGui::TextColored(UpdateVariables::colMenuInformation, "Neighbor Search");

			ImGui::Separator();
			ImGui::Spacing();

			sliderHelper("Max Neighbors", "Controls the maximum neighbor count range", myParam.colorVisuals.maxNeighbors, 1, 2000, parametersSliderX, parametersSliderY, enabled);

			ImGui::Spacing();
			ImGui::Separator();

			ImGui::TextColored(UpdateVariables::colMenuInformation, "Color Parameters");

			ImGui::Separator();
			ImGui::Spacing();

			sliderHelper("Max Force Color", "Controls the acceleration threshold to use the secondary color", myParam.colorVisuals.maxColorAcc, 1.0f, 400.0f, parametersSliderX, parametersSliderY, enabled, LogSlider);
			sliderHelper("Max Velocity Color", "Controls the max velocity used to map the colors in the velocity color mode", myParam.colorVisuals.maxVel, 10.0f, 10000.0f, parametersSliderX, parametersSliderY, enabled, LogSlider);
			sliderHelper("Max Shockwave Accel", "Controls the acceleration threshold to map the particle color in Shockwave color mode", myParam.colorVisuals.ShockwaveMaxAcc, 1.0f, 120.0f, parametersSliderX, parametersSliderY, enabled, LogSlider);
			ImGui::Separator();
			sliderHelper("Max Turbulence Color", "Controls the turbulence threshold to use the secondary color", myParam.colorVisuals.maxColorTurbulence, 1.0f, 512.0f, parametersSliderX, parametersSliderY, enabled, LogSlider);
			sliderHelper("Turbulence Fade Rate", "Controls how fast turbulence fades away", myParam.colorVisuals.turbulenceFadeRate, 0.00f, 1.0f, parametersSliderX, parametersSliderY, enabled, LogSlider);
			sliderHelper("Turbulence Contrast", "Controls how much contrast turbulence color has", myParam.colorVisuals.turbulenceContrast, 0.1f, 4.0f, parametersSliderX, parametersSliderY, enabled, LogSlider);
			buttonHelper("Turbulence Custom Colors", "Enables the use of primary and secondary colors for turbulence", myParam.colorVisuals.turbulenceCustomCol, 212.0f, 24.0f, true, enabled);
			ImGui::Separator();
			sliderHelper("Max Pressure Color", "Controls the max pressure used to map the colors in the pressure color mode", myParam.colorVisuals.maxPress, 100.0f, 100000.0f, parametersSliderX, parametersSliderY, enabled, LogSlider);
			sliderHelper("Max Temperature Color", "Controls the max temperature used to map the colors in the temperature color mode", myParam.colorVisuals.tempColorMaxTemp, 10.0f, 50000.0f, parametersSliderX, parametersSliderY, enabled, LogSlider);
			sliderHelper("Max Constraint Stress", "Controls the max constraint stress used to map the colors in the constraints stress color mode. If set to 0, it will set the max stress to the material's breaking limit", myVar.constraintMaxStressColor, 0.0f, 1.0f, parametersSliderX, parametersSliderY, enabled, LogSlider);

			ImGui::Spacing();
			ImGui::Separator();

			ImGui::TextColored(UpdateVariables::colMenuInformation, "Size Parameters");

			ImGui::Separator();
			ImGui::Spacing();

			sliderHelper("Max Dynamic Size", "Controls the maximum size particles can have when chaning size dynamically", myParam.densitySize.maxSize, 0.1f, 5.0f, parametersSliderX, parametersSliderY, enabled, LogSlider);
			sliderHelper("Min Dynamic Size", "Controls the minimum size particles can have when chaning size dynamically", myParam.densitySize.minSize, 0.001f, 5.0f, parametersSliderX, parametersSliderY, enabled, LogSlider);
			sliderHelper("Max Size Force", "Controls the acceleration threshold to map the particle size", myParam.densitySize.sizeAcc, 1.0f, 400.0f, parametersSliderX, parametersSliderY, enabled, LogSlider);
			sliderHelper("Max Size Neighbors", "Controls the neighbors threshold to map the particle size", myParam.densitySize.maxNeighbors, 1, 1000, parametersSliderX, parametersSliderY, enabled);
			sliderHelper("Particles Size", "Controls the size of all particles", myVar.particleSizeMultiplier, 0.0f, 5.0f, parametersSliderX, parametersSliderY, enabled, LogSlider);

			ImGui::Spacing();
			ImGui::Separator();

			ImGui::TextColored(UpdateVariables::colMenuInformation, "Trails Parameters");

			ImGui::Separator();
			ImGui::Spacing();

			sliderHelper("Trails Length", "Controls how long should the trails be. This feature is computationally expensive", myVar.trailMaxLength, 0, 1500, parametersSliderX, parametersSliderY, enabled);
			sliderHelper("Trails Thickness", "Controls the trails thickness", myParam.trails.trailThickness, 0.01f, 1.5f, parametersSliderX, parametersSliderY, enabled);

			ImGui::Spacing();
			ImGui::Separator();

			ImGui::TextColored(UpdateVariables::colMenuInformation, "Field Parameters");

			ImGui::Separator();
			ImGui::Spacing();

			sliderHelper("Field Res", "Controls how much gravity affects the field colors", field.res, 50, 1000, parametersSliderX, parametersSliderY, enabled);
			sliderHelper("Gravity Display Threshold", "Controls how much gravity affects the field colors", field.gravityDisplayThreshold, 10.0f, 3000.0f, parametersSliderX, parametersSliderY, enabled, LogSlider);
			sliderHelper("Gravity Display Softness", "Controls how soft the gravity display looks", field.gravityDisplaySoftness, 0.4f, 8.0f, parametersSliderX, parametersSliderY, enabled, LogSlider);
			sliderHelper("Gravity Display Stretch", "Controls how contrasty the gravity display looks", field.gravityStretchFactor, 1.0f, 10000.0f, parametersSliderX, parametersSliderY, enabled, LogSlider);
			buttonHelper("Gravity Custom Colors", "Enables the use of primary and secondary colors for the gravity field", field.gravityCustomColors, 212.0f, 24.0f, true, enabled);
			sliderHelper("Gravity Custom Color Exp.", "Controls the exposure of the custom color mode for the gravity field", field.gravityExposure, 0.001f, 15.0f, parametersSliderX, parametersSliderY, enabled, LogSlider);

			ImGui::Spacing();
			ImGui::Separator();

			ImGui::TextColored(UpdateVariables::colMenuInformation, "Misc. Parameters");

			ImGui::Separator();
			ImGui::Spacing();

			sliderHelper("Path Prediction Length", "Controls how long is the predicted path", myVar.predictPathLength, 100, 2000, parametersSliderX, parametersSliderY, enabled);

			bool isSPHDisabled = !myVar.isSPHEnabled;

			static bool prevSPHState = false;
			static bool prevMassMultiplierEnabled = false;
			static float prevMassScatter = 0.0f;

			if (myVar.isSPHEnabled != prevSPHState) {
				if (myVar.isSPHEnabled) {
					prevMassMultiplierEnabled = myParam.particlesSpawning.massMultiplierEnabled;
					prevMassScatter = myVar.massScatter;

					myParam.particlesSpawning.massMultiplierEnabled = false;
					myVar.massScatter = 0.0f;
				}
				else {
					myParam.particlesSpawning.massMultiplierEnabled = prevMassMultiplierEnabled;
					myVar.massScatter = prevMassScatter;
				}
			}

			prevSPHState = myVar.isSPHEnabled;

			ImGui::Spacing();
		}

		if (bPhysicsSliders) {

			ImGui::Spacing();
			ImGui::Separator();

			ImGui::TextColored(UpdateVariables::colMenuInformation, "System Parameters");

			ImGui::Separator();
			ImGui::Spacing();

			sliderHelper("Threads Amount", "Controls the amount of threads used by the simulation. Half your total amount of threads is usually the sweet spot", myVar.threadsAmount, 1, 32, parametersSliderX, parametersSliderY, enabled);

			ImGui::Spacing();
			ImGui::Separator();

			ImGui::TextColored(UpdateVariables::colMenuInformation, "Simulation Parameters");

			ImGui::Separator();
			ImGui::Spacing();

			sliderHelper("Theta", "Controls the quality of the gravity calculation. Higher means lower quality", myVar.theta, 0.1f, 5.0f, parametersSliderX, parametersSliderY, enabled, LogSlider);

			if (!myVar.is3DMode) {
				sliderHelper("Domain Width", "Controls the width of the global container", myVar.domainSize.x, 200.0f, 3840.0f, parametersSliderX, parametersSliderY, enabled);
				sliderHelper("Domain Height", "Controls the height of the global container", myVar.domainSize.y, 200.0f, 2160.0f, parametersSliderX, parametersSliderY, enabled);
			}
			else {
				sliderHelper("Domain Width", "Controls the width of the global container", myVar.domainSize3D.x, 200.0f, 3840.0f, parametersSliderX, parametersSliderY, enabled);
				sliderHelper("Domain Height", "Controls the height of the global container", myVar.domainSize3D.y, 200.0f, 2160.0f, parametersSliderX, parametersSliderY, enabled);
				sliderHelper("Domain Depth", "Controls the depth of the global container", myVar.domainSize3D.z, 200.0f, 2160.0f, parametersSliderX, parametersSliderY, enabled);
			}

			ImGui::Spacing();
			ImGui::Separator();

			ImGui::TextColored(UpdateVariables::colMenuInformation, " General Physics Parameters");

			ImGui::Separator();
			ImGui::Spacing();

			sliderHelper("Time Scale", "Controls how fast time passes", myVar.timeStepMultiplier, 0.0f, 15.0f, parametersSliderX, parametersSliderY, enabled, LogSlider);
			sliderHelper("Softening", "Controls the smoothness of the gravity forces", myVar.softening, 0.5f, 30.0f, parametersSliderX, parametersSliderY, enabled, LogSlider);
			sliderHelper("Gravity Strength", "Controls how much particles attract eachother", myVar.gravityMultiplier, 0.0f, 100.0f, parametersSliderX, parametersSliderY, enabled, LogSlider);

			ImGui::Spacing();
			ImGui::Separator();

			ImGui::TextColored(UpdateVariables::colMenuInformation, "Temperature Parameters");

			ImGui::Separator();
			ImGui::Spacing();

			sliderHelper("Ambient Temperature", "Controls the desired temperature of the scene in Kelvin. 1 is near absolute zero. The default value is set just high enough to allow liquid water", myVar.ambientTemp, 1.0f, 2500.0f, parametersSliderX, parametersSliderY, enabled, LogSlider);
			sliderHelper("Ambient Heat Rate", "Controls how fast particles' temperature try to match ambient temperature", myVar.globalAmbientHeatRate, 0.0f, 10.0f, parametersSliderX, parametersSliderY, enabled, LogSlider);
			sliderHelper("Heat Conductivity Multiplier", "Controls the global heat conductivity of particles", myVar.globalHeatConductivity, 0.001f, 1.0f, parametersSliderX, parametersSliderY, enabled, LogSlider);

			ImGui::Spacing();
			ImGui::Separator();

			ImGui::TextColored(UpdateVariables::colMenuInformation, "Constraints Parameters");

			ImGui::Separator();
			ImGui::Spacing();

			sliderHelper("Constraints Stiffness Multiplier", "Controls the global stiffness multiplier for constraints", myVar.globalConstraintStiffnessMult, 0.001f, 3.0f, parametersSliderX, parametersSliderY, enabled, LogSlider);
			sliderHelper("Constraints Resistance Multiplier", "Controls the global resistance multiplier for constraints", myVar.globalConstraintResistance, 0.001f, 30.0f, parametersSliderX, parametersSliderY, enabled, LogSlider);

			ImGui::Spacing();
			ImGui::Separator();

			ImGui::TextColored(UpdateVariables::colMenuInformation, "Fluids Parameters");

			ImGui::Separator();
			ImGui::Spacing();

			sliderHelper("Fluid Vertical Gravity", "Controls the vertical gravity strength in Fluid Ground Mode", myVar.verticalGravity, 0.0f, 10.0f, parametersSliderX, parametersSliderY, enabled, LogSlider);
			sliderHelper("Fluid Mass Multiplier", "Controls the fluid mass of particles", myVar.mass, 0.005f, 0.15f, parametersSliderX, parametersSliderY, enabled, LogSlider);
			sliderHelper("Fluid Viscosity", "Controls how viscous particles are", myVar.viscosity, 0.01f, 15.0f, parametersSliderX, parametersSliderY, enabled, LogSlider);
			sliderHelper("Fluid Stiffness", "Controls how stiff particles are", myVar.stiffMultiplier, 0.01f, 15.0f, parametersSliderX, parametersSliderY, enabled, LogSlider);
			sliderHelper("Fluid Cohesion", "Controls how sticky particles are", myVar.cohesionCoefficient, 0.0f, 10.0f, parametersSliderX, parametersSliderY, enabled, LogSlider);
			sliderHelper("Fluid Delta", "Controls the scaling factor in the pressure solver to enforce fluid incompressibility", myVar.delta, 500.0f, 20000.0f, parametersSliderX, parametersSliderY, enabled, LogSlider);
			sliderHelper("Fluid Max Velocity", "Controls the maximum velocity a particle can have in Fluid mode", myVar.sphMaxVel, 0.0f, 2000.0f, parametersSliderX, parametersSliderY, enabled, LogSlider);
		}

		if (bSoundWindow) {

			bool enabled = true;

			ImGui::Spacing();
			ImGui::Separator();

			ImGui::TextColored(UpdateVariables::colMenuInformation, "General Sound Parameters");

			ImGui::Separator();
			ImGui::Spacing();

			sliderHelper("Global Volume", "Controls global sound volume", geSound.globalVolume, 0.0f, 1.0f, parametersSliderX, parametersSliderY, enabled);
			sliderHelper("Menu Volume", "Controls menu sounds volume", geSound.menuVolume, 0.0f, 1.0f, parametersSliderX, parametersSliderY, enabled);
			sliderHelper("Music Volume", "Controls soundtrack volume", geSound.musicVolume, 0.0f, 1.0f, parametersSliderX, parametersSliderY, enabled);

			ImGui::Spacing();
			ImGui::Separator();

			ImGui::TextColored(UpdateVariables::colMenuInformation, "Soundtrack Parameters");

			ImGui::Separator();
			ImGui::Spacing();

			if (buttonHelper("<- Previous Track", "Plays the previous track in the playlist", geSound.hasTrackChanged, -1.0f, settingsButtonY, true, enabled)) {
				geSound.currentSongIndex--;
			}

			if (buttonHelper("Next Track ->", "Plays the next track in the playlist", geSound.hasTrackChanged, -1.0f, settingsButtonY, true, enabled)) {
				geSound.currentSongIndex++;
			}
		}

		if (bRecordingSettings) {

			float oldSpacingY = ImGui::GetStyle().ItemSpacing.y;
			ImGui::GetStyle().ItemSpacing.y = 5.0f; // Set the spacing only for the recording settings buttons

			ImGui::Spacing();
			ImGui::Separator();

			ImGui::TextColored(UpdateVariables::colMenuInformation, "General Recording Parameters");

			ImGui::Separator();
			ImGui::Spacing();

			buttonHelper("Pause After Recording", "Pauses the simulation after recording is finished", myVar.pauseAfterRecording, -1.0f, settingsButtonY, true, enabled);
			buttonHelper("Clean Scene After Recording", "Clears all particles from the scene after recording is finished", myVar.cleanSceneAfterRecording, -1.0f, settingsButtonY, true, enabled);

			ImGui::Separator();

			bool isEnabled = true;
			if (myVar.isRecording) {
				isEnabled = false;
			}
			sliderHelper("Recording Time Limit", "Set a time limit for the recording. 0 means no limit.", myVar.recordingTimeLimit, 0.0f, 60.0f, parametersSliderX, parametersSliderY, isEnabled);


			ImGui::GetStyle().ItemSpacing.y = oldSpacingY;

			ImGui::Spacing();
			ImGui::Separator();

			ImGui::TextColored(UpdateVariables::colMenuInformation, "Simulation Recording");

			ImGui::Separator();
			ImGui::Spacing();

			buttonHelper("Store Playback On Memory", "Stores the playback on memory instead of disk", myVar.playBackOnMemory, -1.0f, settingsButtonY, true, enabled);

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			sliderHelper("Keyframe Interval", "Sets the number of frames in between each stored frame", myVar.keyframeTickInterval, 1, 30, parametersSliderX, parametersSliderY, enabled);

			std::string buttonText;
			std::string tooltipText;

			if (myVar.playbackRecord)
			{
				buttonText = "Stop Recording";
				tooltipText = "Stop simulation recording";
			}
			else if (!myParam.playbackFrames.empty())
			{
				buttonText = "Resume Simulation";
				tooltipText = "Resume recording the simulation";
			}
			else
			{
				buttonText = "Record Simulation";
				tooltipText = "Start recording simulation frames";
			}

			if (buttonHelper(buttonText.c_str(), tooltipText.c_str(), myVar.playbackRecord, -1.0f, settingsButtonY, true, enabled)) {
				
				if (!myVar.playbackRecord) {
					myVar.runPlayback = true;

					std::swap(myParam.pParticles3D, myParam.pParticles3DPlaybackResume);
					std::swap(myParam.rParticles3D, myParam.rParticles3DPlaybackResume);
				}


				if (!myParam.playbackFrames.empty() && myVar.playbackRecord) {

					std::swap(myParam.pParticles3D, myParam.pParticles3DPlaybackResume);
					std::swap(myParam.rParticles3D, myParam.rParticles3DPlaybackResume);

					myParam.pParticles3DPlaybackResume.clear();
					myParam.rParticles3DPlaybackResume.clear();
				}
			}

			bool enablePlaybackControls = false;

			if (!myParam.playbackFrames.empty() && !myVar.playbackRecord) {
				enablePlaybackControls = true;
			}

			if (myVar.playbackRecord) {
				myVar.runPlayback = false;
			}

			buttonHelper("Run Playback", "Plays the recorded simulation", myVar.runPlayback, -1.0f, settingsButtonY, true, enablePlaybackControls);

			sliderHelper("Playback speed", "Sets how fast the playback is played", myVar.playbackSpeed, 0.0f, 20.0f, parametersSliderX, parametersSliderY, enablePlaybackControls, LogSlider);

			if (sliderHelper("Playback Timeline", "Scroll through the playback frames", myVar.playbackProgress, 0.0f, static_cast<float>(myParam.playbackFrames.size() - 2), parametersSliderX, parametersSliderY, enablePlaybackControls)) {
				myVar.runPlayback = false;
			}

			sliderHelper("Playback Particles Size", "Modifies the size of playback particles", myVar.playbackParticlesSizeMult, 0.0f, 5.0f, parametersSliderX, parametersSliderY, enablePlaybackControls, LogSlider);

			if (buttonHelper("Delete Playback", "Deletes playback and resumes simulation", myVar.deletePlayback, -1.0f, settingsButtonY, true, enabled) && !myVar.playbackRecord) {
				myVar.runPlayback = false;

				myParam.playbackFrames.clear();

				std::swap(myParam.pParticles3D, myParam.pParticles3DPlaybackResume);
				std::swap(myParam.rParticles3D, myParam.rParticles3DPlaybackResume);

				myParam.pParticles3DPlaybackResume.clear();
				myParam.rParticles3DPlaybackResume.clear();

				if (std::filesystem::exists(myVar.playbackPath)) {
					std::filesystem::remove(myVar.playbackPath);
				}

				myVar.deletePlayback = false;
			}
		
			size_t totalBytes = 0;
			for (const auto& frame : myParam.playbackFrames) totalBytes += frame.size() * sizeof(PlaybackParticle);
			double totalMB = totalBytes / (1024.0 * 1024.0);

			ImGui::Text("Total Space: %.2f MB", totalMB);
		}

		if (bStatsWindow) {
			statsWindowLogic(myParam, myVar);
		}

		if (bLightingWindow) {

			bool enabled = true;

			ImVec4 imguiLightColor = rlImGuiColors::Convert(lighting.lightColor);

			Color imguiLightColorRl;

			ImGui::Spacing();
			ImGui::Separator();

			ImGui::TextColored(UpdateVariables::colMenuInformation, "Color Settings");

			ImGui::Separator();
			ImGui::Spacing();

			if (ImGui::ColorPicker3("Light Color", (float*)&imguiLightColor, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_DisplayRGB)) {
				imguiLightColorRl = rlImGuiColors::Convert(imguiLightColor);
				lighting.lightColor.r = imguiLightColorRl.r;
				lighting.lightColor.g = imguiLightColorRl.g;
				lighting.lightColor.b = imguiLightColorRl.b;
				lighting.lightColor.a = imguiLightColorRl.a;

				lighting.isSliderLightColor = true;
			}

			ImVec4 imguiWallBaseColor = rlImGuiColors::Convert(lighting.wallBaseColor);

			Color imguiWallBaseColorRl;

			if (ImGui::ColorPicker3("Wall Base Color", (float*)&imguiWallBaseColor, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_DisplayRGB)) {
				imguiWallBaseColorRl = rlImGuiColors::Convert(imguiWallBaseColor);
				lighting.wallBaseColor.r = imguiWallBaseColorRl.r;
				lighting.wallBaseColor.g = imguiWallBaseColorRl.g;
				lighting.wallBaseColor.b = imguiWallBaseColorRl.b;
				lighting.wallBaseColor.a = imguiWallBaseColorRl.a;

				lighting.isSliderBaseColor = true;
			}

			ImVec4 imguiWallSpecularColor = rlImGuiColors::Convert(lighting.wallSpecularColor);

			Color imguiWallSpecularColorRl;

			if (ImGui::ColorPicker3("Wall Specular Color", (float*)&imguiWallSpecularColor, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_DisplayRGB)) {
				imguiWallSpecularColorRl = rlImGuiColors::Convert(imguiWallSpecularColor);
				lighting.wallSpecularColor.r = imguiWallSpecularColorRl.r;
				lighting.wallSpecularColor.g = imguiWallSpecularColorRl.g;
				lighting.wallSpecularColor.b = imguiWallSpecularColorRl.b;
				lighting.wallSpecularColor.a = imguiWallSpecularColorRl.a;

				lighting.isSliderSpecularColor = true;
			}

			ImVec4 imguiWallRefractionColor = rlImGuiColors::Convert(lighting.wallRefractionColor);

			Color imguiWallRefractionColorRl;

			if (ImGui::ColorPicker3("Wall Refraction Color", (float*)&imguiWallRefractionColor, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_DisplayRGB)) {
				imguiWallRefractionColorRl = rlImGuiColors::Convert(imguiWallRefractionColor);
				lighting.wallRefractionColor.r = imguiWallRefractionColorRl.r;
				lighting.wallRefractionColor.g = imguiWallRefractionColorRl.g;
				lighting.wallRefractionColor.b = imguiWallRefractionColorRl.b;
				lighting.wallRefractionColor.a = imguiWallRefractionColorRl.a;

				lighting.isSliderRefractionCol = true;
			}

			ImVec4 imguiWallEmissionColor = rlImGuiColors::Convert(lighting.wallEmissionColor);

			Color imguiWallEmissionColorRl;

			if (ImGui::ColorPicker3("Wall Emission Color", (float*)&imguiWallEmissionColor, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_DisplayRGB)) {
				imguiWallEmissionColorRl = rlImGuiColors::Convert(imguiWallEmissionColor);
				lighting.wallEmissionColor.r = imguiWallEmissionColorRl.r;
				lighting.wallEmissionColor.g = imguiWallEmissionColorRl.g;
				lighting.wallEmissionColor.b = imguiWallEmissionColorRl.b;
				lighting.wallEmissionColor.a = imguiWallEmissionColorRl.a;

				lighting.isSliderEmissionCol = true;
			}

			ImGui::Spacing();
			ImGui::Separator();

			ImGui::TextColored(UpdateVariables::colMenuInformation, "Light Settings");

			ImGui::Separator();
			ImGui::Spacing();

			if (sliderHelper("Light Gain", "Controls lights brightness", lighting.lightGain, 0.0f, 1.0f, parametersSliderX, parametersSliderY, enabled, LogSlider)) {
				lighting.isSliderLightGain = true;
			}

			if (sliderHelper("Light Spread", "Controls the spread of area and cone lights", lighting.lightSpread, 0.0f, 1.0f, parametersSliderX, parametersSliderY, enabled)) {
				lighting.isSliderlightSpread = true;
			}

			ImGui::Spacing();
			ImGui::Separator();

			ImGui::TextColored(UpdateVariables::colMenuInformation, "Wall Material Settings");

			ImGui::Separator();
			ImGui::Spacing();

			if (sliderHelper("Wall Specular Roughness", "Controls the specular reflections roughness of walls", lighting.wallSpecularRoughness, 0.0f, 1.0f, parametersSliderX, parametersSliderY, enabled)) {
				lighting.isSliderSpecularRough = true;
			}
			if (sliderHelper("Wall Refraction Roughness", "Controls the refraction surface roughness of walls", lighting.wallRefractionRoughness, 0.0f, 1.0f, parametersSliderX, parametersSliderY, enabled)) {
				lighting.isSliderRefractionRough = true;
			}

			if (sliderHelper("Wall Refraction Amount", "Controls how much light walls will refract", lighting.wallRefractionAmount, 0.0f, 1.0f, parametersSliderX, parametersSliderY, enabled)) {
				lighting.isSliderRefractionAmount = true;;
			}

			if (sliderHelper("Wall IOR", "Controls the IOR of walls", lighting.wallIOR, 0.0f, 100.0f, parametersSliderX, parametersSliderY, enabled)) {
				lighting.isSliderIor = true;
			}

			if (sliderHelper("Wall Dispersion", "Controls how much light gets dispersed after refracting from this wall", lighting.wallDispersion, 0.0f, 0.2f, parametersSliderX, parametersSliderY, enabled)) {
				lighting.isSliderDispersion = true;
			}

			if (sliderHelper("Wall Emission Gain", "Controls how much light walls emit", lighting.wallEmissionGain, 0.0f, 1.0f, parametersSliderX, parametersSliderY, enabled)) {
				lighting.isSliderEmissionGain = true;
			}

			ImGui::Spacing();
			ImGui::Separator();

			ImGui::TextColored(UpdateVariables::colMenuInformation, "Shape Settings");

			ImGui::Separator();
			ImGui::Spacing();

			sliderHelper("Shape Relax Iter.", "Controls the iterations used to relax the shapes when drawing", lighting.shapeRelaxIter, 0, 50, parametersSliderX, parametersSliderY, enabled);
			sliderHelper("Shape Relax Factor", "Controls how much the drawn shape should relax each iteration", lighting.shapeRelaxFactor, 0.0f, 1.0f, parametersSliderX, parametersSliderY, enabled);

			ImGui::Spacing();
			ImGui::Separator();

			ImGui::TextColored(UpdateVariables::colMenuInformation, "Render Settings");

			ImGui::Separator();
			ImGui::Spacing();

			if (sliderHelper("Max Samples", "Controls the total amount of lighting iterations", lighting.maxSamples, 1, 2048, parametersSliderX, parametersSliderY, enabled)) {
				lighting.shouldRender = true;
			}
			if (sliderHelper("Rays Per Sample", "Controls amount of rays emitted on each sample", lighting.sampleRaysAmount, 1, 8192, parametersSliderX, parametersSliderY, enabled)) {
				lighting.shouldRender = true;
			}
			if (sliderHelper("Max Bounces", "Controls how many times rays can bounce", lighting.maxBounces, 0, 16, parametersSliderX, parametersSliderY, enabled)) {
				lighting.shouldRender = true;
			}

			if (buttonHelper("Global Illumination", "Enables global illumination", lighting.isDiffuseEnabled, -1.0f, settingsButtonY, enabled, enabled)) {
				lighting.shouldRender = true;
			}
			if (buttonHelper("Specular Reflections", "Enables specular reflections", lighting.isSpecularEnabled, -1.0f, settingsButtonY, enabled, enabled)) {
				lighting.shouldRender = true;
			}
			if (buttonHelper("Refraction", "Enables refraction", lighting.isRefractionEnabled, -1.0f, settingsButtonY, enabled, enabled)) {
				lighting.shouldRender = true;
			}
			if (buttonHelper("Dispersion", "Enables light dispersion with refraction", lighting.isDispersionEnabled, -1.0f, settingsButtonY, enabled, enabled)) {
				lighting.shouldRender = true;
			}
			if (buttonHelper("Emission", "Allows walls to emit light", lighting.isEmissionEnabled, -1.0f, settingsButtonY, enabled, enabled)) {
				lighting.shouldRender = true;
			}

			ImGui::Spacing();
			ImGui::Separator();

			ImGui::TextColored(UpdateVariables::colMenuInformation, "Misc. Settings");

			ImGui::Separator();
			ImGui::Spacing();

			buttonHelper("Symmetrical Lens", "Makes both sides of the next lens editable. Hold LCTRL to move both sides at the same time", lighting.symmetricalLens, -1.0f, settingsButtonY, enabled, enabled);

			buttonHelper("Show Normals", "Displays the direction a wall is pointing at, also know as the normal", lighting.drawNormals, -1.0f, settingsButtonY, enabled, enabled);

			buttonHelper("Relax Shape When Moved", "Relaxes shapes when moving their walls. This is affected too by the relax sliders", lighting.relaxMove, -1.0f, settingsButtonY, enabled, enabled);

		}
	}

	ImGui::EndChild();

	ImGui::End();

	myParam.rightClickSettings.rightClickMenu(myVar, myParam);

	myParam.controls.showControls();
	myParam.controls.showInfo(myVar.fullscreenState);

	ImVec2 statsSize = { 250.0f, myVar.isOpticsEnabled ? 230.0f : 120.0f };

	if (lighting.selectedWalls > 0) {
		statsSize.y += 25.0f;
	}

	if (lighting.selectedLights > 0) {
		statsSize.y += 25.0f;
	}

	float statsPosX = screenX - statsSize.x - buttonsWindowX - 20.0f;

	ImGui::SetNextWindowSize(statsSize, ImGuiCond_Always);
	ImGui::SetNextWindowPos(ImVec2(screenX - statsSize.x - buttonsWindowX - 20.0f, 0.0f), ImGuiCond_Always);

	ImGui::Begin("Stats", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

	ImGui::PushFont(myVar.robotoMediumFont);

	ImGui::SetWindowFontScale(1.5f);

	int particlesAmout = static_cast<int>(myParam.pParticles.size()) + static_cast<int>(myParam.pParticles3D.size());
	int selecParticlesAmout = static_cast<int>(myParam.pParticlesSelected.size()) + static_cast<int>(myParam.pParticlesSelected3D.size());

	ImGui::TextColored(UpdateVariables::colMenuInformation, "%s%d", "Total Particles: ", particlesAmout);

	ImGui::TextColored(UpdateVariables::colMenuInformation, "%s%d", "Selected Particles: ", selecParticlesAmout);

	if (GetFPS() >= 60) {
		ImGui::TextColored(ImVec4(0.0f, 0.8f, 0.0f, 1.0f), "%s%d", "FPS: ", GetFPS());
	}
	else if (GetFPS() < 60 && GetFPS() > 30) {
		ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.0f, 1.0f), "%s%d", "FPS: ", GetFPS());
	}
	else {
		ImGui::TextColored(ImVec4(0.8f, 0.0f, 0.0f, 1.0f), "%s%d", "FPS: ", GetFPS());
	}

	if (myVar.isOpticsEnabled) {

		ImGui::Spacing();
		ImGui::Separator();

		ImGui::TextColored(UpdateVariables::colMenuInformation, "%s%d", "Total Walls: ", static_cast<int>(lighting.walls.size()));

		if (lighting.selectedWalls > 0) {
			ImGui::TextColored(UpdateVariables::colButtonHover, "%s%d", "Selected Walls: ", lighting.selectedWalls);
		}


		ImGui::TextColored(UpdateVariables::colMenuInformation, "%s%d", "Total Lights: ", lighting.totalLights);

		if (lighting.selectedLights > 0) {
			ImGui::TextColored(UpdateVariables::colButtonHover, "%s%d", "Selected Lights: ", lighting.selectedLights);
		}

		ImGui::TextColored(UpdateVariables::colMenuInformation, "%s%d", "Total Rays: ", lighting.accumulatedRays);

		ImGui::Spacing();

		float samplesPorgress = static_cast<float>(lighting.currentSamples) / static_cast<float>(lighting.maxSamples);

		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, UpdateVariables::colButtonHover);
		float progress = samplesPorgress;
		ImVec2 size = ImVec2(ImGui::GetContentRegionAvail().x, 22.0f);
		float radius = 8.0f;

		ImVec2 pos = ImGui::GetCursorScreenPos();
		ImDrawList* draw_list = ImGui::GetWindowDrawList();

		draw_list->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y),
			ImGui::GetColorU32(ImVec4(0.2f, 0.2f, 0.2f, 1.0f)), radius);

		draw_list->AddRectFilled(pos, ImVec2(pos.x + size.x * progress, pos.y + size.y),
			ImGui::GetColorU32(UpdateVariables::colButtonHover), radius);

		char buffer[128];
		snprintf(buffer, sizeof(buffer), "Samples %d / %d", lighting.currentSamples - 1, lighting.maxSamples);

		float fontScale = 0.85f;
		ImFont* font = ImGui::GetFont();
		float fontSize = ImGui::GetFontSize() * fontScale;

		ImVec2 text_size = font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, buffer);
		ImVec2 text_pos = ImVec2(
			pos.x + (size.x - text_size.x) * 0.5f,
			pos.y + (size.y - text_size.y) * 0.5f
		);

		draw_list->AddText(
			font,
			fontSize,
			text_pos,
			ImGui::GetColorU32(ImVec4(1, 1, 1, 1)),
			buffer
		);

		ImGui::Dummy(size);
		ImGui::PopStyleColor();
	}

	ImGui::PopFont();

	ImGui::End();

	// Tools Menu //

	ImVec2 toolsSize = { 250.0f, 370.0f };
	ImGui::SetNextWindowSize(toolsSize, ImGuiCond_Once);
	ImGui::SetNextWindowPos(ImVec2(parametersWindowSizeX + 20.0f, 0.0f), ImGuiCond_Once);
	ImGui::Begin("Tools", nullptr);
	ImGui::BeginTabBar("##ToolsBar", ImGuiTabBarFlags_NoTabListScrollingButtons);

	struct ToolButton {
		const char* label;
		const char* tooltip;
		bool* flag;
	};

	auto activateExclusiveTool = [](ToolButton* group, int count, int activeIndex) {
		for (int i = 0; i < count; ++i) {
			*group[i].flag = (i == activeIndex);
		}
		};

	// Particle tab
	if (ImGui::BeginTabItem("Particle")) {

		ToolButton particleTools[] = {
			{ "Draw Particles", "Draw particles with the brush", &myVar.toolDrawParticles },
			{ "Black Hole", "Throw a black hole particle", &myVar.toolSpawnHeavyParticle },
			{ "Galaxy", "Spawn a large galaxy", &myVar.toolSpawnGalaxy },
			{ "Star", "Spawn a small star. This is not meant for fluid mode", &myVar.toolSpawnStar },
			{ "Big Bang", "Spawn the Big Bang", &myVar.toolSpawnBigBang }
		};

		for (int i = 0; i < IM_ARRAYSIZE(particleTools); ++i) {
			if (buttonHelper(particleTools[i].label, particleTools[i].tooltip, *particleTools[i].flag, -1.0f, settingsButtonY, enabled, enabled)) {
				activateExclusiveTool(particleTools, IM_ARRAYSIZE(particleTools), i);

				myVar.toolErase = false;
				myVar.toolRadialForce = false;
				myVar.toolSpin = false;
				myVar.toolMove = false;
				myVar.toolRaiseTemp = false;
				myVar.toolLowerTemp = false;

				myVar.toolPointLight = false;
				myVar.toolAreaLight = false;
				myVar.toolConeLight = false;
				myVar.toolCircle = false;
				myVar.toolDrawShape = false;
				myVar.toolLens = false;
				myVar.toolWall = false;
				myVar.toolMoveOptics = false;
				myVar.toolEraseOptics = false;
				myVar.toolSelectOptics = false;

				myVar.longExposureFlag = false;
			}
		}

		ImGui::EndTabItem();

		ImGui::Spacing();
		ImGui::Separator();

		ImGui::TextColored(UpdateVariables::colMenuInformation, "ParticleAmount");

		ImGui::Separator();
		ImGui::Spacing();

		sliderHelper("Visible P. Amount Multiplier", "Controls the spawn amount of visible particles", myVar.particleAmountMultiplier, 0.1f, 100.0f, parametersSliderX, parametersSliderY, enabled, LogSlider);
		sliderHelper("DM P. Amount Multiplier", "Controls the spawn amount of dark matter particles", myVar.DMAmountMultiplier, 0.1f, 100.0f, parametersSliderX, parametersSliderY, enabled, LogSlider);

		bool isSPHDisabled = !myVar.isSPHEnabled;

		sliderHelper("Random Mass multiplier", "Controls how much mass can vary for each particle", myVar.massScatter, 0.0f, 1.0f, parametersSliderX, parametersSliderY, isSPHDisabled);
		buttonHelper("Mass Multiplier", "Decides if particles' masses should be inversely multiplied by the amount of particles multiplier", myParam.particlesSpawning.massMultiplierEnabled, 240.0f, 30.0f, true, isSPHDisabled);

		if (myVar.toolSpawnGalaxy) {
			if(!myVar.is3DMode){

				ImGui::Spacing();
				ImGui::Separator();

				ImGui::TextColored(UpdateVariables::colMenuInformation, "Disk");

				ImGui::Separator();
				ImGui::Spacing();

				sliderHelper("Galaxy Outer Radius", "Controls the outer limit of the galaxy", myParam.particlesSpawning.outerRadius, 10.0f, 500.0f, parametersSliderX, parametersSliderY, enabled, LogSlider);
				sliderHelper("Galaxy Radius", "Controls the radius of the galaxy core", myParam.particlesSpawning.scaleLength, 10.0f, 500.0f, parametersSliderX, parametersSliderY, enabled, LogSlider);

				ImGui::Spacing();
				ImGui::Separator();

				ImGui::TextColored(UpdateVariables::colMenuInformation, "Dark Matter");

				ImGui::Separator();
				ImGui::Spacing();

				sliderHelper("DM Halo Size", "Controls the size of the galaxy dark matter halo", myParam.particlesSpawning.outerRadiusDM, 500.0f, 12000.0f, parametersSliderX, parametersSliderY, enabled, LogSlider);
				sliderHelper("DM Halo Core Size", "Controls the size of the galaxy dark matter halo core", myParam.particlesSpawning.radiusCoreDM, 0.5f, 15.0f, parametersSliderX, parametersSliderY, enabled, LogSlider);
			}
			else {
				ImGui::Spacing();
				ImGui::Separator();

				ImGui::TextColored(UpdateVariables::colMenuInformation, "Rotation");

				ImGui::Separator();
				ImGui::Spacing();
				
				sliderHelper("Disk Rotation X", "Controls rotation of disk in the X axist", myParam.particlesSpawning3D.diskAxisX, 0.0f, 180.0f, parametersSliderX, parametersSliderY, enabled);
				sliderHelper("Disk Rotation Y", "Controls rotation of disk in the Y axist", myParam.particlesSpawning3D.diskAxisY, 0.0f, 180.0f, parametersSliderX, parametersSliderY, enabled);
				
				ImGui::Spacing();
				ImGui::Separator();

				ImGui::TextColored(UpdateVariables::colMenuInformation, "Disk");

				ImGui::Separator();
				ImGui::Spacing();
				
				sliderHelper("Galaxy Outer Radius", "Controls the outer limit of the galaxy", myParam.particlesSpawning3D.outerRadius, 10.0f, 500.0f, parametersSliderX, parametersSliderY, enabled, LogSlider);
				sliderHelper("Galaxy Core Radius", "Controls the radius of the galaxy core", myParam.particlesSpawning3D.radiusCore, 0.1f, 700.0f, parametersSliderX, parametersSliderY, enabled, LogSlider);
				sliderHelper("Galaxy Thickness", "Controls the thickness of the galaxy", myParam.particlesSpawning3D.diskThickness, 0.05f, 12.0f, parametersSliderX, parametersSliderY, enabled, LogSlider);
				
				ImGui::Spacing();
				ImGui::Separator();

				ImGui::TextColored(UpdateVariables::colMenuInformation, "Bulge");

				ImGui::Separator();
				ImGui::Spacing();

				sliderHelper("Galaxy Bulge Size", "Controls the size of the galaxy central bulge", myParam.particlesSpawning3D.bulgeSize, 10.0f, 4000.0f, parametersSliderX, parametersSliderY, enabled, LogSlider);
				sliderHelper("Galaxy Bulge Thickness", "Controls the thickness of the galaxy central bulge", myParam.particlesSpawning3D.bulgeThickness, 0.5f, 15.0f, parametersSliderX, parametersSliderY, enabled, LogSlider);
				
				ImGui::Spacing();
				ImGui::Separator();

				ImGui::TextColored(UpdateVariables::colMenuInformation, "Dark Matter");

				ImGui::Separator();
				ImGui::Spacing();

				sliderHelper("DM Halo Size", "Controls the size of the galaxy dark matter halo", myParam.particlesSpawning3D.outerRadiusDM, 500.0f, 12000.0f, parametersSliderX, parametersSliderY, enabled, LogSlider);
				sliderHelper("DM Halo Core Size", "Controls the size of the galaxy dark matter halo core", myParam.particlesSpawning3D.radiusCoreDM, 0.5f, 15.0f, parametersSliderX, parametersSliderY, enabled, LogSlider);
			}
		}

		if (myVar.toolSpawnHeavyParticle) {

			ImGui::Spacing();
			ImGui::Separator();

			ImGui::TextColored(UpdateVariables::colMenuInformation, "Black Hole");

			ImGui::Separator();
			ImGui::Spacing();

			sliderHelper("Black Hole Init Mass", "Controls the mass of black holes when spawned", myVar.heavyParticleWeightMultiplier, 0.005f, 15.0f, parametersSliderX, parametersSliderY, enabled, LogSlider);
		}
	}

	// Brush tab
	if (ImGui::BeginTabItem("Brush")) {
		ToolButton brushTools[] = {
			{ "Eraser Brush", "Erase particles with the brush", &myVar.toolErase },
			{ "Gravity Brush", "Push particles away. Hold LCTRL to invert.", &myVar.toolRadialForce },
			{ "Spin Brush", "Spins particles. Hold LCTRL to invert.", &myVar.toolSpin },
			{ "Grab Brush", "Grab particles inside the brush", &myVar.toolMove },
			{ "Heat Brush", "Heats the particles inside the brush", &myVar.toolRaiseTemp },
			{ "Cool Brush", "Cools the particles inside the brush", &myVar.toolLowerTemp }
		};

		for (int i = 0; i < IM_ARRAYSIZE(brushTools); ++i) {
			if (buttonHelper(brushTools[i].label, brushTools[i].tooltip, *brushTools[i].flag, -1.0f, settingsButtonY, enabled, enabled)) {
				activateExclusiveTool(brushTools, IM_ARRAYSIZE(brushTools), i);

				myVar.toolDrawParticles = false;
				myVar.toolSpawnHeavyParticle = false;
				myVar.toolSpawnGalaxy = false;
				myVar.toolSpawnStar = false;
				myVar.toolSpawnBigBang = false;

				myVar.toolPointLight = false;
				myVar.toolAreaLight = false;
				myVar.toolConeLight = false;
				myVar.toolCircle = false;
				myVar.toolDrawShape = false;
				myVar.toolLens = false;
				myVar.toolWall = false;
				myVar.toolMoveOptics = false;
				myVar.toolEraseOptics = false;
				myVar.toolSelectOptics = false;

				myVar.longExposureFlag = false;
			}
		}

		ImGui::EndTabItem();

		sliderHelper("Gravity Brush Force", "Controls the force of the gravity brush", myVar.brushAttractForceMult, 0.01f, 10.0f, parametersSliderX, parametersSliderY, enabled, LogSlider);
		sliderHelper("Spin Brush Force", "Controls the force of the spin brush", myVar.brushSpinForceMult, 0.01f, 10.0f, parametersSliderX, parametersSliderY, enabled, LogSlider);
	}

	// Optics tab
	if (ImGui::BeginTabItem("Optics")) {
		ToolButton opticTools[] = {
			{ "Point Light", "Spawn point light", &myVar.toolPointLight },
			{ "Area Light", "Spawn area light", &myVar.toolAreaLight },
			{ "Cone Light", "Spawn cone light", &myVar.toolConeLight },
			{ "Wall", "Spawn a wall", &myVar.toolWall },
			{ "Circle", "Spawn a circle", &myVar.toolCircle },
			{ "Draw Shape", "Draw a shape", &myVar.toolDrawShape },
			{ "Lens", "Spawn a lens", &myVar.toolLens },
			{ "Move", "Move optics elements inside the brush", &myVar.toolMoveOptics },
			{ "Erase", "Erase optics elements like walls and lights", &myVar.toolEraseOptics},
			{ "Select", "Select optics elements like walls and lights to modify them. LCTRL adds to selection. LALT removes from selection. LSHIFT selects entire shapes.", &myVar.toolSelectOptics}
		};

		for (int i = 0; i < IM_ARRAYSIZE(opticTools); ++i) {
			if (buttonHelper(opticTools[i].label, opticTools[i].tooltip, *opticTools[i].flag, -1.0f, settingsButtonY, enabled, myVar.isOpticsEnabled)) {
				activateExclusiveTool(opticTools, IM_ARRAYSIZE(opticTools), i);

				myVar.toolDrawParticles = false;
				myVar.toolSpawnHeavyParticle = false;
				myVar.toolSpawnGalaxy = false;
				myVar.toolSpawnStar = false;
				myVar.toolSpawnBigBang = false;

				myVar.toolErase = false;
				myVar.toolRadialForce = false;
				myVar.toolSpin = false;
				myVar.toolMove = false;
				myVar.toolRaiseTemp = false;
				myVar.toolLowerTemp = false;

				myVar.longExposureFlag = false;
			}
		}

		ImGui::EndTabItem();
	}

	// Fun tools tab
	if (ImGui::BeginTabItem("Fun")) {

		//ToolButton funTools[] = {
		//	
		//};

		//for (int i = 0; i < IM_ARRAYSIZE(funTools); ++i) {
		//	if (buttonHelper(funTools[i].label, funTools[i].tooltip, *funTools[i].flag, -1.0f, settingsButtonY, enabled, enabled)) {
		//		//activateExclusiveTool(funTools, IM_ARRAYSIZE(funTools), i);

		//		myVar.toolDrawParticles = false;
		//		myVar.toolSpawnHeavyParticle = false;
		//		myVar.toolSpawnGalaxy = false;
		//		myVar.toolSpawnStar = false;
		//		myVar.toolSpawnBigBang = false;

		//		myVar.toolErase = false;
		//		myVar.toolRadialForce = false;
		//		myVar.toolSpin = false;
		//		myVar.toolMove = false;
		//		myVar.toolRaiseTemp = false;
		//		myVar.toolLowerTemp = false;

		//		myVar.toolPointLight = false;
		//		myVar.toolAreaLight = false;
		//		myVar.toolConeLight = false;
		//		myVar.toolCircle = false;
		//		myVar.toolDrawShape = false;
		//		myVar.toolLens = false;
		//		myVar.toolWall = false;
		//		myVar.toolMoveOptics = false;
		//		myVar.toolEraseOptics = false;
		//		myVar.toolSelectOptics = false;
		//	}
		//}

		ImGui::Spacing();
		ImGui::Separator();

		ImGui::TextColored(UpdateVariables::colMenuInformation, "Long Exposure");

		ImGui::Separator();
		ImGui::Spacing();

		buttonHelper("Long Exposure Duration", "Controls the duration of the long exposure shot", myVar.longExposureFlag, -1.0f, settingsButtonY, enabled, enabled);
		sliderHelper("Long Exposure Duration", "Controls the duration of the long exposure shot", myVar.longExposureDuration, 2, 1000, parametersSliderX, parametersSliderY, enabled);

		ImGui::Spacing();
		ImGui::Separator();

		ImGui::TextColored(UpdateVariables::colMenuInformation, ".PLY Export");

		ImGui::Separator();
		ImGui::Spacing();

		buttonHelper("Export .ply File", "Exports particles to a .ply file", myVar.exportPlyFlag, -1.0f, settingsButtonY, true, enabled);
		buttonHelper("Export .ply Seq.", "Exports particles to a .ply file each frame, creating a .ply sequence", myVar.exportPlySeqFlag, -1.0f, settingsButtonY, true, enabled);

		if (myVar.plyFrameNumber != 0) {
			ImGui::TextColored(UpdateVariables::colMenuInformation, "%s%d", "Frames Exported: ", myVar.plyFrameNumber);
		}

		ImGui::Spacing();
		ImGui::Separator();

		ImGui::TextColored(UpdateVariables::colMenuInformation, "Spaceship");

		ImGui::Separator();
		ImGui::Spacing();

		buttonHelper("Enable Spaceship", "Enables controlling particles", ship.isShipEnabled, -1.0f, settingsButtonY, true, enabled);
		buttonHelper("Ship Gas", "Enables gas particles coming from the ship when controlling particles", myVar.isShipGasEnabled, -1.0f, settingsButtonY, true, enabled);
		sliderHelper("Spaceship Acceleration", "Controls the acceleration of the spaceship when controlling particles",ship.acceleration, 1.0f, 16.0f, parametersSliderX, parametersSliderY, enabled, LogSlider);

		ImGui::EndTabItem();
	}

	ImGui::EndTabBar();
	ImGui::End();

	myVar.loadDropDownMenus = false;
}

void UI::statsWindowLogic(UpdateParameters& myParam, UpdateVariables& myVar) {

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	//------ Performance ------//

	ImGui::TextColored(UpdateVariables::colMenuInformation, "Performance");
	ImGui::Spacing();

	float enablePausedPlot = 1.0f;

	plotLinesHelper(enablePausedPlot, "Framerate: ", graphHistoryLimit, ImGui::GetIO().Framerate, 0.0f, 144.0f, { 340.0f, 200.0f });
	ImGui::Spacing();
	plotLinesHelper(enablePausedPlot, "Frame Time: ", graphHistoryLimit, GetFrameTime(), 0.0f, 1.0f, { 340.0f, 200.0f });

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	//------ Particle Count ------//

	ImGui::TextColored(UpdateVariables::colMenuInformation, "Particle Count");
	ImGui::Spacing();

	int particlesAmout = static_cast<int>(myParam.pParticles.size()) + static_cast<int>(myParam.pParticles3D.size());
	int selecParticlesAmout = static_cast<int>(myParam.pParticlesSelected.size()) + static_cast<int>(myParam.pParticlesSelected3D.size());

	ImGui::Text("%s%d", "Total Particles: ", particlesAmout);

	ImGui::Text("%s%d", "Selected Particles: ", selecParticlesAmout);

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	//------ Composition ------//

	ImGui::TextColored(UpdateVariables::colMenuInformation, "Composition");
	ImGui::Spacing();

	float waterAmount = 0.0f;
	float rockAmount = 0.0f;
	float ironAmount = 0.0f;
	float sandAmount = 0.0f;
	float soilAmount = 0.0f;
	float mudAmount = 0.0f;
	float rubberAmount = 0.0f;

	// This is not the ideal way to do it, but I'm using this for now because there are not many materials

	if (!myVar.is3DMode) {
		for (size_t i = 0; i < myParam.pParticles.size(); i++) {
			ParticleRendering& r = myParam.rParticles[i];
			if (myParam.pParticlesSelected.size() == 0) {
				if (r.sphLabel == 1) {
					waterAmount++;
				}
				else if (r.sphLabel == 2) {
					rockAmount++;
				}
				else if (r.sphLabel == 3) {
					ironAmount++;
				}
				else if (r.sphLabel == 4) {
					sandAmount++;
				}
				else if (r.sphLabel == 5) {
					soilAmount++;
				}
				else if (r.sphLabel == 6) {
					mudAmount++;
				}
				else if (r.sphLabel == 7) {
					rubberAmount++;
				}
			}
			else {
				if (r.sphLabel == 1 && r.isSelected) {
					waterAmount++;
				}
				else if (r.sphLabel == 2 && r.isSelected) {
					rockAmount++;
				}
				else if (r.sphLabel == 3 && r.isSelected) {
					ironAmount++;
				}
				else if (r.sphLabel == 4 && r.isSelected) {
					sandAmount++;
				}
				else if (r.sphLabel == 5 && r.isSelected) {
					soilAmount++;
				}
				else if (r.sphLabel == 6 && r.isSelected) {
					mudAmount++;
				}
				else if (r.sphLabel == 7 && r.isSelected) {
					rubberAmount++;
				}
			}
		}
	}
	else {
		for (size_t i = 0; i < myParam.pParticles3D.size(); i++) {
			ParticleRendering3D& r = myParam.rParticles3D[i];
			if (myParam.pParticlesSelected3D.size() == 0) {
				if (r.sphLabel == 1) {
					waterAmount++;
				}
				else if (r.sphLabel == 2) {
					rockAmount++;
				}
				else if (r.sphLabel == 3) {
					ironAmount++;
				}
				else if (r.sphLabel == 4) {
					sandAmount++;
				}
				else if (r.sphLabel == 5) {
					soilAmount++;
				}
				else if (r.sphLabel == 6) {
					mudAmount++;
				}
				else if (r.sphLabel == 7) {
					rubberAmount++;
				}
			}
			else {
				if (r.sphLabel == 1 && r.isSelected) {
					waterAmount++;
				}
				else if (r.sphLabel == 2 && r.isSelected) {
					rockAmount++;
				}
				else if (r.sphLabel == 3 && r.isSelected) {
					ironAmount++;
				}
				else if (r.sphLabel == 4 && r.isSelected) {
					sandAmount++;
				}
				else if (r.sphLabel == 5 && r.isSelected) {
					soilAmount++;
				}
				else if (r.sphLabel == 6 && r.isSelected) {
					mudAmount++;
				}
				else if (r.sphLabel == 7 && r.isSelected) {
					rubberAmount++;
				}
			}
		}
	}

	std::vector<const char*> labels;
	std::vector<float> values;

	if (waterAmount > 0) { labels.push_back("Water"); values.push_back(waterAmount); }
	if (rockAmount > 0) { labels.push_back("Rock"); values.push_back(rockAmount); }
	if (ironAmount > 0) { labels.push_back("Iron"); values.push_back(ironAmount); }
	if (sandAmount > 0) { labels.push_back("Sand"); values.push_back(sandAmount); }
	if (soilAmount > 0) { labels.push_back("Soil"); values.push_back(soilAmount); }
	if (mudAmount > 0) { labels.push_back("Mud"); values.push_back(mudAmount); }
	if (rubberAmount > 0) { labels.push_back("Rubber"); values.push_back(rubberAmount); }

	if (!values.empty() && ImPlot::BeginPlot("Material Distribution", ImVec2(300, 300), ImPlotFlags_Equal)) {

		ImPlot::SetupAxes(nullptr, nullptr,
			ImPlotAxisFlags_NoDecorations | ImPlotAxisFlags_Lock,
			ImPlotAxisFlags_NoDecorations | ImPlotAxisFlags_Lock);

		ImPlot::SetupAxesLimits(-1, 1, -1, 1, ImGuiCond_Always);

		ImPlot::PlotPieChart(labels.data(), values.data(), static_cast<int>(values.size()), 0.0, 0.0, 0.8);

		ImPlot::EndPlot();
	}

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	//------ Mass ------//

	ImGui::TextColored(UpdateVariables::colMenuInformation, "Mass");
	ImGui::Spacing();

	double totalMass = 0.0;

	if (!myVar.is3DMode) {
		for (size_t i = 0; i < myParam.pParticles.size(); i++) {
			totalMass += myParam.pParticles[i].mass;
		}
	}
	else {
		for (size_t i = 0; i < myParam.pParticles3D.size(); i++) {
			totalMass += myParam.pParticles3D[i].mass;
		}
	}

	ImGui::Text("Total Mass: %.2f", totalMass);

	double selectedMas = 0.0;

	if (!myVar.is3DMode) {
		for (size_t i = 0; i < myParam.pParticles.size(); i++) {
			if (myParam.rParticles[i].isSelected) {
				selectedMas += myParam.pParticles[i].mass;
			}
		}
	}
	else {
		for (size_t i = 0; i < myParam.pParticles3D.size(); i++) {
			if (myParam.rParticles3D[i].isSelected) {
				selectedMas += myParam.pParticles3D[i].mass;
			}
		}
	}

	ImGui::Text("Selected Mass: %.2f", selectedMas);

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	//------ Velocity ------//

	ImGui::TextColored(UpdateVariables::colMenuInformation, "Selected Velocity");
	ImGui::Spacing();

	glm::vec3 selectedVel = { 0.0f, 0.0f, 0.0f };
	float totalVel = 0.0f;

	if (!myVar.is3DMode) {
		for (size_t i = 0; i < myParam.pParticles.size(); i++) {
			if (myParam.rParticles[i].isSelected) {
				selectedVel += glm::vec3{myParam.pParticles[i].vel, 0.0f};
			}
		}

		if (myParam.pParticlesSelected.size() > 0) {
			selectedVel /= myParam.pParticlesSelected.size();
			totalVel = sqrt(selectedVel.x * selectedVel.x + selectedVel.y * selectedVel.y);
		}
	}
	else {
		for (size_t i = 0; i < myParam.pParticles3D.size(); i++) {
			if (myParam.rParticles3D[i].isSelected) {
				selectedVel += myParam.pParticles3D[i].vel;
			}
		}

		if (myParam.pParticlesSelected3D.size() > 0) {
			selectedVel /= myParam.pParticlesSelected3D.size();
			totalVel = sqrt(selectedVel.x * selectedVel.x + selectedVel.y * selectedVel.y + selectedVel.z * selectedVel.z);
		}
	}

	plotLinesHelper(myVar.timeFactor, "Velocity X: ", graphHistoryLimit, selectedVel.x, -300.0f, 300.0f, graphDefaultSize);
	ImGui::Spacing();
	plotLinesHelper(myVar.timeFactor, "Velocity Y: ", graphHistoryLimit, selectedVel.y, -300.0f, 300.0f, graphDefaultSize);
	ImGui::Spacing();
	if (myVar.is3DMode) {
		plotLinesHelper(myVar.timeFactor, "Velocity Z: ", graphHistoryLimit, selectedVel.z, -300.0f, 300.0f, graphDefaultSize);
		ImGui::Spacing();
	}
	plotLinesHelper(myVar.timeFactor, "Total Velocity: ", graphHistoryLimit, totalVel, -300.0f, 300.0f, graphDefaultSize);

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	//------ Acceleration ------//

	ImGui::TextColored(UpdateVariables::colMenuInformation, "Selected Acceleration");
	ImGui::Spacing();

	glm::vec3 selectedAcc = { 0.0f, 0.0f, 0.0f };
	float totalAcc = 0.0f;

	if (!myVar.is3DMode) {
		for (size_t i = 0; i < myParam.pParticles.size(); i++) {
			if (myParam.rParticles[i].isSelected) {
				selectedAcc += glm::vec3{ myParam.pParticles[i].acc, 0.0f };
			}
		}

		if (myParam.pParticlesSelected.size() > 0) {
			selectedAcc /= myParam.pParticlesSelected.size();
			totalAcc = sqrt(selectedAcc.x * selectedAcc.x + selectedAcc.y * selectedAcc.y);
		}
	}
	else {
		for (size_t i = 0; i < myParam.pParticles3D.size(); i++) {
			if (myParam.rParticles3D[i].isSelected) {
				selectedAcc += myParam.pParticles3D[i].acc;
			}
		}

		if (myParam.pParticlesSelected3D.size() > 0) {
			selectedAcc /= myParam.pParticlesSelected3D.size();
			totalAcc = sqrt(selectedAcc.x * selectedAcc.x + selectedAcc.y * selectedAcc.y + selectedAcc.z * selectedAcc.z);
		}
	}

	plotLinesHelper(myVar.timeFactor, "Acceleration X: ", graphHistoryLimit, selectedAcc.x, -300.0f, 300.0f, graphDefaultSize);
	ImGui::Spacing();
	plotLinesHelper(myVar.timeFactor, "Acceleration Y: ", graphHistoryLimit, selectedAcc.y, -300.0f, 300.0f, graphDefaultSize);
	ImGui::Spacing();
	if (myVar.is3DMode) {
		plotLinesHelper(myVar.timeFactor, "Acceleration Z: ", graphHistoryLimit, selectedAcc.z, -300.0f, 300.0f, graphDefaultSize);
		ImGui::Spacing();
	}
	plotLinesHelper(myVar.timeFactor, "Total Acceleration: ", graphHistoryLimit, totalAcc, -300.0f, 300.0f, graphDefaultSize);

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	//------ Pressure ------//

	ImGui::TextColored(UpdateVariables::colMenuInformation, "Selected Pressure");
	ImGui::Spacing();

	float totalPress = 0.0f;

	if (!myVar.is3DMode) {
		for (size_t i = 0; i < myParam.pParticles.size(); i++) {
			if (myParam.rParticles[i].isSelected) {
				totalPress += myParam.pParticles[i].press;
			}
		}

		if (myParam.pParticlesSelected.size() > 0) {
			totalPress /= myParam.pParticlesSelected.size();
		}
	}
	else {
		for (size_t i = 0; i < myParam.pParticles3D.size(); i++) {
			if (myParam.rParticles3D[i].isSelected) {
				totalPress += myParam.pParticles3D[i].press;
			}
		}

		if (myParam.pParticlesSelected3D.size() > 0) {
			totalPress /= myParam.pParticlesSelected3D.size();
		}
	}

	plotLinesHelper(myVar.timeFactor, "Pressure: ", graphHistoryLimit, totalPress, 0.0f, 100.0f, graphDefaultSize);

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	//------ Temperature ------//

	ImGui::TextColored(UpdateVariables::colMenuInformation, "Selected Temperature");
	ImGui::Spacing();

	float totalTemp = 0.0f;

	if (!myVar.is3DMode) {
		for (size_t i = 0; i < myParam.pParticles.size(); i++) {
			if (myParam.rParticles[i].isSelected) {
				totalTemp += myParam.pParticles[i].temp;
			}
		}

		if (myParam.pParticlesSelected.size() > 0) {
			totalTemp /= myParam.pParticlesSelected.size();
		}
	}
	else {
		for (size_t i = 0; i < myParam.pParticles3D.size(); i++) {
			if (myParam.rParticles3D[i].isSelected) {
				totalTemp += myParam.pParticles3D[i].temp;
			}
		}

		if (myParam.pParticlesSelected3D.size() > 0) {
			totalTemp /= myParam.pParticlesSelected3D.size();
		}
	}

	plotLinesHelper(myVar.timeFactor, "Temperature: ", graphHistoryLimit, totalTemp, 0.0f, 100.0f, graphDefaultSize);
}

std::unordered_map<std::string, PlotData> UI::plotDataMap;

void UI::plotLinesHelper(const float& timeFactor, std::string label,
	const int length,
	float value, const float minValue, const float maxValue, ImVec2 size) {

	auto& plotData = plotDataMap[label];

	if (plotData.values.size() != length) {
		plotData.values.resize(length, 0.0f);
		plotData.offset = 0;
	}

	if (timeFactor > 0.0f) {
		plotData.values[plotData.offset] = value;
		plotData.offset = (plotData.offset + 1) % length;
	}

	std::vector<float> ordered_values(length);
	std::vector<float> ordered_x(length);

	for (int i = 0; i < length; ++i) {
		int idx = (plotData.offset + i) % length;
		ordered_values[i] = plotData.values[idx];
		ordered_x[i] = static_cast<float>(i);
	}



	if (ImPlot::BeginPlot(label.c_str(), size, ImPlotFlags_NoInputs)) {

		ImPlot::SetupAxis(ImAxis_Y1, nullptr, ImPlotAxisFlags_AutoFit);

		ImPlot::PushStyleColor(ImPlotCol_Line, UpdateVariables::colPlotLine);
		ImPlot::PushStyleColor(ImPlotCol_AxisText, UpdateVariables::colAxisText);
		ImPlot::PushStyleColor(ImPlotCol_AxisGrid, UpdateVariables::colAxisGrid);
		ImPlot::PushStyleColor(ImPlotCol_AxisBg, UpdateVariables::colAxisBg);
		ImPlot::PushStyleColor(ImPlotCol_FrameBg, UpdateVariables::colFrameBg);
		ImPlot::PushStyleColor(ImPlotCol_PlotBg, UpdateVariables::colPlotBg);
		ImPlot::PushStyleColor(ImPlotCol_PlotBorder, UpdateVariables::colPlotBorder);
		ImPlot::PushStyleColor(ImPlotCol_LegendBg, UpdateVariables::colLegendBg);

		ImPlot::PlotLine(label.c_str(), ordered_x.data(), ordered_values.data(), length);

		ImPlot::PopStyleColor(8);

		ImPlot::EndPlot();
	}
}

static bool wasHovered = false;

bool UI::buttonHelper(std::string label, std::string tooltip, bool& parameter,
	float sizeX, float sizeY, bool canSelfDeactivate, bool& isEnabled) {

	ImGuiID buttonId = ImGui::GetID(label.c_str());
	static std::unordered_map<ImGuiID, bool> hoverStates;

	if (!isEnabled) {
		ImGui::BeginDisabled();
	}

	bool pushedColor = false;
	if (parameter) {
		ImGui::PushStyleColor(ImGuiCol_Button, UpdateVariables::colButtonActive);
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, UpdateVariables::colButtonActiveHover);
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, UpdateVariables::colButtonActivePress);
		pushedColor = true;
	}

	bool hasBeenPressed = false;
	ImVec2 buttonSize;

	if (sizeX > 0.0f && sizeY > 0.0f) {
		buttonSize = ImVec2(sizeX, sizeY);
	}
	else if (sizeX < 0.0f && sizeY > 0.0f) {
		buttonSize = ImVec2(ImGui::GetContentRegionAvail().x, sizeY);
	}
	else if (sizeX > 0.0f && sizeY < 0.0f) {
		buttonSize = ImVec2(sizeX, ImGui::GetContentRegionAvail().y);
	}
	else {
		buttonSize = ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y);
	}

	std::vector<Sound>* soundPool = nullptr;

	if (ImGui::Button(label.c_str(), buttonSize)) {

		if (!parameter) {

			soundPool = &GESound::soundButtonEnablePool;

			if (soundPool && !soundPool->empty()) {
				bool played = false;

				for (Sound& sound : *soundPool) {
					if (!IsSoundPlaying(sound)) {
						PlaySound(sound);
						played = true;
						break;
					}
				}

				if (!played) {
					PlaySound(soundPool->back());
				}
			}
		}
		else {
			soundPool = &GESound::soundButtonDisablePool;

			if (soundPool && !soundPool->empty()) {
				bool played = false;

				for (Sound& sound : *soundPool) {
					if (!IsSoundPlaying(sound)) {
						PlaySound(sound);
						played = true;
						break;
					}
				}

				if (!played) {
					PlaySound(soundPool->back());
				}
			}
		}

		if (canSelfDeactivate) {
			parameter = !parameter;
		}
		else if (!parameter) {
			parameter = true;
		}
		hasBeenPressed = true;
	}

	if (pushedColor) {
		ImGui::PopStyleColor(3);
	}

	bool isHovered = ImGui::IsItemHovered();

	if (isHovered) {
		ImGui::SetTooltip("%s", tooltip.c_str());

		int randSoundNum = rand() % 3;

		if (!hoverStates[buttonId]) {
			std::vector<Sound>* soundPool = nullptr;

			switch (randSoundNum) {
			case 0:
				soundPool = &GESound::soundButtonHover1Pool;
				break;
			case 1:
				soundPool = &GESound::soundButtonHover2Pool;
				break;
			case 2:
				soundPool = &GESound::soundButtonHover3Pool;
				break;
			}

			if (soundPool && !soundPool->empty()) {
				bool played = false;

				for (Sound& sound : *soundPool) {
					if (!IsSoundPlaying(sound)) {
						PlaySound(sound);
						played = true;
						break;
					}
				}

				if (!played) {
					PlaySound(soundPool->back());
				}
			}
		}
	}

	hoverStates[buttonId] = isHovered;

	if (!isEnabled) {
		ImGui::EndDisabled();
	}

	return hasBeenPressed;
}



bool UI::sliderHelper(std::string label, std::string tooltip, float& parameter, float minVal, float maxVal,
	float sizeX, float sizeY, bool& isEnabled, int logarithmic) {

	bool isSliderUsed = false;

	ImGuiID sliderId = ImGui::GetID(label.c_str());
	static std::unordered_map<ImGuiID, bool> hoverStates;
	static std::unordered_map<ImGuiID, float> defaultValues;

	if (!isEnabled) {
		ImGui::BeginDisabled();
	}

	if (defaultValues.find(sliderId) == defaultValues.end()) {
		defaultValues[sliderId] = parameter;
	}

	ImVec2 sliderSize;

	if (sizeX > 0.0f && sizeY > 0.0f) {
		sliderSize = ImVec2(sizeX, sizeY);
	}
	else if (sizeX < 0.0f && sizeY > 0.0f) {
		sliderSize = ImVec2(ImGui::GetContentRegionAvail().x, sizeY);
	}
	else if (sizeX > 0.0f && sizeY < 0.0f) {
		sliderSize = ImVec2(sizeX, ImGui::GetContentRegionAvail().y);
	}
	else {
		sliderSize = ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y);
	}

	ImGui::Text("%s", label.c_str());

	if (ImGui::SliderFloat(("##" + label).c_str(), &parameter, minVal, maxVal, "%.3f", ImGuiSliderFlags_Logarithmic)) {
		isSliderUsed = true;
	}

	std::vector<Sound>* soundPool = nullptr;

	static bool hasBeenPressed = false;

	if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {

		hasBeenPressed = true;

		soundPool = &GESound::soundButtonEnablePool;

		if (soundPool && !soundPool->empty()) {
			bool played = false;

			for (Sound& sound : *soundPool) {
				if (!IsSoundPlaying(sound)) {
					PlaySound(sound);
					played = true;
					break;
				}
			}

			if (!played) {
				PlaySound(soundPool->back());
			}
		}
	}

	if (hasBeenPressed && ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {

		soundPool = &GESound::soundButtonDisablePool;

		if (soundPool && !soundPool->empty()) {
			bool played = false;

			for (Sound& sound : *soundPool) {
				if (!IsSoundPlaying(sound)) {
					PlaySound(sound);
					played = true;
					break;
				}
			}

			if (!played) {
				PlaySound(soundPool->back());
			}
		}

		hasBeenPressed = false;
	}

	static float prevValue = parameter;
	static bool wasPlaying = false;
	static ImVec2 lastMousePos = ImGui::GetMousePos();

	if (ImGui::IsItemActive()) {
		ImVec2 currentMousePos = ImGui::GetMousePos();
		float mouseDelta = abs(currentMousePos.x - lastMousePos.x) + abs(currentMousePos.y - lastMousePos.y);

		if (mouseDelta > 2.0f) {
			if (!wasPlaying || parameter != prevValue) {
				soundPool = &GESound::soundSliderSlidePool;
				if (soundPool && !soundPool->empty()) {
					bool played = false;
					for (Sound& sound : *soundPool) {
						if (!IsSoundPlaying(sound)) {
							PlaySound(sound);
							played = true;
							wasPlaying = true;
							lastMousePos = currentMousePos;
							break;
						}
					}
					if (!played) {
						PlaySound(soundPool->back());
						wasPlaying = true;
						lastMousePos = currentMousePos;
					}
				}
			}
		}
		prevValue = parameter;
	}
	else {
		wasPlaying = false;
	}

	bool isHovered = ImGui::IsItemHovered();

	if (isHovered) {
		ImGui::SetTooltip("%s", tooltip.c_str());

		int randSoundNum = rand() % 3;

		if (!hoverStates[sliderId]) {
			std::vector<Sound>* soundPool = nullptr;

			switch (randSoundNum) {
			case 0:
				soundPool = &GESound::soundButtonHover1Pool;
				break;
			case 1:
				soundPool = &GESound::soundButtonHover2Pool;
				break;
			case 2:
				soundPool = &GESound::soundButtonHover3Pool;
				break;
			}

			if (soundPool && !soundPool->empty()) {
				bool played = false;

				for (Sound& sound : *soundPool) {
					if (!IsSoundPlaying(sound)) {
						PlaySound(sound);
						played = true;
						break;
					}
				}

				if (!played) {
					PlaySound(soundPool->back());
				}
			}
		}
	}

	hoverStates[sliderId] = isHovered;

	if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
		parameter = defaultValues[sliderId];

		isSliderUsed = true;
	}

	if (!isEnabled) {
		ImGui::EndDisabled();
	}

	return isSliderUsed;
}

bool UI::sliderHelper(std::string label, std::string tooltip, float& parameter, float minVal, float maxVal,
	float sizeX, float sizeY, bool& isEnabled) {

	bool isSliderUsed = false;

	ImGuiID sliderId = ImGui::GetID(label.c_str());
	static std::unordered_map<ImGuiID, bool> hoverStates;
	static std::unordered_map<ImGuiID, float> defaultValues;

	if (!isEnabled) {
		ImGui::BeginDisabled();
	}

	if (defaultValues.find(sliderId) == defaultValues.end()) {
		defaultValues[sliderId] = parameter;
	}

	ImVec2 sliderSize;

	if (sizeX > 0.0f && sizeY > 0.0f) {
		sliderSize = ImVec2(sizeX, sizeY);
	}
	else if (sizeX < 0.0f && sizeY > 0.0f) {
		sliderSize = ImVec2(ImGui::GetContentRegionAvail().x, sizeY);
	}
	else if (sizeX > 0.0f && sizeY < 0.0f) {
		sliderSize = ImVec2(sizeX, ImGui::GetContentRegionAvail().y);
	}
	else {
		sliderSize = ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y);
	}

	ImGui::Text("%s", label.c_str());

	if (ImGui::SliderFloat(("##" + label).c_str(), &parameter, minVal, maxVal, "%.3f")) {
		isSliderUsed = true;
	}

	std::vector<Sound>* soundPool = nullptr;

	static bool hasBeenPressed = false;

	if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {

		hasBeenPressed = true;

		soundPool = &GESound::soundButtonEnablePool;

		if (soundPool && !soundPool->empty()) {
			bool played = false;

			for (Sound& sound : *soundPool) {
				if (!IsSoundPlaying(sound)) {
					PlaySound(sound);
					played = true;
					break;
				}
			}

			if (!played) {
				PlaySound(soundPool->back());
			}
		}
	}

	if (hasBeenPressed && ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {

		soundPool = &GESound::soundButtonDisablePool;

		if (soundPool && !soundPool->empty()) {
			bool played = false;

			for (Sound& sound : *soundPool) {
				if (!IsSoundPlaying(sound)) {
					PlaySound(sound);
					played = true;
					break;
				}
			}

			if (!played) {
				PlaySound(soundPool->back());
			}
		}

		hasBeenPressed = false;
	}

	static float prevValue = parameter;
	static bool wasPlaying = false;
	static ImVec2 lastMousePos = ImGui::GetMousePos();

	if (ImGui::IsItemActive()) {
		ImVec2 currentMousePos = ImGui::GetMousePos();
		float mouseDelta = abs(currentMousePos.x - lastMousePos.x) + abs(currentMousePos.y - lastMousePos.y);

		if (mouseDelta > 2.0f) {
			if (!wasPlaying || parameter != prevValue) {
				soundPool = &GESound::soundSliderSlidePool;
				if (soundPool && !soundPool->empty()) {
					bool played = false;
					for (Sound& sound : *soundPool) {
						if (!IsSoundPlaying(sound)) {
							PlaySound(sound);
							played = true;
							wasPlaying = true;
							lastMousePos = currentMousePos;
							break;
						}
					}
					if (!played) {
						PlaySound(soundPool->back());
						wasPlaying = true;
						lastMousePos = currentMousePos;
					}
				}
			}
		}
		prevValue = parameter;
	}
	else {
		wasPlaying = false;
	}

	bool isHovered = ImGui::IsItemHovered();

	if (isHovered) {
		ImGui::SetTooltip("%s", tooltip.c_str());

		int randSoundNum = rand() % 3;

		if (!hoverStates[sliderId]) {
			std::vector<Sound>* soundPool = nullptr;

			switch (randSoundNum) {
			case 0:
				soundPool = &GESound::soundButtonHover1Pool;
				break;
			case 1:
				soundPool = &GESound::soundButtonHover2Pool;
				break;
			case 2:
				soundPool = &GESound::soundButtonHover3Pool;
				break;
			}

			if (soundPool && !soundPool->empty()) {
				bool played = false;

				for (Sound& sound : *soundPool) {
					if (!IsSoundPlaying(sound)) {
						PlaySound(sound);
						played = true;
						break;
					}
				}

				if (!played) {
					PlaySound(soundPool->back());
				}
			}
		}
	}

	hoverStates[sliderId] = isHovered;

	if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
		parameter = defaultValues[sliderId];

		isSliderUsed = true;
	}

	if (!isEnabled) {
		ImGui::EndDisabled();
	}

	return isSliderUsed;
}

bool UI::sliderHelper(std::string label, std::string tooltip, int& parameter, int minVal, int maxVal,
	float sizeX, float sizeY, bool& isEnabled) {

	bool isSliderUsed = false;

	ImGuiID sliderId = ImGui::GetID(label.c_str());
	static std::unordered_map<ImGuiID, bool> hoverStates;
	static std::unordered_map<ImGuiID, float> defaultValues;

	if (!isEnabled) {
		ImGui::BeginDisabled();
	}

	if (defaultValues.find(sliderId) == defaultValues.end()) {
		defaultValues[sliderId] = parameter;
	}

	ImVec2 sliderSize;

	if (sizeX > 0.0f && sizeY > 0.0f) {
		sliderSize = ImVec2(sizeX, sizeY);
	}
	else if (sizeX < 0.0f && sizeY > 0.0f) {
		sliderSize = ImVec2(ImGui::GetContentRegionAvail().x, sizeY);
	}
	else if (sizeX > 0.0f && sizeY < 0.0f) {
		sliderSize = ImVec2(sizeX, ImGui::GetContentRegionAvail().y);
	}
	else {
		sliderSize = ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y);
	}

	ImGui::Text("%s", label.c_str());

	if (ImGui::SliderInt(("##" + label).c_str(), &parameter, minVal, maxVal)) {
		isSliderUsed = true;
	}

	std::vector<Sound>* soundPool = nullptr;

	static bool hasBeenPressed = false;

	if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {

		hasBeenPressed = true;

		soundPool = &GESound::soundButtonEnablePool;

		if (soundPool && !soundPool->empty()) {
			bool played = false;

			for (Sound& sound : *soundPool) {
				if (!IsSoundPlaying(sound)) {
					PlaySound(sound);
					played = true;
					break;
				}
			}

			if (!played) {
				PlaySound(soundPool->back());
			}
		}
	}

	if (hasBeenPressed && ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {

		soundPool = &GESound::soundButtonDisablePool;

		if (soundPool && !soundPool->empty()) {
			bool played = false;

			for (Sound& sound : *soundPool) {
				if (!IsSoundPlaying(sound)) {
					PlaySound(sound);
					played = true;
					break;
				}
			}

			if (!played) {
				PlaySound(soundPool->back());
			}
		}

		hasBeenPressed = false;
	}

	static int prevValue = parameter;
	static bool wasPlaying = false;
	static ImVec2 lastMousePos = ImGui::GetMousePos();

	if (ImGui::IsItemActive()) {
		ImVec2 currentMousePos = ImGui::GetMousePos();
		int mouseDelta = abs(currentMousePos.x - lastMousePos.x) + abs(currentMousePos.y - lastMousePos.y);

		if (mouseDelta > 2.0f) {
			if (!wasPlaying || parameter != prevValue) {
				soundPool = &GESound::soundSliderSlidePool;
				if (soundPool && !soundPool->empty()) {
					bool played = false;
					for (Sound& sound : *soundPool) {
						if (!IsSoundPlaying(sound)) {
							PlaySound(sound);
							played = true;
							wasPlaying = true;
							lastMousePos = currentMousePos;
							break;
						}
					}
					if (!played) {
						PlaySound(soundPool->back());
						wasPlaying = true;
						lastMousePos = currentMousePos;
					}
				}
			}
		}
		prevValue = parameter;
	}
	else {
		wasPlaying = false;
	}

	bool isHovered = ImGui::IsItemHovered();

	if (isHovered) {
		ImGui::SetTooltip("%s", tooltip.c_str());

		int randSoundNum = rand() % 3;

		if (!hoverStates[sliderId]) {
			std::vector<Sound>* soundPool = nullptr;

			switch (randSoundNum) {
			case 0:
				soundPool = &GESound::soundButtonHover1Pool;
				break;
			case 1:
				soundPool = &GESound::soundButtonHover2Pool;
				break;
			case 2:
				soundPool = &GESound::soundButtonHover3Pool;
				break;
			}

			if (soundPool && !soundPool->empty()) {
				bool played = false;

				for (Sound& sound : *soundPool) {
					if (!IsSoundPlaying(sound)) {
						PlaySound(sound);
						played = true;
						break;
					}
				}

				if (!played) {
					PlaySound(soundPool->back());
				}
			}
		}
	}

	hoverStates[sliderId] = isHovered;

	static int defaultVal = parameter;

	if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
		parameter = defaultValues[sliderId];

		isSliderUsed = true;
	}

	if (!isEnabled) {
		ImGui::EndDisabled();
	}

	return isSliderUsed;
}
