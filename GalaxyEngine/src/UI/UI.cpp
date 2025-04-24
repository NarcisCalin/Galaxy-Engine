#include "../../include/UI/UI.h"


void UI::uiLogic(UpdateParameters& myParam, UpdateVariables& myVar) {

	toggleSettingsButtons[0].pos = { static_cast<float>(myVar.screenWidth) - 34.0f, 65.0f };
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

	if (myVar.timeStepMultiplier == 0.0f) {
		DrawRectangleV({ myVar.screenWidth - 200.0f, 20.0f }, { 10.0f, 30.0f }, WHITE);
		DrawRectangleV({ myVar.screenWidth - 220.0f, 20.0f }, { 10.0f, 30.0f }, WHITE);
	}


	if (showSettings) {

		settingsButtonsArray[0].pos = { static_cast<float>(myVar.screenWidth) - 195.0f, 80.0f };
		for (size_t i = 1; i < settingsButtonsArray.size(); ++i) {
			settingsButtonsArray[i].pos.x = settingsButtonsArray[i - 1].pos.x;
			settingsButtonsArray[i].pos.y = settingsButtonsArray[i - 1].pos.y + settingsButtonsArray[i].size.y + 10;
			settingsButtonsArray[i].size = settingsButtonsArray[i - 1].size;

		}

		bool buttonGlobalTrailsHovering = settingsButtonsArray[0].buttonLogic(myVar.isGlobalTrailsEnabled);
		bool buttonSelectedTrailsHovering = settingsButtonsArray[1].buttonLogic(myVar.isSelectedTrailsEnabled);
		bool buttonLocalTrailsHovering = settingsButtonsArray[2].buttonLogic(myVar.isLocalTrailsEnabled);
		bool buttonWhiteTrailsHovering = settingsButtonsArray[3].buttonLogic(myParam.trails.whiteTrails);

		bool buttonSolidColorHovering = settingsButtonsArray[4].buttonLogic(myParam.colorVisuals.solidColor);
		bool buttonDensityColorHovering = settingsButtonsArray[5].buttonLogic(myParam.colorVisuals.densityColor);
		bool buttonForceColorHovering = settingsButtonsArray[6].buttonLogic(myParam.colorVisuals.forceColor);
		bool buttonVelocityColorHovering = settingsButtonsArray[7].buttonLogic(myParam.colorVisuals.velocityColor);
		bool buttonDeltaVColorHovering = settingsButtonsArray[8].buttonLogic(myParam.colorVisuals.deltaVColor);
		bool buttonSelectedColorHovering = settingsButtonsArray[9].buttonLogic(myParam.colorVisuals.selectedColor);

		bool buttonDarkMatterHovering = settingsButtonsArray[10].buttonLogic(myVar.isDarkMatterEnabled);
		bool buttonShowDarkMatterHovering = settingsButtonsArray[11].buttonLogic(myParam.colorVisuals.showDarkMatterEnabled);

		bool buttonPeriodicBoundaryHovering = settingsButtonsArray[12].buttonLogic(myVar.isPeriodicBoundaryEnabled);
		bool buttonBarnesHutHovering = settingsButtonsArray[13].buttonLogic(myVar.isBarnesHutEnabled);
		bool buttonMultiThreadingHovering = settingsButtonsArray[14].buttonLogic(myVar.isMultiThreadingEnabled);
		bool buttonCollisionsHovering = settingsButtonsArray[15].buttonLogic(myVar.isCollisionsEnabled);

		bool buttonDensitySizeHovering = settingsButtonsArray[16].buttonLogic(myVar.isDensitySizeEnabled);
		bool buttonForceSizeHovering = settingsButtonsArray[17].buttonLogic(myVar.isForceSizeEnabled);

		bool buttonGlowHovering = settingsButtonsArray[18].buttonLogic(myVar.isGlowEnabled);

		bool buttonPredictPathsHovering = settingsButtonsArray[19].buttonLogic(myParam.particlesSpawning.enablePathPrediction);

		bool buttonControlsHovering = settingsButtonsArray[20].buttonLogic(myParam.controls.isShowControlsEnabled);

		bool buttonInformationHovering = settingsButtonsArray[21].buttonLogic(myVar.isInformationEnabled);

		bool buttonVisualsSlidersHovering = settingsButtonsArray[22].buttonLogic(bVisualsSliders);
		bool buttonPhysicsSlidersHovering = settingsButtonsArray[23].buttonLogic(bPhysicsSliders);

		if (buttonDarkMatterHovering ||
			buttonPeriodicBoundaryHovering ||
			buttonGlobalTrailsHovering ||
			buttonBarnesHutHovering ||
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
			buttonDeltaVColorHovering
			) {
			myVar.isMouseNotHoveringUI = false;
			myVar.isDragging = false;
		}
		else {
			myVar.isMouseNotHoveringUI = true;
		}


		if (bVisualsSliders) {

			visualsSliders[0].sliderPos = { 20.0f, 210.0f };
			for (size_t i = 1; i < visualsSliders.size(); ++i) {
				visualsSliders[i].sliderPos.x = visualsSliders[i - 1].sliderPos.x;
				visualsSliders[i].sliderPos.y = visualsSliders[i - 1].sliderPos.y + visualsSliders[i].sliderSize.y + 32;
				visualsSliders[i].sliderSize = visualsSliders[i - 1].sliderSize;
			}
			bool sliderRed1Hovering = visualsSliders[0].sliderLogic(0, myParam.colorVisuals.primaryR, 255);
			bool sliderGreen1Hovering = visualsSliders[1].sliderLogic(0, myParam.colorVisuals.primaryG, 255);
			bool sliderBlue1Hovering = visualsSliders[2].sliderLogic(0, myParam.colorVisuals.primaryB, 255);
			bool sliderAlpha1Hovering = visualsSliders[3].sliderLogic(0, myParam.colorVisuals.primaryA, 255);

			bool sliderRed2Hovering = visualsSliders[4].sliderLogic(0, myParam.colorVisuals.secondaryR, 255);
			bool sliderGreen2Hovering = visualsSliders[5].sliderLogic(0, myParam.colorVisuals.secondaryG, 255);
			bool sliderBlue2Hovering = visualsSliders[6].sliderLogic(0, myParam.colorVisuals.secondaryB, 255);
			bool sliderAlpha2Hovering = visualsSliders[7].sliderLogic(0, myParam.colorVisuals.secondaryA, 255);

			bool sliderDensityHovering = visualsSliders[8].sliderLogic(0.0f, myParam.colorVisuals.densityRadius, 30.0f);
			bool sliderMaxNeighborsHovering = visualsSliders[9].sliderLogic(1, myParam.colorVisuals.maxNeighbors, 300);

			bool sliderMaxColorForceHovering = visualsSliders[10].sliderLogic(1.0f, myParam.colorVisuals.maxColorAcc, 400.0f);
			bool sliderMaxSizeForceHovering = visualsSliders[11].sliderLogic(1.0f, myParam.densitySize.sizeAcc, 400.0f);

			bool sliderTrailsLengthHovering = visualsSliders[12].sliderLogic(0, myVar.trailMaxLength, 1500);
			bool sliderTrailsThicknessHovering = visualsSliders[13].sliderLogic(0.007f, myParam.trails.trailThickness, 0.5f);

			bool sliderParticlesSizeHovering = visualsSliders[14].sliderLogic(0.1f, myVar.particleSizeMultiplier, 5.0f);

			bool sliderPathPredictionLengthHovering = visualsSliders[15].sliderLogic(100, myParam.particlesSpawning.predictPathLength, 2000);

			bool sliderParticleAmountMultHovering = visualsSliders[16].sliderLogic(0.1f, myParam.particlesSpawning.particleAmountMultiplier, 100.0f);
			bool sliderDMAmountMultHovering = visualsSliders[17].sliderLogic(0.1f, myParam.particlesSpawning.DMAmountMultiplier, 100.0f);

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
				sliderDMAmountMultHovering
				) {
				myVar.isMouseNotHoveringUI = false;
				myVar.isDragging = false;
			}
			else {
				myVar.isMouseNotHoveringUI = true;
			}
		}
		else if (bPhysicsSliders) {

			physicsSliders[0].sliderPos = { 20.0f, 210.0f };
			for (size_t i = 1; i < physicsSliders.size(); ++i) {
				physicsSliders[i].sliderPos.x = physicsSliders[i - 1].sliderPos.x;
				physicsSliders[i].sliderPos.y = physicsSliders[i - 1].sliderPos.y + physicsSliders[i].sliderSize.y + 32;
				physicsSliders[i].sliderSize = physicsSliders[i - 1].sliderSize;
			}

			bool sliderSofteningHovering = physicsSliders[0].sliderLogic(1.0f, myVar.softening, 30.0f);
			bool sliderThetaHovering = physicsSliders[1].sliderLogic(0.1f, myVar.theta, 5.0f);
			bool sliderTimeScaleHovering = physicsSliders[2].sliderLogic(0.0f, myVar.timeStepMultiplierSlider, 15.0f);
			bool sliderSubstepsHovering = physicsSliders[3].sliderLogic(1, myVar.substeps, 64);

			bool sliderGravityStrengthHovering = physicsSliders[4].sliderLogic(0.0f, myVar.gravityMultiplier, 100.0f);
			bool sliderHeavyParticleMassHovering = physicsSliders[5].sliderLogic(0.005f, myParam.particlesSpawning.heavyParticleWeightMultiplier, 15.0f);
			bool sliderParticlesBouncinessHovering = physicsSliders[6].sliderLogic(0.0f, myVar.particleBounciness, 1.0f);

			bool sliderThreadsAmountHovering = physicsSliders[7].sliderLogic(1, myVar.threadsAmount, 32);

			if (
				sliderSofteningHovering ||
				sliderThetaHovering ||
				sliderTimeScaleHovering ||
				sliderGravityStrengthHovering ||
				sliderHeavyParticleMassHovering ||
				sliderSubstepsHovering ||
				sliderParticlesBouncinessHovering ||
				sliderThreadsAmountHovering
				) {
				myVar.isMouseNotHoveringUI = false;
				myVar.isDragging = false;
			}
			else {
				myVar.isMouseNotHoveringUI = true;
			}
		}

		if (buttonSolidColorHovering && IsMouseButtonPressed(0)) {
			myParam.colorVisuals.velocityColor = false;
			myParam.colorVisuals.densityColor = false;
			myParam.colorVisuals.forceColor = false;
			myParam.colorVisuals.deltaVColor = false;
		}
		if (buttonDensityColorHovering && IsMouseButtonPressed(0)) {
			myParam.colorVisuals.velocityColor = false;
			myParam.colorVisuals.solidColor = false;
			myParam.colorVisuals.forceColor = false;
			myParam.colorVisuals.deltaVColor = false;
		}
		if (buttonVelocityColorHovering && IsMouseButtonPressed(0)) {
			myParam.colorVisuals.densityColor = false;
			myParam.colorVisuals.solidColor = false;
			myParam.colorVisuals.forceColor = false;
			myParam.colorVisuals.deltaVColor = false;
		}
		if (buttonForceColorHovering && IsMouseButtonPressed(0)) {
			myParam.colorVisuals.densityColor = false;
			myParam.colorVisuals.solidColor = false;
			myParam.colorVisuals.velocityColor = false;
			myParam.colorVisuals.deltaVColor = false;
		}
		if (buttonDeltaVColorHovering && IsMouseButtonPressed(0)) {
			myParam.colorVisuals.densityColor = false;
			myParam.colorVisuals.solidColor = false;
			myParam.colorVisuals.velocityColor = false;
			myParam.colorVisuals.forceColor = false;
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

		myParam.controls.showControls(myVar);
		myParam.controls.showMoreInfo(myVar);
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
		DrawText(TextFormat("FPS: %i", GetFPS()), myVar.screenWidth - 150, 50, 18, GREEN);

	}
	else if (GetFPS() < 60 && GetFPS() > 30) {
		DrawText(TextFormat("FPS: %i", GetFPS()), myVar.screenWidth - 150, 50, 18, YELLOW);
	}
	else {
		DrawText(TextFormat("FPS: %i", GetFPS()), myVar.screenWidth - 150, 50, 18, RED);
	}
}
