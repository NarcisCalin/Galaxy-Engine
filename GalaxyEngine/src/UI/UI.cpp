#include "../../include/UI/UI.h"


void UI::uiLogic(UpdateParameters& myParam, UpdateVariables& myVar, SPH& sph, SaveSystem& save) {


	if (IsKeyPressed(KEY_U)) {
		showSettings = !showSettings;
	}

	if (myVar.timeFactor == 0.0f) {
		DrawRectangleV({ GetScreenWidth() - 200.0f, 20.0f }, { 10.0f, 30.0f }, WHITE);
		DrawRectangleV({ GetScreenWidth() - 220.0f, 20.0f }, { 10.0f, 30.0f }, WHITE);
	}

	rlImGuiBegin();

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

	float buttonsWindowX = 300.0f;
	float buttonsWindowY = screenY - 30.0f;

	float settingsButtonX = 250.0f;
	float settingsButtonY = 30.0f;

	ImGui::SetNextWindowSize(ImVec2(buttonsWindowX, buttonsWindowY), ImGuiCond_Once);
	ImGui::SetNextWindowSizeConstraints(ImVec2(buttonsWindowX, buttonsWindowY), ImVec2(buttonsWindowX, buttonsWindowY));
	ImGui::SetNextWindowPos(ImVec2(screenX - buttonsWindowX, 0), ImGuiCond_Always);
	ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_NoResize);

	float contentRegionWidth = ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x;
	float buttonX = (contentRegionWidth - settingsButtonX) * 0.5f;

	struct settingsParams {
		std::string text;
		bool& parameter;

		settingsParams(const std::string& t, bool& p) : text(t), parameter(p) {}
	};

	ImVec4 buttonEnabledColor = { 0.2f,0.5f, 0.2f, 1.0f };
	ImVec4 buttonDisabledColor = { 0.3f,0.3f, 0.3f, 1.0f };

	static std::array<settingsParams, 30> settingsButtonsParams = {
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
		settingsParams("Visual Settings", bVisualsSliders),
		settingsParams("Physics Settings", bPhysicsSliders),
		settingsParams("Ship Gas", myVar.isShipGasEnabled),
		settingsParams("Save Scene", save.saveFlag),
		settingsParams("Load Scene", save.loadFlag)

	};

	float oldSpacingY = ImGui::GetStyle().ItemSpacing.y;
	ImGui::GetStyle().ItemSpacing.y = 5.0f; // Set the spacing only for the settings buttons


	for (size_t i = 0; i < settingsButtonsParams.size(); i++) {
		ImGui::SetCursorPosX(ImGui::GetWindowContentRegionMin().x);

		bool isColor = settingsButtonsParams[i].text.find("Color") != std::string::npos;
		bool& current = settingsButtonsParams[i].parameter;
		const std::string& label = settingsButtonsParams[i].text;

		ImVec4& col = current ? buttonEnabledColor : buttonDisabledColor;
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

	/*ImGui::SliderFloat("Slider 1", &value1, 0.0f, 1.0f);
	if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
		value1 = default1;
	if (ImGui::IsItemHovered()) {
		ImGui::SetTooltip("Right-click to reset");
	}*/


	ImGui::End();

	rlImGuiEnd();

	if (showSettings) {

		if (bVisualsSliders) {

			visualsSliders[0].sliderPos = { 20.0f, 40.0f };
			for (size_t i = 1; i < visualsSliders.size(); ++i) {
				visualsSliders[i].sliderPos.x = visualsSliders[i - 1].sliderPos.x;
				visualsSliders[i].sliderPos.y = visualsSliders[i - 1].sliderPos.y + visualsSliders[i].sliderSize.y + 32;
				visualsSliders[i].sliderSize = visualsSliders[i - 1].sliderSize;

				if (GetScreenWidth() >= 1920 || GetScreenHeight() >= 1080) {

					Vector2 sliderSize = { 230.0f, 7.0f };

					visualsSliders[i].sliderPos.x = visualsSliders[i - 1].sliderPos.x;
					visualsSliders[i].sliderPos.y = visualsSliders[i - 1].sliderPos.y + visualsSliders[i].sliderSize.y + 32;
					visualsSliders[i].sliderSize = visualsSliders[i - 1].sliderSize;
				}
				else if (GetScreenWidth() < 1920 || GetScreenHeight() < 1080) {

					Vector2 sliderSize = { 200.0f, 5.0f };

					visualsSliders[i].sliderPos.x = visualsSliders[i - 1].sliderPos.x;
					visualsSliders[i].sliderPos.y = visualsSliders[i - 1].sliderPos.y + visualsSliders[i].sliderSize.y + 26;
					visualsSliders[i].sliderSize = visualsSliders[i - 1].sliderSize;
				}
			}
			bool sliderRed1Hovering = visualsSliders[0].sliderLogic(0, myParam.colorVisuals.primaryR, 255);
			bool sliderGreen1Hovering = visualsSliders[1].sliderLogic(0, myParam.colorVisuals.primaryG, 255);
			bool sliderBlue1Hovering = visualsSliders[2].sliderLogic(0, myParam.colorVisuals.primaryB, 255);
			bool sliderAlpha1Hovering = visualsSliders[3].sliderLogic(0, myParam.colorVisuals.primaryA, 255);

			bool sliderRed2Hovering = visualsSliders[4].sliderLogic(0, myParam.colorVisuals.secondaryR, 255);
			bool sliderGreen2Hovering = visualsSliders[5].sliderLogic(0, myParam.colorVisuals.secondaryG, 255);
			bool sliderBlue2Hovering = visualsSliders[6].sliderLogic(0, myParam.colorVisuals.secondaryB, 255);
			bool sliderAlpha2Hovering = visualsSliders[7].sliderLogic(0, myParam.colorVisuals.secondaryA, 255);

			bool sliderDensityHovering = visualsSliders[8].sliderLogic(0.0f, myParam.neighborSearch.densityRadius, 30.0f);
			bool sliderMaxNeighborsHovering = visualsSliders[9].sliderLogic(1, myParam.colorVisuals.maxNeighbors, 500);

			bool sliderMaxColorForceHovering = visualsSliders[10].sliderLogic(1.0f, myParam.colorVisuals.maxColorAcc, 400.0f);
			bool sliderMaxSizeForceHovering = visualsSliders[11].sliderLogic(1.0f, myParam.densitySize.sizeAcc, 400.0f);

			bool sliderMaxDeltaVAccelHovering = visualsSliders[12].sliderLogic(1.0f, myParam.colorVisuals.deltaVMaxAccel, 40.0f);

			bool sliderTrailsLengthHovering = visualsSliders[13].sliderLogic(0, myVar.trailMaxLength, 1500);
			bool sliderTrailsThicknessHovering = visualsSliders[14].sliderLogic(0.007f, myParam.trails.trailThickness, 0.5f);

			bool sliderParticlesSizeHovering = visualsSliders[15].sliderLogic(0.1f, myVar.particleSizeMultiplier, 5.0f);

			bool sliderPathPredictionLengthHovering = visualsSliders[16].sliderLogic(100, myParam.particlesSpawning.predictPathLength, 2000);

			bool sliderParticleAmountMultHovering = visualsSliders[17].sliderLogic(0.1f, myParam.particlesSpawning.particleAmountMultiplier, 100.0f);
			bool sliderDMAmountMultHovering = visualsSliders[18].sliderLogic(0.1f, myParam.particlesSpawning.DMAmountMultiplier, 100.0f);

			if (
				sliderRed1Hovering ||
				sliderGreen1Hovering ||
				sliderBlue1Hovering ||
				sliderAlpha1Hovering ||
				sliderDensityHovering ||
				sliderMaxNeighborsHovering ||
				sliderTrailsLengthHovering ||
				sliderParticlesSizeHovering ||
				sliderTrailsThicknessHovering ||
				sliderRed2Hovering ||
				sliderGreen2Hovering ||
				sliderBlue2Hovering ||
				sliderAlpha2Hovering ||
				sliderMaxColorForceHovering ||
				sliderMaxSizeForceHovering ||
				sliderPathPredictionLengthHovering ||
				sliderParticleAmountMultHovering ||
				sliderDMAmountMultHovering ||
				sliderMaxDeltaVAccelHovering
				) {

			}
		}
		if (bPhysicsSliders || loadSettings) { // I use the loadSettings flag the first time the program is open to save default values

			float sliderPosYBias = 32.0f;
			physicsSliders[0].sliderPos = { 20.0f, 40.0f };
			for (size_t i = 1; i < physicsSliders.size(); ++i) {
				physicsSliders[i].sliderPos.x = physicsSliders[i - 1].sliderPos.x;
				physicsSliders[i].sliderPos.y = physicsSliders[i - 1].sliderPos.y + physicsSliders[i].sliderSize.y + sliderPosYBias;
				physicsSliders[i].sliderSize = sliderSize;
			}

			bool sliderSofteningHovering = physicsSliders[0].sliderLogic(1.0f, myVar.softening, 30.0f);
			bool sliderThetaHovering = physicsSliders[1].sliderLogic(0.1f, myVar.theta, 5.0f);
			bool sliderTimeScaleHovering = physicsSliders[2].sliderLogic(0.0f, myVar.timeStepMultiplier, 15.0f);
			bool sliderSubstepsHovering = physicsSliders[3].sliderLogic(1, myVar.substeps, 64);

			bool sliderGravityStrengthHovering = physicsSliders[4].sliderLogic(0.0f, myVar.gravityMultiplier, 100.0f);
			bool sliderHeavyParticleMassHovering = physicsSliders[5].sliderLogic(0.005f, myParam.particlesSpawning.heavyParticleWeightMultiplier, 15.0f);
			bool sliderParticlesBouncinessHovering = physicsSliders[6].sliderLogic(0.0f, myVar.particleBounciness, 1.0f);

			bool sliderDomainWidthHovering = physicsSliders[7].sliderLogic(200.0f, myVar.domainSize.x, 3840.0f);
			bool sliderDomainHeightHovering = physicsSliders[8].sliderLogic(200.0f, myVar.domainSize.y, 2160.0f);

			bool sliderThreadsAmountHovering = physicsSliders[9].sliderLogic(1, myVar.threadsAmount, 32);

			bool sliderSPHRestPressureHovering = physicsSliders[10].sliderLogic(0.01f, sph.restDensity, 5.0f);
			bool sliderSPHStiffnessHovering = physicsSliders[11].sliderLogic(0.0f, sph.stiffness, 300.0f);
			bool sliderSPHRadiusHovering = physicsSliders[12].sliderLogic(0.1f, sph.radiusMultiplier, 4.0f);
			bool sliderSPHMassHovering = physicsSliders[13].sliderLogic(0.05f, sph.mass, 0.15f);
			bool sliderSPHViscosityHovering = physicsSliders[14].sliderLogic(1.0f, sph.viscosity, 250.0f);
			bool sliderSPHCohesionHovering = physicsSliders[15].sliderLogic(0.0f, sph.cohesionCoefficient, 10.0f);

			if (
				sliderSofteningHovering ||
				sliderThetaHovering ||
				sliderTimeScaleHovering ||
				sliderGravityStrengthHovering ||
				sliderHeavyParticleMassHovering ||
				sliderSubstepsHovering ||
				sliderParticlesBouncinessHovering ||
				sliderThreadsAmountHovering ||
				sliderSPHRestPressureHovering ||
				sliderSPHStiffnessHovering ||
				sliderSPHRadiusHovering ||
				sliderSPHMassHovering ||
				sliderSPHViscosityHovering ||
				sliderSPHCohesionHovering ||
				sliderDomainWidthHovering ||
				sliderDomainHeightHovering
				) {

			}

			SPHMaterialButtonsArray[0].pos.x = 20.0f;
			SPHMaterialButtonsArray[0].pos.y = physicsSliders.size() * (physicsSliders[0].sliderSize.y + sliderPosYBias) + 19.0f;

			for (size_t i = 1; i < SPHMaterialButtonsArray.size(); ++i) {
				if (GetScreenWidth() >= 1920 || GetScreenHeight() >= 1080) {

					Vector2 sphButtonSize = { 175.0f, 26.0f };

					SPHMaterialButtonsArray[0].size = sphButtonSize;

					SPHMaterialButtonsArray[i].pos.x = SPHMaterialButtonsArray[i - 1].pos.x;
					SPHMaterialButtonsArray[i].pos.y = SPHMaterialButtonsArray[i - 1].pos.y + sphButtonSize.y + 5.0f;
					SPHMaterialButtonsArray[i].size = sphButtonSize;
				}
				else if (GetScreenWidth() < 1920 || GetScreenHeight() < 1080) {

					Vector2 sphButtonSize = { 175.0f, 20.0f };

					SPHMaterialButtonsArray[0].size = sphButtonSize;

					SPHMaterialButtonsArray[i].pos.x = SPHMaterialButtonsArray[i - 1].pos.x;
					SPHMaterialButtonsArray[i].pos.y = SPHMaterialButtonsArray[i - 1].pos.y + sphButtonSize.y + 2.0f;
					SPHMaterialButtonsArray[i].size = sphButtonSize;
				}
			}

			bool buttonWaterHovering = SPHMaterialButtonsArray[0].buttonLogic(myParam.brush.SPHWater);
			bool buttonRockHovering = SPHMaterialButtonsArray[1].buttonLogic(myParam.brush.SPHRock);
			bool buttonSandHovering = SPHMaterialButtonsArray[2].buttonLogic(myParam.brush.SPHSand);
			bool buttonMudHovering = SPHMaterialButtonsArray[3].buttonLogic(myParam.brush.SPHMud);

			if (
				buttonWaterHovering ||
				buttonRockHovering ||
				buttonSandHovering ||
				buttonMudHovering
				) {

			}

			if (buttonWaterHovering && IsMouseButtonPressed(0)) {
				myParam.brush.SPHRock = false;
				myParam.brush.SPHSand = false;
				myParam.brush.SPHMud = false;
			}

			if (buttonRockHovering && IsMouseButtonPressed(0)) {
				myParam.brush.SPHWater = false;
				myParam.brush.SPHSand = false;
				myParam.brush.SPHMud = false;
			}

			if (buttonSandHovering && IsMouseButtonPressed(0)) {
				myParam.brush.SPHRock = false;
				myParam.brush.SPHWater = false;
				myParam.brush.SPHMud = false;
			}

			if (buttonMudHovering && IsMouseButtonPressed(0)) {
				myParam.brush.SPHRock = false;
				myParam.brush.SPHSand = false;
				myParam.brush.SPHWater = false;
			}

			loadSettings = false;
		}

		myParam.controls.showControls(myVar.isMouseNotHoveringUI, myVar.isDragging);
		myParam.controls.showMoreInfo();
	}

	myParam.rightClickSettings.rightClickMenu(myVar, myParam);

	DrawText(TextFormat("Particles: %i", myParam.pParticles.size()), 400, 50, 25, WHITE);
	if (myParam.pParticlesSelected.size() > 0) {
		DrawText(TextFormat("Selected Particles: %i", myParam.pParticlesSelected.size()), 700, 50, 25, WHITE);
	}

	if (GetFPS() >= 60) {
		DrawText(TextFormat("FPS: %i", GetFPS()), GetScreenWidth() - 150, 50, 18, GREEN);

	}
	else if (GetFPS() < 60 && GetFPS() > 30) {
		DrawText(TextFormat("FPS: %i", GetFPS()), GetScreenWidth() - 150, 50, 18, YELLOW);
	}
	else {
		DrawText(TextFormat("FPS: %i", GetFPS()), GetScreenWidth() - 150, 50, 18, RED);
	}
}
