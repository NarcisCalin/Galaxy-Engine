#include "UI/UI.h"

void UI::uiLogic(UpdateParameters& myParam, UpdateVariables& myVar, SPH& sph, SaveSystem& save, GESound& geSound, Lighting& lighting) {


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

	std::vector<SimilarTypeButton::Mode> color{
	{ "Solid Color",     "Particles will only use the primary color",           &myParam.colorVisuals.solidColor },
	{ "Density Color",   "Maps particle neighbor amount to primary and secondary colors",            &myParam.colorVisuals.densityColor },
	{ "Force Color",     "Maps particle acceleration to primary and secondary colors",               &myParam.colorVisuals.forceColor },
	{ "Velocity Color",  "Maps particle velocity to color",                    &myParam.colorVisuals.velocityColor },
	{ "Shockwave Color", "Maps particle acceleration to color",                &myParam.colorVisuals.shockwaveColor },
	{ "Pressure Color",  "Maps particle pressure to color",                    &myParam.colorVisuals.pressureColor },
	{ "Temperature Color",  "Maps particle temperature to color",                    &myParam.colorVisuals.temperatureColor },
	{ "Temperature Gas Color",  "Maps particle temperature to primary and secondary colors",                    &myParam.colorVisuals.gasTempColor },
	{ "Material Color",       "Uses materials colors",                      &myParam.colorVisuals.SPHColor }
	};

	std::vector<SimilarTypeButton::Mode> size{
{ "Density Size", "Maps particle neighbor amount to size", &myVar.isDensitySizeEnabled },
{ "Force Size", "Maps particle acceleration to size", &myVar.isForceSizeEnabled }
	};

	buttonHelper("Fullscreen", "Toggles fulscreen", myVar.fullscreenState, -1.0f, settingsButtonY, true, enabled);

	SimilarTypeButton::buttonIterator(controlsAndInfo, -1.0f, settingsButtonY, true, enabled);

	SimilarTypeButton::buttonIterator(trails, -1.0f, settingsButtonY, true, enabled);

	buttonHelper("Local Trails", "Enables trails moving relative to particles average position", myVar.isLocalTrailsEnabled, -1.0f, settingsButtonY, true, enabled);
	buttonHelper("White Trails", "Makes all trails white", myParam.trails.whiteTrails, -1.0f, settingsButtonY, true, enabled);

	SimilarTypeButton::buttonIterator(color, -1.0f, settingsButtonY, false, enabled);

	buttonHelper("Selected Color", "Highlight selected particles", myParam.colorVisuals.selectedColor, -1.0f, settingsButtonY, true, enabled);
	buttonHelper("Dark Matter", "Enables dark matter particles. This works for galaxies and Big Bang", myVar.isDarkMatterEnabled, -1.0f, settingsButtonY, true, enabled);
	buttonHelper("Show Dark Matter", "Unhides dark matter particles", myParam.colorVisuals.showDarkMatterEnabled, -1.0f, settingsButtonY, true, enabled);
	buttonHelper("Looping Space", "Particles disappearing on one side will appear on the other side", myVar.isPeriodicBoundaryEnabled, -1.0f, settingsButtonY, true, enabled);
	buttonHelper("Multi-Threading", "Distributes the simulation across multiple threads", myVar.isMultiThreadingEnabled, -1.0f, settingsButtonY, true, enabled);

	bool wasEnabled = myVar.isSPHEnabled;

	if (buttonHelper("Fluid Mode", "Enables SPH fluids", myVar.isSPHEnabled, -1.0f, settingsButtonY, true, enabled)) {
		if (!wasEnabled && myVar.isSPHEnabled) {
			for (size_t i = 0; i < color.size(); i++) {
				if (color[i].flag == &myParam.colorVisuals.SPHColor) {

					*color[i].flag = true;
				}
				else {
					*color[i].flag = false;
				}
			}
		}
	}

	buttonHelper("Fluid Ground Mode", "Adds vertical gravity and makes particles collide with the domain walls", myVar.sphGround, -1.0f, settingsButtonY, true, myVar.isSPHEnabled);

	buttonHelper("Temperature Simulation", "Enables temperature simulation", myVar.isTempEnabled, -1.0f, settingsButtonY, true, enabled);

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

	SimilarTypeButton::buttonIterator(size, -1.0f, settingsButtonY, true, enabled);

	buttonHelper("Glow", "Enables glow shader", myVar.isGlowEnabled, -1.0f, settingsButtonY, true, enabled);
	buttonHelper("Predict Path", "Predicts the trajectory of heavy particles before launching them", myParam.particlesSpawning.enablePathPrediction, -1.0f, settingsButtonY, true, enabled);
	buttonHelper("Ship Gas", "Enables gas particles coming from the ship when controlling particles", myVar.isShipGasEnabled, -1.0f, settingsButtonY, true, enabled);
	buttonHelper("Save Scene", "Save current scene to disk", save.saveFlag, -1.0f, settingsButtonY, true, enabled);
	buttonHelper("Load Scene", "Load a scene from disk", save.loadFlag, -1.0f, settingsButtonY, true, enabled);

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

		if (ImGui::BeginTabItem("Lighting")) {

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

			sliderHelper("Density Radius", "Controls the neighbor search radius", myParam.neighborSearch.densityRadius, 0.0f, 7.0f, parametersSliderX, parametersSliderY, enabled);
			sliderHelper("Max Neighbors", "Controls the maximum neighbor count range", myParam.colorVisuals.maxNeighbors, 1, 500, parametersSliderX, parametersSliderY, enabled);
			sliderHelper("Max Color Force", "Controls the acceleration threshold to use the secondary color", myParam.colorVisuals.maxColorAcc, 1.0f, 400.0f, parametersSliderX, parametersSliderY, enabled);
			sliderHelper("Max Size Force", "Controls the acceleration threshold to map the particle size", myParam.densitySize.sizeAcc, 1.0f, 400.0f, parametersSliderX, parametersSliderY, enabled);
			sliderHelper("Max Dynamic Size", "Controls the maximum size particles can have when chaning size dynamically", myParam.densitySize.maxSize, 0.1f, 5.0f, parametersSliderX, parametersSliderY, enabled);
			sliderHelper("Min Dynamic Size", "Controls the minimum size particles can have when chaning size dynamically", myParam.densitySize.minSize, 0.001f, 5.0f, parametersSliderX, parametersSliderY, enabled);
			sliderHelper("Max Shockwave Accel", "Controls the acceleration threshold to map the particle color in Shockwave color mode", myParam.colorVisuals.ShockwaveMaxAcc, 1.0f, 120.0f, parametersSliderX, parametersSliderY, enabled);
			sliderHelper("Max Velocity Color", "Controls the max velocity used to map the colors in the velocity color mode", myParam.colorVisuals.maxVel, 10.0f, 10000.0f, parametersSliderX, parametersSliderY, enabled);
			sliderHelper("Max Pressure Color", "Controls the max pressure used to map the colors in the pressure color mode", myParam.colorVisuals.maxPress, 100.0f, 100000.0f, parametersSliderX, parametersSliderY, enabled);
			sliderHelper("Max Temperature Color", "Controls the max temperature used to map the colors in the temperature color mode", myParam.colorVisuals.tempColorMaxTemp, 10.0f, 50000.0f, parametersSliderX, parametersSliderY, enabled);
			sliderHelper("Max Constraint Stress", "Controls the max constraint stress used to map the colors in the constraints stress color mode. If set to 0, it will set the max stress to the material's breaking limit", myVar.constraintMaxStressColor, 0.0f, 1.0f, parametersSliderX, parametersSliderY, enabled);
			sliderHelper("Particles Size", "Controls the size of all particles", myVar.particleSizeMultiplier, 0.1f, 5.0f, parametersSliderX, parametersSliderY, enabled);
			sliderHelper("Trails Length", "Controls how long should the trails be. This feature is computationally expensive", myVar.trailMaxLength, 0, 1500, parametersSliderX, parametersSliderY, enabled);
			sliderHelper("Trails Thickness", "Controls the trails thickness", myParam.trails.trailThickness, 0.007f, 0.5f, parametersSliderX, parametersSliderY, enabled);
			sliderHelper("Path Prediction Length", "Controls how long is the predicted path", myParam.particlesSpawning.predictPathLength, 100, 2000, parametersSliderX, parametersSliderY, enabled);
			sliderHelper("Visible P. Amount Multiplier", "Controls the spawn amount of visible particles", myParam.particlesSpawning.particleAmountMultiplier, 0.1f, 100.0f, parametersSliderX, parametersSliderY, enabled);
			sliderHelper("DM P. Amount Multiplier", "Controls the spawn amount of dark matter particles", myParam.particlesSpawning.DMAmountMultiplier, 0.1f, 100.0f, parametersSliderX, parametersSliderY, enabled);

			bool massMultiplierButtonEnable = true;
			if (myVar.isSPHEnabled) {
				myParam.particlesSpawning.massMultiplierEnabled = false;
				massMultiplierButtonEnable = false;
			}
			buttonHelper("Mass Multiplier", "Decides if particles' masses should be inversely multiplied by the amount of particles multiplier", myParam.particlesSpawning.massMultiplierEnabled, 240.0f, 30.0f, true, massMultiplierButtonEnable);
		}

		if (bPhysicsSliders) {

			sliderHelper("Softening", "Controls the smoothness of the gravity forces", myVar.softening, 1.0f, 30.0f, parametersSliderX, parametersSliderY, enabled);
			sliderHelper("Theta", "Controls the quality of the gravity calculation. Higher means lower quality", myVar.theta, 0.1f, 5.0f, parametersSliderX, parametersSliderY, enabled);
			sliderHelper("Time Scale", "Controls how fast time passes", myVar.timeStepMultiplier, 0.0f, 15.0f, parametersSliderX, parametersSliderY, enabled);
			sliderHelper("Gravity Strength", "Controls how much particles attract eachother", myVar.gravityMultiplier, 0.0f, 100.0f, parametersSliderX, parametersSliderY, enabled);
			sliderHelper("Heavy Particle Init Mass", "Controls the mass of the heavy particles when spawned", myParam.particlesSpawning.heavyParticleWeightMultiplier, 0.005f, 15.0f, parametersSliderX, parametersSliderY, enabled);
			sliderHelper("Domain Width", "Controls the width of the global container", myVar.domainSize.x, 200.0f, 3840.0f, parametersSliderX, parametersSliderY, enabled);
			sliderHelper("Domain Height", "Controls the height of the global container", myVar.domainSize.y, 200.0f, 2160.0f, parametersSliderX, parametersSliderY, enabled);
			sliderHelper("Threads Amount", "Controls the amount of threads used by the simulation. Half your total amount of threads is usually the sweet spot", myVar.threadsAmount, 1, 32, parametersSliderX, parametersSliderY, enabled);

			sliderHelper("Ambient Temperature", "Controls the desired temperature of the scene in Kelvin. 1 is near absolute zero. The default value is set just high enough to allow liquid water", myVar.ambientTemp, 1.0f, 2500.0f, parametersSliderX, parametersSliderY, enabled);
			sliderHelper("Ambient Heat Rate", "Controls how fast particles' temperature try to match ambient temperature", myVar.globalAmbientHeatRate, 0.0f, 10.0f, parametersSliderX, parametersSliderY, enabled);
			sliderHelper("Heat Conductivity Multiplier", "Controls the global heat conductivity of particles", myVar.globalHeatConductivity, 0.001f, 1.0f, parametersSliderX, parametersSliderY, enabled);

			sliderHelper("Constraints Stiffness Multiplier", "Controls the global stiffness multiplier for constraints", myVar.globalConstraintStiffnessMult, 0.001f, 3.0f, parametersSliderX, parametersSliderY, enabled);
			sliderHelper("Constraints Resistance Multiplier", "Controls the global resistance multiplier for constraints", myVar.globalConstraintResistance, 0.001f, 30.0f, parametersSliderX, parametersSliderY, enabled);

			sliderHelper("Fluid Vertical Gravity", "Controls the vertical gravity strength in Fluid Ground Mode", sph.verticalGravity, 0.0f, 10.0f, parametersSliderX, parametersSliderY, enabled);
			sliderHelper("Fluid Mass Multiplier", "Controls the fluid mass of particles", sph.mass, 0.005f, 0.15f, parametersSliderX, parametersSliderY, enabled);
			sliderHelper("Fluid Viscosity", "Controls how viscous particles are", sph.viscosity, 0.01f, 15.0f, parametersSliderX, parametersSliderY, enabled);
			sliderHelper("Fluid Stiffness", "Controls how stiff particles are", sph.stiffMultiplier, 0.01f, 15.0f, parametersSliderX, parametersSliderY, enabled);
			sliderHelper("Fluid Cohesion", "Controls how sticky particles are", sph.cohesionCoefficient, 0.0f, 10.0f, parametersSliderX, parametersSliderY, enabled);
			sliderHelper("Fluid Delta", "Controls the scaling factor in the pressure solver to enforce fluid incompressibility", sph.delta, 500.0f, 20000.0f, parametersSliderX, parametersSliderY, enabled);
			sliderHelper("Fluid Max Velocity", "Controls the maximum velocity a particle can have in Fluid mode", myVar.sphMaxVel, 0.0f, 2000.0f, parametersSliderX, parametersSliderY, enabled);

			std::vector<SimilarTypeButton::Mode> sphMats{
				{ "Water", "Water", &myParam.brush.SPHWater},
				{ "Rock", "Rock", &myParam.brush.SPHRock},
				{ "Iron", "Iron", &myParam.brush.SPHIron},
				{"Sand", "Sand", &myParam.brush.SPHSand},
				{"Soil", "Soil", &myParam.brush.SPHSoil},
				{"Ice", "Ice", &myParam.brush.SPHIce},
				{"Mud", "Mud", &myParam.brush.SPHMud},
				{"Rubber", "Rubber", &myParam.brush.SPHRubber},
				{"Gas", "Gas", &myParam.brush.SPHGas}
			};

			float oldSpacingY = ImGui::GetStyle().ItemSpacing.y;
			ImGui::GetStyle().ItemSpacing.y = 5.0f; // Set the spacing only for the settings buttons

			SimilarTypeButton::buttonIterator(sphMats, -1.0f, settingsButtonY, true, myVar.isSPHEnabled);

			ImGui::GetStyle().ItemSpacing.y = oldSpacingY;

		}

		if (bSoundWindow) {

			bool enabled = true;

			sliderHelper("Global Volume", "Controls global sound volume", geSound.globalVolume, 0.0f, 1.0f, parametersSliderX, parametersSliderY, enabled);
			sliderHelper("Menu Volume", "Controls menu sounds volume", geSound.menuVolume, 0.0f, 1.0f, parametersSliderX, parametersSliderY, enabled);
			sliderHelper("Music Volume", "Controls soundtrack volume", geSound.musicVolume, 0.0f, 1.0f, parametersSliderX, parametersSliderY, enabled);

			if (buttonHelper("<- Previous Track", "Plays the previous track in the playlist", geSound.hasTrackChanged, -1.0f, settingsButtonY, true, enabled)) {
				geSound.currentSongIndex--;
			}

			if (buttonHelper("Next Track ->", "Plays the next track in the playlist", geSound.hasTrackChanged, -1.0f, settingsButtonY, true, enabled)) {
				geSound.currentSongIndex++;
			}
		}

		if (bRecordingSettings) {
			static std::array<settingsParams, 2> recordingButtonsParams = {
				settingsParams("Pause After Recording", "Pauses the simulation after recording is finished", myVar.pauseAfterRecording),
				settingsParams("Clean Scene After Recording", "Clears all particles from the scene after recording is finished", myVar.cleanSceneAfterRecording)
			};


			float oldSpacingY = ImGui::GetStyle().ItemSpacing.y;
			ImGui::GetStyle().ItemSpacing.y = 5.0f; // Set the spacing only for the recording settings buttons

			buttonHelper("Pause After Recording", "Pauses the simulation after recording is finished", myVar.pauseAfterRecording, -1.0f, settingsButtonY, true, enabled);
			buttonHelper("Clean Scene After Recording", "Clears all particles from the scene after recording is finished", myVar.cleanSceneAfterRecording, -1.0f, settingsButtonY, true, enabled);

			ImGui::Separator(); // Add a separator

			bool isEnabled = true;
			if (myVar.isRecording) { // Check if recording is active
				isEnabled = false; // Disable the slider
			}
			sliderHelper("RecordingTimeLimit", "Set a time limit for the recording. 0 means no limit.", myVar.recordingTimeLimit, 0.0f, 60.0f, parametersSliderX, parametersSliderY, isEnabled);


			ImGui::GetStyle().ItemSpacing.y =
				oldSpacingY; // End the recording settings buttons spacing
		}

		if (bStatsWindow) {
			statsWindowLogic(myParam, myVar);
		}

		if (bLightingWindow) {

			bool enabled = true;

			ImGui::Text("Galaxy Engine 1.7.0 - Lighting Beta");
			ImGui::Text("Beta placeholder controls:");
			ImGui::Text("Hold and drag V: Create wall");
			ImGui::Text("Press 5: Create point light");
			ImGui::Text("Hold and drag 6: Create area light");

			ImGui::Text("Be careful with these sliders,");
			ImGui::Text("they can make the program run very slow.");
			

			ImGui::Separator();
			ImGui::Spacing();

			ImVec4 imguiWallColor = rlImGuiColors::Convert(lighting.wallColor);

			Color imguiWallColorRl;

			if (ImGui::ColorPicker4("WallColor", (float*)&imguiWallColor, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_DisplayRGB)) {
				imguiWallColorRl = rlImGuiColors::Convert(imguiWallColor);
				lighting.wallColor.r = imguiWallColorRl.r;
				lighting.wallColor.g = imguiWallColorRl.g;
				lighting.wallColor.b = imguiWallColorRl.b;
				lighting.wallColor.a = imguiWallColorRl.a;
			}

			sliderHelper("Max Samples", "Controls the total amount of lighting iterations", lighting.maxSamples, 1, 6, parametersSliderX, parametersSliderY, enabled);
			sliderHelper("Rays Per Sample", "Controls amount of rays emitted on each sample", lighting.sampleRaysAmount, 1, 80000, parametersSliderX, parametersSliderY, enabled);
			sliderHelper("Max Bounces", "Controls how many times rays can bounce", lighting.maxBounces, 0, 8, parametersSliderX, parametersSliderY, enabled);

		}
	}

	ImGui::EndChild();

	ImGui::End();

	myParam.rightClickSettings.rightClickMenu(myVar, myParam);

	myParam.controls.showControls();
	myParam.controls.showInfo(myVar.fullscreenState);

	ImVec2 statsSize = { 250.0f, 120.0f };

	ImGui::SetNextWindowSize(statsSize, ImGuiCond_Always);
	ImGui::SetNextWindowPos(ImVec2(screenX - statsSize.x - buttonsWindowX - 20.0f, 0.0f), ImGuiCond_Always);

	ImGui::Begin("Stats", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

	ImGui::PushFont(myVar.robotoMediumFont);

	ImGui::SetWindowFontScale(1.5f);

	int particlesAmout = static_cast<int>(myParam.pParticles.size());
	int selecParticlesAmout = static_cast<int>(myParam.pParticlesSelected.size());

	ImGui::Text("%s%d", "Total Particles: ", particlesAmout);

	ImGui::Text("%s%d", "Selected Particles: ", selecParticlesAmout);

	if (GetFPS() >= 60) {
		ImGui::TextColored(ImVec4(0.0f, 0.8f, 0.0f, 1.0f), "%s%d", "FPS: ", GetFPS());
	}
	else if (GetFPS() < 60 && GetFPS() > 30) {
		ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.0f, 1.0f), "%s%d", "FPS: ", GetFPS());
	}
	else {
		ImGui::TextColored(ImVec4(0.8f, 0.0f, 0.0f, 1.0f), "%s%d", "FPS: ", GetFPS());
	}

	ImGui::PopFont();

	ImGui::End();
}

void UI::statsWindowLogic(UpdateParameters& myParam, UpdateVariables& myVar) {

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	//------ Performance ------//

	ImGui::TextColored(statsNamesCol, "Performance");
	ImGui::Spacing();

	float enablePausedPlot = 1.0f;

	plotLinesHelper(enablePausedPlot, "Framerate: ", graphHistoryLimit, ImGui::GetIO().Framerate, 0.0f, 144.0f, { 340.0f, 200.0f });
	ImGui::Spacing();
	plotLinesHelper(enablePausedPlot, "Frame Time: ", graphHistoryLimit, GetFrameTime(), 0.0f, 1.0f, { 340.0f, 200.0f });

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	//------ Particle Count ------//

	ImGui::TextColored(statsNamesCol, "Particle Count");
	ImGui::Spacing();

	int particlesAmout = static_cast<int>(myParam.pParticles.size());
	int selecParticlesAmout = static_cast<int>(myParam.pParticlesSelected.size());

	ImGui::Text("%s%d", "Total Particles: ", particlesAmout);

	ImGui::Text("%s%d", "Selected Particles: ", selecParticlesAmout);

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	//------ Composition ------//

	ImGui::TextColored(statsNamesCol, "Composition");
	ImGui::Spacing();

	float waterAmount = 0.0f;
	float rockAmount = 0.0f;
	float ironAmount = 0.0f;
	float sandAmount = 0.0f;
	float soilAmount = 0.0f;
	float mudAmount = 0.0f;
	float rubberAmount = 0.0f;

	// This is not the ideal way to do it, but I'm using this for now because there are not many materials
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

	ImGui::TextColored(statsNamesCol, "Mass");
	ImGui::Spacing();

	double totalMass = 0.0f;

	for (size_t i = 0; i < myParam.pParticles.size(); i++) {
		totalMass += myParam.pParticles[i].mass;
	}

	ImGui::Text("Total Mass: %.2f", totalMass);

	double selectedMas = 0.0f;

	for (size_t i = 0; i < myParam.pParticles.size(); i++) {
		if (myParam.rParticles[i].isSelected) {
			selectedMas += myParam.pParticles[i].mass;
		}
	}

	ImGui::Text("Selected Mass: %.2f", selectedMas);

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	//------ Velocity ------//

	ImGui::TextColored(statsNamesCol, "Selected Velocity");
	ImGui::Spacing();

	glm::vec2 selectedVel = { 0.0f, 0.0f };
	float totalVel = 0.0f;

	for (size_t i = 0; i < myParam.pParticles.size(); i++) {
		if (myParam.rParticles[i].isSelected) {
			selectedVel += myParam.pParticles[i].vel;
		}
	}

	if (myParam.pParticlesSelected.size() > 0) {
		selectedVel /= myParam.pParticlesSelected.size();
		totalVel = sqrt(selectedVel.x * selectedVel.x + selectedVel.y * selectedVel.y);
	}

	plotLinesHelper(myVar.timeFactor, "Velocity X: ", graphHistoryLimit, selectedVel.x, -300.0f, 300.0f, graphDefaultSize);
	ImGui::Spacing();
	plotLinesHelper(myVar.timeFactor, "Velocity Y: ", graphHistoryLimit, selectedVel.y, -300.0f, 300.0f, graphDefaultSize);
	ImGui::Spacing();
	plotLinesHelper(myVar.timeFactor, "Total Velocity: ", graphHistoryLimit, totalVel, -300.0f, 300.0f, graphDefaultSize);

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	//------ Acceleration ------//

	ImGui::TextColored(statsNamesCol, "Selected Acceleration");
	ImGui::Spacing();

	glm::vec2 selectedAcc = { 0.0f, 0.0f };
	float totalAcc = 0.0f;

	for (size_t i = 0; i < myParam.pParticles.size(); i++) {
		if (myParam.rParticles[i].isSelected) {
			selectedAcc += myParam.pParticles[i].acc;
		}
	}

	if (myParam.pParticlesSelected.size() > 0) {
		selectedAcc /= myParam.pParticlesSelected.size();
		totalAcc = sqrt(selectedAcc.x * selectedAcc.x + selectedAcc.y * selectedAcc.y);
	}

	plotLinesHelper(myVar.timeFactor, "Acceleration X: ", graphHistoryLimit, selectedAcc.x, -300.0f, 300.0f, graphDefaultSize);
	ImGui::Spacing();
	plotLinesHelper(myVar.timeFactor, "Acceleration Y: ", graphHistoryLimit, selectedAcc.y, -300.0f, 300.0f, graphDefaultSize);
	ImGui::Spacing();
	plotLinesHelper(myVar.timeFactor, "Total Acceleration: ", graphHistoryLimit, totalAcc, -300.0f, 300.0f, graphDefaultSize);

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	//------ Pressure ------//

	ImGui::TextColored(statsNamesCol, "Selected Pressure");
	ImGui::Spacing();

	float totalPress = 0.0f;

	for (size_t i = 0; i < myParam.pParticles.size(); i++) {
		if (myParam.rParticles[i].isSelected) {
			totalPress += myParam.pParticles[i].press;
		}
	}

	if (myParam.pParticlesSelected.size() > 0) {
		totalPress /= myParam.pParticlesSelected.size();
	}

	plotLinesHelper(myVar.timeFactor, "Pressure: ", graphHistoryLimit, totalPress, 0.0f, 100.0f, graphDefaultSize);

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	//------ Temperature ------//

	ImGui::TextColored(statsNamesCol, "Selected Temperature");
	ImGui::Spacing();

	float totalTemp = 0.0f;

	for (size_t i = 0; i < myParam.pParticles.size(); i++) {
		if (myParam.rParticles[i].isSelected) {
			totalTemp += myParam.pParticles[i].temp;
		}
	}

	if (myParam.pParticlesSelected.size() > 0) {
		totalTemp /= myParam.pParticlesSelected.size();
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



void UI::sliderHelper(std::string label, std::string tooltip, float& parameter, float minVal, float maxVal,
	float sizeX, float sizeY, bool& isEnabled) {

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

	ImGui::SliderFloat(("##" + label).c_str(), &parameter, minVal, maxVal, "%.3f", ImGuiSliderFlags_Logarithmic);

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
	}

	if (!isEnabled) {
		ImGui::EndDisabled();
	}
}

void UI::sliderHelper(std::string label, std::string tooltip, int& parameter, int minVal, int maxVal,
	float sizeX, float sizeY, bool& isEnabled) {

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

	ImGui::SliderInt(("##" + label).c_str(), &parameter, minVal, maxVal);

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
	}

	if (!isEnabled) {
		ImGui::EndDisabled();
	}
}
