#include "../../include/UI/rightClickSettings.h"
#include "../../include/parameters.h"

void RightClickSettings::rightClickMenuSpawnLogic(bool& isMouseNotHoveringUI) {

	static bool isMouseMoving = false;
	static Vector2 dragStartPos = { 0.0f, 0.0f };

	if (isMenuActive) {
		if (GetMousePosition().x > menuPos.x &&
			GetMousePosition().x < menuPos.x + menuSize.x &&
			GetMousePosition().y > menuPos.y &&
			GetMousePosition().y < menuPos.y + menuSize.y
			) {
			isMouseOnMenu = true;
		}
		else {
			isMouseOnMenu = false;
		}
	}
	else {
		isMouseOnMenu = false;
	}

	if (IsMouseButtonPressed(1) && isMouseNotHoveringUI && !isMouseOnMenu) {
		dragStartPos = GetMousePosition();
		isMouseMoving = false;
	}

	if (IsMouseButtonDown(1) && isMouseNotHoveringUI && !isMouseOnMenu) {
		Vector2 currentPos = GetMousePosition();
		float dragThreshold = 5.0f;
		float dx = currentPos.x - dragStartPos.x;
		float dy = currentPos.y - dragStartPos.y;

		if (dx * dx + dy * dy > dragThreshold * dragThreshold) {
			isMouseMoving = true;
		}
	}

	if (IsMouseButtonReleased(1) && (!IsKeyDown(KEY_LEFT_CONTROL) && !IsKeyDown(KEY_LEFT_ALT)) &&
		!isMouseMoving && isMouseNotHoveringUI && !IsMouseButtonDown(0) && !isMouseOnMenu) {
		isMenuActive = true;

		menuSize.x = buttonSizeX + 5.0f;
		menuSize.y = (menuBottons.size() + 1) * (buttonSizeY + menuButtonGap) +
			(menuSliders.size() + 1) * (sliderSizeY + menuSliderGap - (menuSliderGap / static_cast<float>(menuSliders.size())));

		menuPos.x = std::clamp(static_cast<float>(GetMouseX()), 0.0f, static_cast<float>(GetScreenWidth()) - menuSize.x);
		menuPos.y = std::clamp(static_cast<float>(GetMouseY()), 0.0f, static_cast<float>(GetScreenHeight()) - menuSize.y);

		menuBottons[0].pos = menuPos;
	}
}


void RightClickSettings::rightClickMenu(UpdateVariables& myVar, UpdateParameters& myParam) {

	rightClickMenuSpawnLogic(myVar.isMouseNotHoveringUI);


	if (isMenuActive) {

		menuBottons[0].pos.x = menuPos.x + menuButtonGap;

		menuBottons[0].pos.y = menuPos.y + menuButtonGap;

		menuBottons[0].size = { buttonSizeX, buttonSizeY };


		for (size_t i = 1; i < menuBottons.size(); i++) {

			menuBottons[i].pos.x = menuBottons[i - 1].pos.x;
			menuBottons[i].pos.y = menuBottons[i - 1].pos.y + menuBottons[i].size.y + 5.0f;
			menuBottons[i].size = menuBottons[i - 1].size;
		}


		DrawRectangleV(menuPos, menuSize, menuColor);

		bool buttonSubdivideAllHovering = menuBottons[0].buttonLogic(myParam.subdivision.subdivideAll);
		bool buttonSubdivideSelectedHovering = menuBottons[1].buttonLogic(myParam.subdivision.subdivideSelected);

		bool buttonInvertSelectionHovering = menuBottons[2].buttonLogic(myParam.particleSelection.invertParticleSelection);
		bool buttonDeselectAllHovering = menuBottons[3].buttonLogic(myParam.particleSelection.deselectParticles);

		bool buttonFollowSelectionHovering = menuBottons[4].buttonLogic(myParam.myCamera.centerCamera);

		bool buttonClustersSelectionHovering = menuBottons[5].buttonLogic(myParam.particleSelection.selectManyClusters);

		bool buttonDeleteSelectionHovering = menuBottons[6].buttonLogic(myParam.particleDeletion.deleteSelection);
		bool buttonDeleteStraysHovering = menuBottons[7].buttonLogic(myParam.particleDeletion.deleteNonImportant);

		bool buttonResetColorsHovering = menuBottons[8].buttonLogic(resetParticleColors);

		bool buttonDrawZCurvesHovering = menuBottons[9].buttonLogic(myVar.drawZCurves);
		bool buttonDrawQuadtreeHovering = menuBottons[10].buttonLogic(myVar.drawQuadtree);

		bool buttonRecordDiskModeHovering = false;
		if (!myParam.screenCapture.isFunctionRecording) { // If it is recording, lock the setting
			buttonRecordDiskModeHovering = menuBottons[11].buttonLogic(myParam.screenCapture.isDiskModeEnabled);
		}


		if (buttonSubdivideAllHovering ||
			buttonSubdivideSelectedHovering ||
			buttonInvertSelectionHovering ||
			buttonFollowSelectionHovering ||
			buttonDeleteSelectionHovering ||
			buttonClustersSelectionHovering ||
			buttonDeleteStraysHovering ||
			buttonResetColorsHovering ||
			buttonDrawZCurvesHovering ||
			buttonDrawQuadtreeHovering ||
			buttonDeselectAllHovering ||
			buttonRecordDiskModeHovering
			) {
			myVar.isMouseNotHoveringUI = false;
			myVar.isDragging = false;
		}

		menuSliders[0].sliderPos.x = menuPos.x + menuButtonGap;

		menuSliders[0].sliderPos.y = menuPos.y + (menuBottons.size() + 1) * (buttonSizeY + menuButtonGap) + menuSliderGap + 15.0f;

		menuSliders[0].sliderSize = { buttonSizeX, sliderSizeY };

		for (size_t i = 1; i < menuSliders.size(); i++) {

			menuSliders[i].sliderSize = menuSliders[i - 1].sliderSize;
			menuSliders[i].sliderPos.x = menuSliders[i - 1].sliderPos.x;
			menuSliders[i].sliderPos.y = menuSliders[i - 1].sliderPos.y + menuSliders[i].sliderSize.y + 25.0f;
		}

		bool sliderPRHovering = menuSliders[0].sliderLogic(0.0f, pR, 1.0f);
		bool sliderPGHovering = menuSliders[1].sliderLogic(0.0f, pG, 1.0f);
		bool sliderPBHovering = menuSliders[2].sliderLogic(0.0f, pB, 1.0f);
		bool sliderPAHovering = menuSliders[3].sliderLogic(0.0f, pA, 1.0f);

		bool sliderSRHovering = menuSliders[4].sliderLogic(0.0f, sR, 1.0f);
		bool sliderSGHovering = menuSliders[5].sliderLogic(0.0f, sG, 1.0f);
		bool sliderSBHovering = menuSliders[6].sliderLogic(0.0f, sB, 1.0f);
		bool sliderSAHovering = menuSliders[7].sliderLogic(0.0f, sA, 1.0f);

		// Color sliders only
		if (sliderPRHovering ||
			sliderPGHovering ||
			sliderPBHovering ||
			sliderPAHovering ||
			sliderSRHovering ||
			sliderSGHovering ||
			sliderSBHovering ||
			sliderSAHovering
			) {
			myVar.isMouseNotHoveringUI = false;
			myVar.isDragging = false;
			isMouseOnSlider = true;

			if (IsMouseButtonPressed(0)) {
				if (myParam.colorVisuals.selectedColor) {
					selectedColorChanged = true;
					myParam.colorVisuals.selectedColor = false;
				}
			}
		}
		else {
			isMouseOnSlider = false;
		}

		if (IsMouseButtonPressed(0) && !isMouseOnSlider) {
			myVar.isMouseNotHoveringUI = false;
			myVar.isDragging = false;
			isMenuActive = false;
			if (selectedColorChanged) {
				myParam.colorVisuals.selectedColor = true;
				selectedColorChanged = false;
			}
		}

		if (resetParticleColors) {
			for (size_t i = 0; i < myParam.rParticles.size(); i++) {
				myParam.rParticles[i].PRGBA = { 1.0f, 1.0f, 1.0f, 1.0f };
				myParam.rParticles[i].SRGBA = { 1.0f, 1.0f, 1.0f, 1.0f };
			}
			resetParticleColors = false;
		}

		if (myParam.rParticlesSelected.size() > 0 && !IsMouseButtonDown(0)) {

			pR = 0.0f;
			pG = 0.0f;
			pB = 0.0f;
			pA = 0.0f;

			sR = 0.0f;
			sG = 0.0f;
			sB = 0.0f;
			sA = 0.0f;

			int visibleSelectedAmount = 0;

			for (size_t i = 0; i < myParam.rParticles.size(); i++) {

				ParticleRendering& rP = myParam.rParticles[i];

				if (rP.isSelected && (!rP.isDarkMatter || !rP.uniqueColor)) {


					pR += rP.PRGBA.r;
					pG += rP.PRGBA.g;
					pB += rP.PRGBA.b;
					pA += rP.PRGBA.a;

					sR += rP.SRGBA.r;
					sG += rP.SRGBA.g;
					sB += rP.SRGBA.b;
					sA += rP.SRGBA.a;

					visibleSelectedAmount++;
				}
			}

			pR /= visibleSelectedAmount;
			pG /= visibleSelectedAmount;
			pB /= visibleSelectedAmount;
			pA /= visibleSelectedAmount;

			sR /= visibleSelectedAmount;
			sG /= visibleSelectedAmount;
			sB /= visibleSelectedAmount;
			sA /= visibleSelectedAmount;
		}

		if (IsMouseButtonDown(0) && myParam.rParticlesSelected.size() > 0 && isMenuActive) {
			for (size_t i = 0; i < myParam.rParticles.size(); i++) {

				ParticleRendering& rP = myParam.rParticles[i];
				if (rP.isSelected && (!rP.isDarkMatter || !rP.uniqueColor)) {

					rP.PRGBA.r = pR;
					rP.PRGBA.g = pG;
					rP.PRGBA.b = pB;
					rP.PRGBA.a = pA;

					rP.SRGBA.r = sR;
					rP.SRGBA.g = sG;
					rP.SRGBA.b = sB;
					rP.SRGBA.a = sA;
				}
			}

		}
	}
}
