#include "../../include/UI/UI.h"
#include <variant>


void UI::uiLogic(UpdateParameters& myParam, UpdateVariables& myVar, SPH& sph, SaveSystem& save) {


	if (IsKeyPressed(KEY_U)) {
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

	static std::array<settingsParams, 28> settingsButtonsParams = {
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
		settingsParams("Velocity Color", "Maps particle velocity to the primary and secondary colors", myParam.colorVisuals.velocityColor),
		settingsParams("DeltaV Color", "Maps particle change in speed to the primary and secondary colors", myParam.colorVisuals.deltaVColor),
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
		11> visualSliders = {
		visualSlidersParams<float>("Density Radius", "Controls the neighbor search radius", myParam.neighborSearch.densityRadius, 0.0f, 7.0f),
		visualSlidersParams<int>("Max Neighbors", "Controls the maximum neighbor count range", myParam.colorVisuals.maxNeighbors, 1, 500),
		visualSlidersParams<float>("Max Color Force", "Controls the acceleration threshold to use the secondary color", myParam.colorVisuals.maxColorAcc, 1.0f, 400.0f),
		visualSlidersParams<float>("Max Size Force", "Controls the acceleration threshold to map the particle size", myParam.densitySize.sizeAcc, 1.0f, 400.0f),
		visualSlidersParams<float>("Max DeltaV Accel", "Controls the change in speed threshold to map the particle color in DeltaV color mode", myParam.colorVisuals.deltaVMaxAccel, 1.0f, 40.0f),
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
		16> physicsSliders = {
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

		physicsSlidersParams<float>("SPH Rest Density", "Controls what the density should be in equilibrium", sph.restDensity, 0.01f, 5.0f),
		physicsSlidersParams<float>("SPH Stiffness", "Controls how rigid particles are", sph.stiffness, 0.0f, 300.0f),
		physicsSlidersParams<float>("SPH Radius", "Controls the hitbox of the particles in SPH mode", sph.radiusMultiplier, 0.1f, 4.0f),
		physicsSlidersParams<float>("SPH Mass Multiplier", "Controls the fluid mass of particles", sph.mass, 0.05f, 0.15f),
		physicsSlidersParams<float>("SPH Viscosity", "Controls how viscous particles are", sph.viscosity, 1.0f, 250.0f),
		physicsSlidersParams<float>("SPH Cohesion", "Controls how sticky particles are", sph.cohesionCoefficient, 0.0f, 10.0f),
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

	ImVec4& colVisual = bVisualsSliders ? myVar.buttonEnabledColor : myVar.buttonDisabledColor;
	ImGui::PushStyleColor(ImGuiCol_Button, colVisual);
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(colVisual.x + 0.1f, colVisual.y + 0.1f, colVisual.z + 0.1f, colVisual.w));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(colVisual.x - 0.1f, colVisual.y - 0.1f, colVisual.z - 0.1f, colVisual.w));

	if (ImGui::Button("Visual Sliders", ImVec2(halfButtonWidth, settingsButtonY))) {
		bVisualsSliders = true;
		bPhysicsSliders = false;
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
	}
	ImGui::PopStyleColor(3);

	if (bVisualsSliders) {
		Color primaryColors = {
			static_cast<unsigned char>(myParam.colorVisuals.primaryR),
			static_cast<unsigned char>(myParam.colorVisuals.primaryG),
			static_cast<unsigned char>(myParam.colorVisuals.primaryB),
			static_cast<unsigned char>(myParam.colorVisuals.primaryA) };

		ImVec4 imguiPColor = rlImGuiColors::Convert(primaryColors);
		static Color originalPColor = primaryColors;

		if (ImGui::Button("Reset Primary Colors", ImVec2(240.0f, 30.0f))) {
			myParam.colorVisuals.primaryR = originalPColor.r;
			myParam.colorVisuals.primaryG = originalPColor.g;
			myParam.colorVisuals.primaryB = originalPColor.b;
			myParam.colorVisuals.primaryA = originalPColor.a;
		}

		if (ImGui::ColorPicker4("Primary Colors", (float*)&imguiPColor, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_DisplayRGB)) {
			primaryColors = rlImGuiColors::Convert(imguiPColor);
			myParam.colorVisuals.primaryR = primaryColors.r;
			myParam.colorVisuals.primaryG = primaryColors.g;
			myParam.colorVisuals.primaryB = primaryColors.b;
			myParam.colorVisuals.primaryA = primaryColors.a;
		}

		Color secondaryColors = {
			static_cast<unsigned char>(myParam.colorVisuals.secondaryR),
			static_cast<unsigned char>(myParam.colorVisuals.secondaryG),
			static_cast<unsigned char>(myParam.colorVisuals.secondaryB),
			static_cast<unsigned char>(myParam.colorVisuals.secondaryA) };

		ImVec4 imguiSColor = rlImGuiColors::Convert(secondaryColors);
		static Color originalSColor = secondaryColors;

		if (ImGui::Button("Reset Secondary Colors", ImVec2(240.0f, 30.0f))) {
			myParam.colorVisuals.secondaryR = originalSColor.r;
			myParam.colorVisuals.secondaryG = originalSColor.g;
			myParam.colorVisuals.secondaryB = originalSColor.b;
			myParam.colorVisuals.secondaryA = originalSColor.a;
		}

		if (ImGui::ColorPicker4("Secondary Colors", (float*)&imguiSColor, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_DisplayRGB)) {
			secondaryColors = rlImGuiColors::Convert(imguiSColor);
			myParam.colorVisuals.secondaryR = secondaryColors.r;
			myParam.colorVisuals.secondaryG = secondaryColors.g;
			myParam.colorVisuals.secondaryB = secondaryColors.b;
			myParam.colorVisuals.secondaryA = secondaryColors.a;
		}

		for (size_t i = 0; i < visualSliders.size(); i++) {

			std::visit([&](auto& s) {
				using T = std::decay_t<decltype(s)>;
				if constexpr (std::is_same_v<T, visualSlidersParams<float>>) {
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

		static std::array<sphParams, 4> sphButtonsParams = {
		sphParams("SPH Water", myParam.brush.SPHWater),
		sphParams("SPH Rock", myParam.brush.SPHRock),
		sphParams("SPH Sand", myParam.brush.SPHSand),
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

	ImGui::End();

	myParam.rightClickSettings.rightClickMenu(myVar, myParam);

	myParam.controls.showControls();
	myParam.controls.showInfo(myVar.fullscreenState);

	ImVec2 statsSize = { 250.0f, 120.0f };

	ImGui::SetNextWindowSize(statsSize, ImGuiCond_Always);
	ImGui::SetNextWindowPos(ImVec2(screenX - statsSize.x - buttonsWindowX - 20.0f, 0.0f), ImGuiCond_Always);

	ImGui::Begin("Stats", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

	ImGui::PushFont(myVar.specialFont);

	ImGui::SetWindowFontScale(1.5f);

	int particlesAmout = static_cast<int>(myParam.pParticles.size());
	int selecParticlesAmout = static_cast<int>(myParam.pParticlesSelected.size());

	ImGui::Text("%s%d","Total Particles: ", particlesAmout);

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
