#include "../../include/UI/UI.h"


void UI::uiLogic(UpdateParameters& myParam, UpdateVariables& myVar, SPH& sph) {

	toggleSettingsButtons[0].pos = { static_cast<float>(GetScreenWidth()) - 34.0f, 65.0f };
	bool buttonShowSettingsHovering = toggleSettingsButtons[0].buttonLogic(showSettings);

	if (buttonShowSettingsHovering) {
		myVar.isMouseNotHoveringUI = false;
		myVar.isDragging = false;
	}
	else {
		myVar.isMouseNotHoveringUI = true;
	}

	if (IsKeyPressed(KEY_U)) {
		showSettings = !showSettings;
	}

	DrawTriangle(
		{ toggleSettingsButtons[0].pos.x + 3.0f, toggleSettingsButtons[0].pos.y + 5.0f },
		{ toggleSettingsButtons[0].pos.x + 7.0f, toggleSettingsButtons[0].pos.y + 11.0f },
		{ toggleSettingsButtons[0].pos.x + 11.0f ,toggleSettingsButtons[0].pos.y + 5.0f }, WHITE);

	if (myVar.timeFactor == 0.0f) {
		DrawRectangleV({ GetScreenWidth() - 200.0f, 20.0f }, { 10.0f, 30.0f }, WHITE);
		DrawRectangleV({ GetScreenWidth() - 220.0f, 20.0f }, { 10.0f, 30.0f }, WHITE);
	}


	if (showSettings) {

		settingsButtonsArray[0].pos = { static_cast<float>(GetScreenWidth()) - 195.0f, 80.0f };

		for (size_t i = 1; i < settingsButtonsArray.size(); ++i) {
			if (GetScreenWidth() >= 1920 || GetScreenHeight() >= 1080) {

				Vector2 buttonSize = { 175.0f, 20.0f };

				settingsButtonsArray[i].pos.x = settingsButtonsArray[i - 1].pos.x;
				settingsButtonsArray[i].pos.y = settingsButtonsArray[i - 1].pos.y + buttonSize.y + 5.0f;
				settingsButtonsArray[i].size = buttonSize;
			}
			else if (GetScreenWidth() < 1920 || GetScreenHeight() < 1080) {

				Vector2 buttonSize = { 175.0f, 17.0f };

				settingsButtonsArray[i].pos.x = settingsButtonsArray[i - 1].pos.x;
				settingsButtonsArray[i].pos.y = settingsButtonsArray[i - 1].pos.y + buttonSize.y + 2.0f;
				settingsButtonsArray[i].size = buttonSize;
			}
		}

		bool buttonFullscreenHovering = settingsButtonsArray[0].buttonLogic(myVar.fullscreenState);
		bool buttonGlobalTrailsHovering = settingsButtonsArray[1].buttonLogic(myVar.isGlobalTrailsEnabled);
		bool buttonSelectedTrailsHovering = settingsButtonsArray[2].buttonLogic(myVar.isSelectedTrailsEnabled);
		bool buttonLocalTrailsHovering = settingsButtonsArray[3].buttonLogic(myVar.isLocalTrailsEnabled);
		bool buttonWhiteTrailsHovering = settingsButtonsArray[4].buttonLogic(myParam.trails.whiteTrails);

		bool buttonSolidColorHovering = settingsButtonsArray[5].buttonLogic(myParam.colorVisuals.solidColor);
		bool buttonDensityColorHovering = settingsButtonsArray[6].buttonLogic(myParam.colorVisuals.densityColor);
		bool buttonForceColorHovering = settingsButtonsArray[7].buttonLogic(myParam.colorVisuals.forceColor);
		bool buttonVelocityColorHovering = settingsButtonsArray[8].buttonLogic(myParam.colorVisuals.velocityColor);
		bool buttonDeltaVColorHovering = settingsButtonsArray[9].buttonLogic(myParam.colorVisuals.deltaVColor);
		bool buttonSPHColorHovering = settingsButtonsArray[10].buttonLogic(myParam.colorVisuals.SPHColor);
		bool buttonSelectedColorHovering = settingsButtonsArray[11].buttonLogic(myParam.colorVisuals.selectedColor);

		bool buttonDarkMatterHovering = settingsButtonsArray[12].buttonLogic(myVar.isDarkMatterEnabled);
		bool buttonShowDarkMatterHovering = settingsButtonsArray[13].buttonLogic(myParam.colorVisuals.showDarkMatterEnabled);

		bool buttonPeriodicBoundaryHovering = settingsButtonsArray[14].buttonLogic(myVar.isPeriodicBoundaryEnabled);

		bool buttonMultiThreadingHovering = settingsButtonsArray[15].buttonLogic(myVar.isMultiThreadingEnabled);

		bool buttonSPHHovering = settingsButtonsArray[16].buttonLogic(myVar.isSPHEnabled);
		bool buttonCollisionsHovering = settingsButtonsArray[17].buttonLogic(myVar.isCollisionsEnabled);

		bool buttonDensitySizeHovering = settingsButtonsArray[18].buttonLogic(myVar.isDensitySizeEnabled);
		bool buttonForceSizeHovering = settingsButtonsArray[19].buttonLogic(myVar.isForceSizeEnabled);

		bool buttonGlowHovering = settingsButtonsArray[20].buttonLogic(myVar.isGlowEnabled);

		bool buttonPredictPathsHovering = settingsButtonsArray[21].buttonLogic(myParam.particlesSpawning.enablePathPrediction);

		bool buttonControlsHovering = settingsButtonsArray[22].buttonLogic(myParam.controls.isShowControlsEnabled);
		bool buttonInformationHovering = settingsButtonsArray[23].buttonLogic(myParam.controls.isInformationEnabled);

		bool buttonVisualsSlidersHovering = settingsButtonsArray[24].buttonLogic(bVisualsSliders);
		bool buttonPhysicsSlidersHovering = settingsButtonsArray[25].buttonLogic(bPhysicsSliders);

		bool buttonShipGasHovering = settingsButtonsArray[26].buttonLogic(myVar.isShipGasEnabled);

		if (buttonFullscreenHovering ||
			buttonDarkMatterHovering ||
			buttonPeriodicBoundaryHovering ||
			buttonGlobalTrailsHovering ||
			buttonMultiThreadingHovering ||
			buttonSolidColorHovering ||
			buttonDensityColorHovering ||
			buttonVelocityColorHovering ||
			buttonCollisionsHovering ||
			buttonControlsHovering ||
			buttonSelectedTrailsHovering ||
			buttonLocalTrailsHovering ||
			buttonDensitySizeHovering ||
			buttonSelectedColorHovering ||
			buttonWhiteTrailsHovering ||
			buttonForceColorHovering ||
			buttonGlowHovering ||
			buttonPredictPathsHovering ||
			buttonShowSettingsHovering ||
			buttonInformationHovering ||
			buttonForceSizeHovering ||
			buttonShowDarkMatterHovering ||
			buttonVisualsSlidersHovering ||
			buttonPhysicsSlidersHovering ||
			buttonDeltaVColorHovering ||
			buttonShipGasHovering ||
			buttonSPHHovering ||
			buttonSPHColorHovering
			) {
			myVar.isMouseNotHoveringUI = false;
			myVar.isDragging = false;
		}


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
				myVar.isMouseNotHoveringUI = false;
				myVar.isDragging = false;
			}
		}
		else if (bPhysicsSliders) {

			float sliderPosYBias = 32.0f;
			physicsSliders[0].sliderPos = { 20.0f, 40.0f };
			for (size_t i = 1; i < physicsSliders.size(); ++i) {
				physicsSliders[i].sliderPos.x = physicsSliders[i - 1].sliderPos.x;
				physicsSliders[i].sliderPos.y = physicsSliders[i - 1].sliderPos.y + physicsSliders[i].sliderSize.y + sliderPosYBias;
				physicsSliders[i].sliderSize = physicsSliders[i - 1].sliderSize;
			}

			bool sliderSofteningHovering = physicsSliders[0].sliderLogic(1.0f, myVar.softening, 30.0f);
			bool sliderThetaHovering = physicsSliders[1].sliderLogic(0.1f, myVar.theta, 5.0f);
			bool sliderTimeScaleHovering = physicsSliders[2].sliderLogic(0.0f, myVar.timeStepMultiplier, 15.0f);
			bool sliderSubstepsHovering = physicsSliders[3].sliderLogic(1, myVar.substeps, 64);

			bool sliderGravityStrengthHovering = physicsSliders[4].sliderLogic(0.0f, myVar.gravityMultiplier, 100.0f);
			bool sliderHeavyParticleMassHovering = physicsSliders[5].sliderLogic(0.005f, myParam.particlesSpawning.heavyParticleWeightMultiplier, 15.0f);
			bool sliderParticlesBouncinessHovering = physicsSliders[6].sliderLogic(0.0f, myVar.particleBounciness, 1.0f);

			bool sliderThreadsAmountHovering = physicsSliders[7].sliderLogic(1, myVar.threadsAmount, 32);

			bool sliderSPHRestPressureHovering = physicsSliders[8].sliderLogic(0.01f, sph.restDensity, 5.0f);
			bool sliderSPHStiffnessHovering = physicsSliders[9].sliderLogic(0.0f, sph.stiffness, 300.0f);
			bool sliderSPHRadiusHovering = physicsSliders[10].sliderLogic(0.1f, sph.radiusMultiplier, 4.0f);
			bool sliderSPHMassHovering = physicsSliders[11].sliderLogic(0.05f, sph.mass, 0.15f);
			bool sliderSPHViscosityHovering = physicsSliders[12].sliderLogic(1.0f, sph.viscosity, 250.0f);
			bool sliderSPHCohesionHovering = physicsSliders[13].sliderLogic(0.0f, sph.cohesionCoefficient, 10.0f);

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
				sliderSPHCohesionHovering
				) {
				myVar.isMouseNotHoveringUI = false;
				myVar.isDragging = false;
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
				myVar.isMouseNotHoveringUI = false;
				myVar.isDragging = false;
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
		}

		if (buttonSolidColorHovering && IsMouseButtonPressed(0)) {
			myParam.colorVisuals.velocityColor = false;
			myParam.colorVisuals.densityColor = false;
			myParam.colorVisuals.forceColor = false;
			myParam.colorVisuals.deltaVColor = false;
			myParam.colorVisuals.SPHColor = false;
		}
		if (buttonDensityColorHovering && IsMouseButtonPressed(0)) {
			myParam.colorVisuals.velocityColor = false;
			myParam.colorVisuals.solidColor = false;
			myParam.colorVisuals.forceColor = false;
			myParam.colorVisuals.deltaVColor = false;
			myParam.colorVisuals.SPHColor = false;
		}
		if (buttonVelocityColorHovering && IsMouseButtonPressed(0)) {
			myParam.colorVisuals.densityColor = false;
			myParam.colorVisuals.solidColor = false;
			myParam.colorVisuals.forceColor = false;
			myParam.colorVisuals.deltaVColor = false;
			myParam.colorVisuals.SPHColor = false;
		}
		if (buttonForceColorHovering && IsMouseButtonPressed(0)) {
			myParam.colorVisuals.densityColor = false;
			myParam.colorVisuals.solidColor = false;
			myParam.colorVisuals.velocityColor = false;
			myParam.colorVisuals.deltaVColor = false;
			myParam.colorVisuals.SPHColor = false;
		}
		if (buttonDeltaVColorHovering && IsMouseButtonPressed(0)) {
			myParam.colorVisuals.densityColor = false;
			myParam.colorVisuals.solidColor = false;
			myParam.colorVisuals.velocityColor = false;
			myParam.colorVisuals.forceColor = false;
			myParam.colorVisuals.SPHColor = false;
		}

		if (buttonSPHColorHovering && IsMouseButtonPressed(0)) {
			myParam.colorVisuals.densityColor = false;
			myParam.colorVisuals.solidColor = false;
			myParam.colorVisuals.velocityColor = false;
			myParam.colorVisuals.forceColor = false;
			myParam.colorVisuals.deltaVColor = false;
		}

		if (buttonGlobalTrailsHovering && IsMouseButtonPressed(0)) {
			myVar.isSelectedTrailsEnabled = false;
			myParam.trails.trailDots.clear();
		}
		if (buttonSelectedTrailsHovering && IsMouseButtonPressed(0)) {
			myVar.isGlobalTrailsEnabled = false;
			myParam.trails.trailDots.clear();
		}

		if (buttonDensitySizeHovering && IsMouseButtonPressed(0)) {
			myVar.isForceSizeEnabled = false;
		}
		if (buttonForceSizeHovering && IsMouseButtonPressed(0)) {
			myVar.isDensitySizeEnabled = false;
		}

		if (buttonVisualsSlidersHovering && IsMouseButtonPressed(0)) {
			bPhysicsSliders = false;
		}
		if (buttonPhysicsSlidersHovering && IsMouseButtonPressed(0)) {
			bVisualsSliders = false;
		}

		if (buttonControlsHovering && IsMouseButtonPressed(0)) {
			myParam.controls.isInformationEnabled = false;
		}
		if (buttonInformationHovering && IsMouseButtonPressed(0)) {
			myParam.controls.isShowControlsEnabled = false;
		}

		if (buttonSPHHovering && myVar.isSPHEnabled && IsMouseButtonPressed(0)) {
			myParam.colorVisuals.SPHColor = true;

			myParam.colorVisuals.densityColor = false;
			myParam.colorVisuals.solidColor = false;
			myParam.colorVisuals.velocityColor = false;
			myParam.colorVisuals.forceColor = false;
			myParam.colorVisuals.deltaVColor = false;
		}

		if (buttonCollisionsHovering && IsMouseButtonPressed(0)) {
			myVar.isSPHEnabled = false;
		}
		if (buttonSPHHovering && IsMouseButtonPressed(0)) {
			myVar.isCollisionsEnabled = false;
		}

		myParam.controls.showControls(myVar.isMouseNotHoveringUI, myVar.isDragging);
		myParam.controls.showMoreInfo();
	}
	else {

		if (buttonShowSettingsHovering) {
			myVar.isMouseNotHoveringUI = false;
		}
		else {
			myVar.isMouseNotHoveringUI = true;
		}
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
