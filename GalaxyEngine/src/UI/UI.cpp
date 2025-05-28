#include "../../include/UI/UI.h"
#include <variant>


void UI::uiLogic(UpdateParameters& myParam, UpdateVariables& myVar, SPH& sph, SaveSystem& save) {


	if (IO::handleShortcut(KEY_U)) {
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
	float settingsButtonY = 30.0f;

	ImGui::SetNextWindowSize(ImVec2(buttonsWindowX, buttonsWindowY), ImGuiCond_Once);
	ImGui::SetNextWindowSizeConstraints(ImVec2(buttonsWindowX, buttonsWindowY), ImVec2(buttonsWindowX, buttonsWindowY));
	ImGui::SetNextWindowPos(ImVec2(screenX - buttonsWindowX, 0.0f), ImGuiCond_Always);
	ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_NoResize);

	float contentRegionWidth = ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x;
	float buttonX = (contentRegionWidth - settingsButtonX) * 0.5f;

	static std::array<settingsParams, 29> settingsButtonsParams = {
		settingsParams("Fullscreen", "Toggles fulscreen", myVar.fullscreenState),
		settingsParams("Controls", "Open controls panel", myParam.controls.isShowControlsEnabled),
		settingsParams("Information", "Open information panel", myParam.controls.isInformationEnabled),
		settingsParams("Global Trails", "Enables trails for all particles", myVar.isGlobalTrailsEnabled),
		settingsParams("Selected Trails", "Enables trails for selected particles", myVar.isSelectedTrailsEnabled),
		settingsParams("Local Trails", "Enables trails moving relative to particles average position", myVar.isLocalTrailsEnabled),
		settingsParams("White Trails", "Makes all trails white", myParam.trails.whiteTrails),
		settingsParams("Solid Color", "Particles will only use the primary color", myParam.colorVisuals.solidColor),
		settingsParams("Density Color", "Maps particle neighbor amount to the primary and secondary colors", myParam.colorVisuals.densityColor),
		settingsParams("Force Color", "Maps particle acceleration to the primary and secondary colors",myParam.colorVisuals.forceColor),
		settingsParams("Velocity Color", "Maps particle velocity to color", myParam.colorVisuals.velocityColor),
		settingsParams("DeltaV Color", "Maps particle change in speed to color", myParam.colorVisuals.deltaVColor),
		settingsParams("Pressure Color", "Maps particle pressure to color", myParam.colorVisuals.pressureColor),
		settingsParams("SPH Color", "Uses the SPH materials colors", myParam.colorVisuals.SPHColor),
		settingsParams("Selected Color", "Highlight selected particles", myParam.colorVisuals.selectedColor),
		settingsParams("Dark Matter", "Enables dark matter particles. This works for galaxies and Big Bang", myVar.isDarkMatterEnabled),
		settingsParams("Show Dark Matter", "Unhides dark matter particles", myParam.colorVisuals.showDarkMatterEnabled),
		settingsParams("Looping Space", "Particles disappearing on one side will appear on the other side", myVar.isPeriodicBoundaryEnabled),
		settingsParams("Multi-Threading", "Distributes the simulation across multiple threads", myVar.isMultiThreadingEnabled),
		settingsParams("SPH", "Enables SPH fluids", myVar.isSPHEnabled),
		settingsParams("SPH Ground Mode", "Adds vertical gravity and makes particles collide with the domain walls", myVar.sphGround),
		settingsParams("Collisions (!!!)", "Enables elastic collisions", myVar.isCollisionsEnabled),
		settingsParams("Density Size", "Maps particle neighbor amount to size", myVar.isDensitySizeEnabled),
		settingsParams("Force Size", "Maps particle acceleration to size", myVar.isForceSizeEnabled),
		settingsParams("Glow", "Enables glow shader", myVar.isGlowEnabled),
		settingsParams("Predict Path", "Predicts the trajectory of heavy particles before launching them", myParam.particlesSpawning.enablePathPrediction),
		settingsParams("Ship Gas", "Enables gas particles coming from the ship when controlling particles", myVar.isShipGasEnabled),
		settingsParams("Save Scene", "Save current scene to disk", save.saveFlag),
		settingsParams("Load Scene", "Load a scene from disk", save.loadFlag)

	};

	static std::array<std::variant<
		visualSlidersParams<float>,
		visualSlidersParams<int>>,
		13> visualSliders = {
		visualSlidersParams<float>("Density Radius", "Controls the neighbor search radius", myParam.neighborSearch.densityRadius, 0.0f, 7.0f),
		visualSlidersParams<int>("Max Neighbors", "Controls the maximum neighbor count range", myParam.colorVisuals.maxNeighbors, 1, 500),
		visualSlidersParams<float>("Max Color Force", "Controls the acceleration threshold to use the secondary color", myParam.colorVisuals.maxColorAcc, 1.0f, 400.0f),
		visualSlidersParams<float>("Max Size Force", "Controls the acceleration threshold to map the particle size", myParam.densitySize.sizeAcc, 1.0f, 400.0f),
		visualSlidersParams<float>("Max DeltaV Accel", "Controls the change in speed threshold to map the particle color in DeltaV color mode", myParam.colorVisuals.deltaVMaxAccel, 0.1f, 40.0f),
		visualSlidersParams<float>("Max Velocity Color", "Controls the max velocity used to map the colors in the velocity color mode", myParam.colorVisuals.maxVel, 10.0f, 10000.0f),
		visualSlidersParams<float>("Max Pressure Color", "Controls the max pressure used to map the colors in the pressure color mode", myParam.colorVisuals.maxPress, 100.0f, 100000.0f),
		visualSlidersParams<int>("Trails Length", "Controls how long should the trails be. This feature is computationally expensive", myVar.trailMaxLength, 0, 1500),
		visualSlidersParams<float>("Trails Thickness", "Controls the trails thickness", myParam.trails.trailThickness, 0.007f, 0.5f),
		visualSlidersParams<float>("Particles Size", "Controls the size of all particles", myVar.particleSizeMultiplier, 0.1f, 5.0f),
		visualSlidersParams<int>("Path Prediction Lenght", "Controls how long is the predicted path", myParam.particlesSpawning.predictPathLength, 100, 2000),
		visualSlidersParams<float>("Visible P. Amount Multiplier", "Controls the spawn amount of visible particles", myParam.particlesSpawning.particleAmountMultiplier, 0.1f, 100.0f),
		visualSlidersParams<float>("DM P. Amount Multiplier", "Controls the spawn amount of dark matter particles", myParam.particlesSpawning.DMAmountMultiplier, 0.1f, 100.0f)
	};

	static std::array<std::variant<
		physicsSlidersParams<float>,
		physicsSlidersParams<int>>,
		17> physicsSliders = {
		physicsSlidersParams<float>("Softening", "Controls the smoothness of the gravity forces", myVar.softening, 1.0f, 30.0f),
		physicsSlidersParams<float>("Theta", "Controls the quality of the gravity calculation. Higher means lower quality", myVar.theta, 0.1f, 5.0f),
		physicsSlidersParams<float>("Time Scale", "Controls how fast time passes", myVar.timeStepMultiplier, 0.0f, 15.0f),
		physicsSlidersParams<int>("Collision Substeps", "Controls the amount of collision substeps. Only works if collisions are on", myVar.substeps, 1, 64),
		physicsSlidersParams<float>("Gravity Strength", "Controls how mcuh particles attract eachother", myVar.gravityMultiplier, 0.0f, 100.0f),
		physicsSlidersParams<float>("Heavy Particle Init Mass", "Controls the mass of the heavy particles when spawned", myParam.particlesSpawning.heavyParticleWeightMultiplier, 0.005f, 15.0f),
		physicsSlidersParams<float>("Collisions Bounciness", "Controls how much particles bounce when collisions mode is enabled", myVar.particleBounciness, 0.0f, 1.0f),
		physicsSlidersParams<float>("Domain Width", "Controls the width of the global container", myVar.domainSize.x, 200.0f, 3840.0f),
		physicsSlidersParams<float>("Domain Height", "Controls the height of the global container", myVar.domainSize.y, 200.0f, 2160.0f),
		physicsSlidersParams<int>("Threads Amount", "Controls the amount of cpu threads used by the simulation", myVar.threadsAmount, 1, 32),

		physicsSlidersParams<float>("SPH Rest Density", "Controls what the density should be in equilibrium", sph.restDensity, 0.001f, 10.0f),
		physicsSlidersParams<float>("SPH Radius", "Controls the hitbox of the particles in SPH mode", sph.radiusMultiplier, 0.1f, 4.0f),
		physicsSlidersParams<float>("SPH Mass Multiplier", "Controls the fluid mass of particles", sph.mass, 0.005f, 0.15f),
		physicsSlidersParams<float>("SPH Viscosity", "Controls how viscous particles are", sph.viscosity, 0.01f, 15.0f),
		physicsSlidersParams<float>("SPH Cohesion", "Controls how sticky particles are", sph.cohesionCoefficient, 0.0f, 10.0f),
		physicsSlidersParams<float>("SPH Delta", "Controls the scaling factor in the pressure solver to enforce fluid incompressibility", sph.delta, 500.0f, 20000.0f),
		physicsSlidersParams<float>("SPH Max Velocity", "Controls the maximum velocity a particle can have in SPH mode", myVar.sphMaxVel, 0.0f, 2000.0f),
	};

	float oldSpacingY = ImGui::GetStyle().ItemSpacing.y;
	ImGui::GetStyle().ItemSpacing.y = 5.0f; // Set the spacing only for the settings buttons


	for (size_t i = 0; i < settingsButtonsParams.size(); i++) {
		ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMin().x);

		bool isColor = settingsButtonsParams[i].text.find("Color") != std::string::npos;
		bool& current = settingsButtonsParams[i].parameter;
		const std::string& label = settingsButtonsParams[i].text;
		const std::string& tooltip = settingsButtonsParams[i].tooltip;

		ImVec4& col = current ? myVar.buttonEnabledColor : myVar.buttonDisabledColor;
		ImGui::PushStyleColor(ImGuiCol_Button, col);
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(col.x + 0.1f, col.y + 0.1f, col.z + 0.1f, col.w));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(col.x - 0.1f, col.y - 0.1f, col.z - 0.1f, col.w));

		auto toggleExclusive = [&](const std::string& self, const std::string& other) {
			if (!current) {
				current = true;
				for (auto& p : settingsButtonsParams) {
					if (p.text == other) p.parameter = false;
				}
			}
			else {
				current = false;
			}
			};

		if (ImGui::Button(label.c_str(), ImVec2(ImGui::GetContentRegionAvail().x, settingsButtonY))) {
			if (label == "Information") {
				toggleExclusive("Information", "Controls");
			}
			else if (label == "Controls") {
				toggleExclusive("Controls", "Information");
			}
			else if (label == "Density Size") {
				toggleExclusive("Density Size", "Force Size");
			}
			else if (label == "Force Size") {
				toggleExclusive("Force Size", "Density Size");
			}
			else if (label == "Global Trails") {
				toggleExclusive("Global Trails", "Selected Trails");
			}
			else if (label == "Selected Trails") {
				toggleExclusive("Selected Trails", "Global Trails");
			}
			else if (label == "Visual Settings") {
				toggleExclusive("Visual Settings", "Physics Settings");
			}
			else if (label == "Physics Settings") {
				toggleExclusive("Physics Settings", "Visual Settings");
			}
			else if (label == "SPH") {
				current = !current;

				// When SPH is turned ON, enable SPH Color and disable others (except "Selected Color")
				if (current) {
					for (auto& p : settingsButtonsParams) {
						if (p.text.find("Color") != std::string::npos &&
							p.text != "SPH Color" &&
							p.text != "Selected Color") {
							p.parameter = false;
						}
						if (p.text == "SPH Color") {
							p.parameter = true;
						}
					}
				}
			}
			else if (isColor && label != "Selected Color") {
				// Standard color toggle logic
				for (auto& p : settingsButtonsParams) {
					if (p.text.find("Color") != std::string::npos &&
						p.text != "Selected Color") {
						p.parameter = false;
					}
				}
				current = true;
			}
			else if (isColor && label != "Selected Color") {
				for (auto& p : settingsButtonsParams) {
					if (p.text.find("Color") != std::string::npos &&
						p.text != "Selected Color") {
						p.parameter = false;
					}
				}
				current = true;
			}
			else {
				current = !current;
			}
		}

		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip("%s", tooltip.c_str());
		}

		ImGui::PopStyleColor(3);
	}

	ImGui::GetStyle().ItemSpacing.y = oldSpacingY; // End the settings buttons spacing

	ImGui::End();

	// Start of settings sliders

	float parametersWindowSizeX = 400.0f;
	float parametersWindowSizeY = screenY - 30.0f;

	float parametersSliderX = 200.0f;
	float parametersSliderY = 30.0f;

	ImGui::SetNextWindowSize(ImVec2(parametersWindowSizeX, parametersWindowSizeY), ImGuiCond_Once);
	ImGui::SetNextWindowSizeConstraints(ImVec2(parametersWindowSizeX, parametersWindowSizeY), ImVec2(parametersWindowSizeX, parametersWindowSizeY));
	ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
	ImGui::Begin("Parameters", nullptr, ImGuiWindowFlags_NoResize);
	float totalWidth = ImGui::GetContentRegionAvail().x;
	float halfButtonWidth = totalWidth * 0.4f;

	// Top row: Visual Sliders | Physics Sliders
	ImVec4& colVisual = bVisualsSliders ? myVar.buttonEnabledColor : myVar.buttonDisabledColor;
	ImGui::PushStyleColor(ImGuiCol_Button, colVisual);
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(colVisual.x + 0.1f, colVisual.y + 0.1f, colVisual.z + 0.1f, colVisual.w));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(colVisual.x - 0.1f, colVisual.y - 0.1f, colVisual.z - 0.1f, colVisual.w));

	if (ImGui::Button("Visual Sliders", ImVec2(halfButtonWidth, settingsButtonY))) {
		bVisualsSliders = true;
		bPhysicsSliders = false;
		bRecordingSettings = false;
		statsWindow = false;
	}
	ImGui::PopStyleColor(3);

	ImGui::SameLine();

	ImVec4& colPhysics = bPhysicsSliders ? myVar.buttonEnabledColor : myVar.buttonDisabledColor;
	ImGui::PushStyleColor(ImGuiCol_Button, colPhysics);
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(colPhysics.x + 0.1f, colPhysics.y + 0.1f, colPhysics.z + 0.1f, colPhysics.w));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(colPhysics.x - 0.1f, colPhysics.y - 0.1f, colPhysics.z - 0.1f, colPhysics.w));

	if (ImGui::Button("Physics Sliders", ImVec2(halfButtonWidth, settingsButtonY))) {
		bPhysicsSliders = true;
		bVisualsSliders = false;
		bRecordingSettings = false;
		statsWindow = false;
	}
	ImGui::PopStyleColor(3);
	// Bottom row: Advanced Statistics | Recording Settings
	ImVec4& colStats = statsWindow ? myVar.buttonEnabledColor : myVar.buttonDisabledColor;
	ImGui::PushStyleColor(ImGuiCol_Button, colStats);
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(colStats.x + 0.1f, colStats.y + 0.1f, colStats.z + 0.1f, colStats.w));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(colStats.x - 0.1f, colStats.y - 0.1f, colStats.z - 0.1f, colStats.w));

	if (ImGui::Button("Advanced Statistics", ImVec2(halfButtonWidth, settingsButtonY))) {
		bPhysicsSliders = false;
		bVisualsSliders = false;
		bRecordingSettings = false;
		statsWindow = true;
	}
	ImGui::PopStyleColor(3);

	ImGui::SameLine();

	ImVec4& colRecording = bRecordingSettings ? myVar.buttonEnabledColor : myVar.buttonDisabledColor;
	ImGui::PushStyleColor(ImGuiCol_Button, colRecording);
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(colRecording.x + 0.1f, colRecording.y + 0.1f, colRecording.z + 0.1f, colRecording.w));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(colRecording.x - 0.1f, colRecording.y - 0.1f, colRecording.z - 0.1f, colRecording.w));

	if (ImGui::Button("Recording Settings", ImVec2(halfButtonWidth, settingsButtonY))) {
		bPhysicsSliders = false;
		bVisualsSliders = false;
		bRecordingSettings = true;
		statsWindow = false;
	}
	ImGui::PopStyleColor(3);

	if (bVisualsSliders) {
		Color primaryColors = {
			static_cast<unsigned char>(myParam.colorVisuals.pColor.r),
			static_cast<unsigned char>(myParam.colorVisuals.pColor.g),
			static_cast<unsigned char>(myParam.colorVisuals.pColor.b),
			static_cast<unsigned char>(myParam.colorVisuals.pColor.a) };

		ImVec4 imguiPColor = rlImGuiColors::Convert(primaryColors);
		static Color originalPColor = primaryColors;

		if (ImGui::Button("Reset Primary Colors", ImVec2(240.0f, 30.0f))) {
			myParam.colorVisuals.pColor.r = originalPColor.r;
			myParam.colorVisuals.pColor.g = originalPColor.g;
			myParam.colorVisuals.pColor.b = originalPColor.b;
			myParam.colorVisuals.pColor.a = originalPColor.a;
		}

		if (ImGui::ColorPicker4("Primary Colors", (float*)&imguiPColor, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_DisplayRGB)) {
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

		if (ImGui::Button("Reset Secondary Colors", ImVec2(240.0f, 30.0f))) {
			myParam.colorVisuals.sColor.r = originalSColor.r;
			myParam.colorVisuals.sColor.g = originalSColor.g;
			myParam.colorVisuals.sColor.b = originalSColor.b;
			myParam.colorVisuals.sColor.a = originalSColor.a;
		}

		if (ImGui::ColorPicker4("Secondary Colors", (float*)&imguiSColor, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_DisplayRGB)) {
			secondaryColors = rlImGuiColors::Convert(imguiSColor);
			myParam.colorVisuals.sColor.r = secondaryColors.r;
			myParam.colorVisuals.sColor.g = secondaryColors.g;
			myParam.colorVisuals.sColor.b = secondaryColors.b;
			myParam.colorVisuals.sColor.a = secondaryColors.a;
		}

		for (size_t i = 0; i < visualSliders.size(); i++) {

			std::visit([&](auto& s) {
				using T = std::decay_t<decltype(s)>;
				if constexpr (std::is_same_v<T, visualSlidersParams<float>>) {
					ImGui::Text("%s", s.text.c_str());

					if (s.min > 0.0f) {
						ImGui::SliderFloat(
							("##" + s.text).c_str(),
							&s.parameter,
							s.min,
							s.max,
							"%.3f", ImGuiSliderFlags_Logarithmic
						);
					}
					else {
						ImGui::SliderFloat(
							("##" + s.text).c_str(),
							&s.parameter,
							s.min,
							s.max
						);
					}

					if (ImGui::IsItemHovered()) {
						ImGui::SetTooltip("%s", s.tooltip.c_str());
					}

					if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
						s.parameter = s.defaultVal;
					}
				}
				else if constexpr (std::is_same_v<T, visualSlidersParams<int>>) {
					ImGui::Text("%s", s.text.c_str());

					ImGui::SliderInt(
						("##" + s.text).c_str(),
						&s.parameter,
						s.min,
						s.max
					);

					if (ImGui::IsItemHovered()) {
						ImGui::SetTooltip("%s", s.tooltip.c_str());
					}

					if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
						s.parameter = s.defaultVal;
					}
				}
				}, visualSliders[i]);

		}
	}

	if (bPhysicsSliders) {

		for (size_t i = 0; i < physicsSliders.size(); i++) {

			std::visit([&](auto& s) {
				using T = std::decay_t<decltype(s)>;
				if constexpr (std::is_same_v<T, physicsSlidersParams<float>>) {
					ImGui::Text("%s", s.text.c_str());

					ImGui::SliderFloat(
						("##" + s.text).c_str(),
						&s.parameter,
						s.min,
						s.max
					);

					if (ImGui::IsItemHovered()) {
						ImGui::SetTooltip("%s", s.tooltip.c_str());
					}

					if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
						s.parameter = s.defaultVal;
					}
				}
				else if constexpr (std::is_same_v<T, physicsSlidersParams<int>>) {
					ImGui::Text("%s", s.text.c_str());

					ImGui::SliderInt(
						("##" + s.text).c_str(),
						&s.parameter,
						s.min,
						s.max
					);

					if (ImGui::IsItemHovered()) {
						ImGui::SetTooltip("%s", s.tooltip.c_str());
					}

					if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
						s.parameter = s.defaultVal;
					}
				}
				}, physicsSliders[i]);


			/*if (ImGui::IsItemHovered()) {
				ImGui::SetTooltip("Right-click to reset");
			}*/

		}

		static std::array<sphParams, 6> sphButtonsParams = {
		sphParams("SPH Water", myParam.brush.SPHWater),
		sphParams("SPH Rock", myParam.brush.SPHRock),
		sphParams("SPH Sand", myParam.brush.SPHSand),
		sphParams("SPH Soil", myParam.brush.SPHSoil),
		sphParams("SPH Ice", myParam.brush.SPHIce),
		sphParams("SPH Mud", myParam.brush.SPHMud)

		};

		float oldSpacingY = ImGui::GetStyle().ItemSpacing.y;
		ImGui::GetStyle().ItemSpacing.y = 5.0f; // Set the spacing only for the settings buttons


		for (size_t i = 0; i < sphButtonsParams.size(); i++) {
			ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMin().x);

			auto& entry = sphButtonsParams[i];
			bool& current = sphButtonsParams[i].parameter;
			const std::string& label = sphButtonsParams[i].text;

			ImVec4& col = current ? myVar.buttonEnabledColor : myVar.buttonDisabledColor;
			ImGui::PushStyleColor(ImGuiCol_Button, col);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(col.x + 0.1f, col.y + 0.1f, col.z + 0.1f, col.w));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(col.x - 0.1f, col.y - 0.1f, col.z - 0.1f, col.w));

			if (ImGui::Button(label.c_str(), ImVec2(ImGui::GetContentRegionAvail().x, settingsButtonY))) {

				if (!current) {
					current = true;

					for (sphParams& other : sphButtonsParams) {
						if (&other != &entry) {
							other.parameter = false;
						}
						else {
							current = true;
						}
					}
				}
			}

			ImGui::PopStyleColor(3);
		}

		ImGui::GetStyle().ItemSpacing.y = oldSpacingY;

	}

	if (bRecordingSettings) {
		static std::array<settingsParams, 2> recordingButtonsParams = {
			settingsParams("Pause After Recording", "Pauses the simulation after recording is finished", myVar.pauseAfterRecording),
			settingsParams("Clean Scene After Recording", "Clears all particles from the scene after recording is finished", myVar.cleanSceneAfterRecording)
		};

		float oldSpacingY = ImGui::GetStyle().ItemSpacing.y;
		ImGui::GetStyle().ItemSpacing.y = 5.0f; // Set the spacing only for the recording settings buttons

		for (size_t i = 0; i < recordingButtonsParams.size(); i++) {
			ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMin().x);

			bool& current = recordingButtonsParams[i].parameter;
			const std::string& label = recordingButtonsParams[i].text;
			const std::string& tooltip = recordingButtonsParams[i].tooltip;

			ImVec4& col = current ? myVar.buttonEnabledColor : myVar.buttonDisabledColor;
			ImGui::PushStyleColor(ImGuiCol_Button, col);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(col.x + 0.1f, col.y + 0.1f, col.z + 0.1f, col.w));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(col.x - 0.1f, col.y - 0.1f, col.z - 0.1f, col.w));

			if (ImGui::Button(label.c_str(), ImVec2(ImGui::GetContentRegionAvail().x, settingsButtonY))) {
				current = !current;
			}

			if (ImGui::IsItemHovered()) {
				ImGui::SetTooltip("%s", tooltip.c_str());
			}

			ImGui::PopStyleColor(3);
		}

		ImGui::Separator(); // Add a separator

		// Recording Timer Slider
		ImGui::Text("Recording Timer (seconds)");
		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip("Set a time limit for the recording. 0 means no limit.");
		}
		if (myVar.isRecording) { // Check if recording is active
			ImGui::BeginDisabled(true); // Disable the slider
		}
		ImGui::SliderFloat("##RecordingTimeLimit", &myVar.recordingTimeLimit, 0.0f, 60.0f, "%.1f s");
		if (myVar.isRecording) { // If recording was active
			ImGui::EndDisabled(); // Re-enable the slider
		}
		if (ImGui::IsItemEdited() && myVar.recordingTimeLimit < 0) {
			myVar.recordingTimeLimit = 0; // Ensure it doesn't go below 0
		}


		ImGui::GetStyle().ItemSpacing.y =
			oldSpacingY; // End the recording settings buttons spacing
	}

	if (statsWindow) {
		statsWindowLogic(myParam, myVar);
	}

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

	ImGui::Separator();
	ImGui::Spacing();

	//------ Frame Rate ------//
	const int fpsHistorySize = 100;
	static float fpsValues[fpsHistorySize] = { 0.0f };
	static int fpsValuesOffset = 0;

	float ImGuiFPS = ImGui::GetIO().Framerate;

	fpsValues[fpsValuesOffset] = ImGuiFPS;
	fpsValuesOffset = (fpsValuesOffset + 1) % fpsHistorySize;

	float fps = ImGui::GetIO().Framerate;
	std::string fpsOverlay = "FPS: " + std::to_string(fps);

	ImGui::PlotLines("##FrameRate", fpsValues, fpsHistorySize, fpsValuesOffset, fpsOverlay.c_str(), 0.0f, 144.0f, ImVec2(0, 100));

	ImGui::Separator();

	//------ Particle Count ------//

	int particlesAmout = static_cast<int>(myParam.pParticles.size());
	int selecParticlesAmout = static_cast<int>(myParam.pParticlesSelected.size());

	ImGui::Text("%s%d", "Total Particles: ", particlesAmout);

	ImGui::Text("%s%d", "Selected Particles: ", selecParticlesAmout);

	ImGui::Separator();

	//------ Particle Mass ------//

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

	ImGui::Separator();

	//------ Particle Velocity ------//

	Vector2 selectedVel = { 0.0f, 0.0f };
	float totalVel = 0.0f;

	for (size_t i = 0; i < myParam.pParticles.size(); i++) {
		if (myParam.rParticles[i].isSelected) {
			selectedVel.x += myParam.pParticles[i].vel.x;
			selectedVel.y += myParam.pParticles[i].vel.y;
		}
	}

	selectedVel.x /= myParam.pParticlesSelected.size();
	selectedVel.y /= myParam.pParticlesSelected.size();

	totalVel = sqrt(selectedVel.x * selectedVel.x + selectedVel.y * selectedVel.y);

	plotLinesHelper("Velocity X: ", 1000, selectedVel.x, -500.0f, 500.0f, { 0.0f, 100.0f });
	ImGui::Spacing();
	plotLinesHelper("Velocity Y: ", 1000, selectedVel.y, -500.0f, 500.0f, { 0.0f, 100.0f });
	ImGui::Spacing();
	plotLinesHelper("Total Velocity: ", 1000, totalVel, -500.0f, 500.0f, { 0.0f, 100.0f });
}

void UI::plotLinesHelper(std::string label,
	const int length,
	float value, const float minValue, const float maxValue, ImVec2 size) {

	auto& plotData = plotDataMap[label];

	if (plotData.values.size() != length) {
		plotData.values.resize(length, 0.0f);
		plotData.offset = 0;
	}

	plotData.values[plotData.offset] = value;
	plotData.offset = (plotData.offset + 1) % length;

	std::string valueOverlay = label + std::to_string(value);

	ImGui::PlotLines(("##" + label).c_str(), plotData.values.data(), length, plotData.offset, valueOverlay.c_str(), minValue, maxValue, size);
}
