#include "../../include/UI/UI.h"
#include <variant>


void UI::uiLogic(UpdateParameters& myParam, UpdateVariables& myVar, SPH& sph, SaveSystem& save) {


	if (IsKeyPressed(KEY_U)) {
		showSettings = !showSettings;
	}

	if (myVar.timeFactor == 0.0f) {
		DrawRectangleV({ GetScreenWidth() - 200.0f, 20.0f }, { 10.0f, 30.0f }, WHITE);
		DrawRectangleV({ GetScreenWidth() - 220.0f, 20.0f }, { 10.0f, 30.0f }, WHITE);
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
		settingsParams("Fullscreen", myVar.fullscreenState),
		settingsParams("Controls", myParam.controls.isShowControlsEnabled),
		settingsParams("Information", myParam.controls.isInformationEnabled),
		settingsParams("Global Trails", myVar.isGlobalTrailsEnabled),
		settingsParams("Selected Trails", myVar.isSelectedTrailsEnabled),
		settingsParams("Local Trails", myVar.isLocalTrailsEnabled),
		settingsParams("White Trails", myParam.trails.whiteTrails),
		settingsParams("Solid Color", myParam.colorVisuals.solidColor),
		settingsParams("Density Color", myParam.colorVisuals.densityColor),
		settingsParams("Force Color", myParam.colorVisuals.forceColor),
		settingsParams("Velocity Color", myParam.colorVisuals.velocityColor),
		settingsParams("DeltaV Color", myParam.colorVisuals.deltaVColor),
		settingsParams("SPH Color", myParam.colorVisuals.SPHColor),
		settingsParams("Selected Color", myParam.colorVisuals.selectedColor),
		settingsParams("Dark Matter", myVar.isDarkMatterEnabled),
		settingsParams("Show Dark Matter", myParam.colorVisuals.showDarkMatterEnabled),
		settingsParams("Looping Space", myVar.isPeriodicBoundaryEnabled),
		settingsParams("Multi-Threading", myVar.isMultiThreadingEnabled),
		settingsParams("SPH", myVar.isSPHEnabled),
		settingsParams("SPH Ground Mode", myVar.sphGround),
		settingsParams("Collisions (!!!)", myVar.isCollisionsEnabled),
		settingsParams("Density Size", myVar.isDensitySizeEnabled),
		settingsParams("Force Size", myVar.isForceSizeEnabled),
		settingsParams("Glow", myVar.isGlowEnabled),
		settingsParams("Predict Path", myParam.particlesSpawning.enablePathPrediction),
		settingsParams("Ship Gas", myVar.isShipGasEnabled),
		settingsParams("Save Scene", save.saveFlag),
		settingsParams("Load Scene", save.loadFlag)

	};

	static std::array<std::variant<
		visualSlidersParams<float>,
		visualSlidersParams<int>>,
		11> visualSliders = {
		visualSlidersParams<float>("Density Radius", myParam.neighborSearch.densityRadius, 0.0f, 7.0f),
		visualSlidersParams<int>("Max Neighbors", myParam.colorVisuals.maxNeighbors, 1, 500),
		visualSlidersParams<float>("Max Color Force", myParam.colorVisuals.maxColorAcc, 1.0f, 400.0f),
		visualSlidersParams<float>("Max Size Force", myParam.densitySize.sizeAcc, 1.0f, 400.0f),
		visualSlidersParams<float>("Max DeltaV Accel", myParam.colorVisuals.deltaVMaxAccel, 1.0f, 40.0f),
		visualSlidersParams<int>("Trails Length", myVar.trailMaxLength, 0, 1500),
		visualSlidersParams<float>("Trails Thickness", myParam.trails.trailThickness, 0.007f, 0.5f),
		visualSlidersParams<float>("Particles Size", myVar.particleSizeMultiplier, 0.1f, 5.0f),
		visualSlidersParams<int>("Path Prediction Lenght", myParam.particlesSpawning.predictPathLength, 100, 2000),
		visualSlidersParams<float>("Visible P. Amount Multiplier", myParam.particlesSpawning.particleAmountMultiplier, 0.1f, 100.0f),
		visualSlidersParams<float>("DM P. Amount Multiplier", myParam.particlesSpawning.DMAmountMultiplier, 0.1f, 100.0f)
	};

	static std::array<std::variant<
		physicsSlidersParams<float>,
		physicsSlidersParams<int>>,
		16> physicsSliders = {
		physicsSlidersParams<float>("Softening", myVar.softening, 1.0f, 30.0f),
		physicsSlidersParams<float>("Theta", myVar.theta, 0.1f, 5.0f),
		physicsSlidersParams<float>("Time Scale", myVar.timeStepMultiplier, 0.0f, 15.0f),
		physicsSlidersParams<int>("Collision Substeps", myVar.substeps, 1, 64),
		physicsSlidersParams<float>("Gravity Strength", myVar.gravityMultiplier, 0.0f, 100.0f),
		physicsSlidersParams<float>("Heavy Particle Init Mass", myParam.particlesSpawning.heavyParticleWeightMultiplier, 0.005f, 15.0f),
		physicsSlidersParams<float>("Collisions Bounciness", myVar.particleBounciness, 0.0f, 1.0f),
		physicsSlidersParams<float>("Domain Width", myVar.domainSize.x, 200.0f, 3840.0f),
		physicsSlidersParams<float>("Domain Height", myVar.domainSize.y, 200.0f, 2160.0f),
		physicsSlidersParams<int>("Threads Amount", myVar.threadsAmount, 1, 32),

		physicsSlidersParams<float>("SPH Rest Pressure", sph.restDensity, 0.01f, 5.0f),
		physicsSlidersParams<float>("SPH Stiffness", sph.stiffness, 0.0f, 300.0f),
		physicsSlidersParams<float>("SPH Radius", sph.radiusMultiplier, 0.1f, 4.0f),
		physicsSlidersParams<float>("SPH Mass Multiplier", sph.mass, 0.05f, 0.15f),
		physicsSlidersParams<float>("SPH Viscosity", sph.viscosity, 1.0f, 250.0f),
		physicsSlidersParams<float>("SPH Cohesion", sph.cohesionCoefficient, 0.0f, 10.0f),
	};

	float oldSpacingY = ImGui::GetStyle().ItemSpacing.y;
	ImGui::GetStyle().ItemSpacing.y = 5.0f; // Set the spacing only for the settings buttons


	for (size_t i = 0; i < settingsButtonsParams.size(); i++) {
		ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMin().x);

		bool isColor = settingsButtonsParams[i].text.find("Color") != std::string::npos;
		bool& current = settingsButtonsParams[i].parameter;
		const std::string& label = settingsButtonsParams[i].text;

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

					if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
						s.parameter = s.defaultVal;
					}
				}
				}, visualSliders[i]);


			if (ImGui::IsItemHovered()) {
				ImGui::SetTooltip("Right-click to reset");
			}

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
