#include "../../include/UI/rightClickSettings.h"
#include "../../include/parameters.h"

void RightClickSettings::rightClickMenuSpawnLogic(bool& isMouseNotHoveringUI) {

	static bool isMouseMoving = false;
	static Vector2 dragStartPos = { 0.0f, 0.0f };

	if (IsMouseButtonPressed(1) && isMouseNotHoveringUI) {
		dragStartPos = GetMousePosition();
		isMouseMoving = false;
	}

	if (IsMouseButtonDown(1) && isMouseNotHoveringUI) {
		Vector2 currentPos = GetMousePosition();
		float dragThreshold = 5.0f;
		float dx = currentPos.x - dragStartPos.x;
		float dy = currentPos.y - dragStartPos.y;

		if (dx * dx + dy * dy > dragThreshold * dragThreshold) {
			isMouseMoving = true;
		}
	}

	if (IsMouseButtonReleased(1) && (!IsKeyDown(KEY_LEFT_CONTROL) && !IsKeyDown(KEY_LEFT_ALT)) &&
		!isMouseMoving && isMouseNotHoveringUI && !IsMouseButtonDown(0)) {
		isMenuActive = true;

		menuSize.x = buttonSizeX + 5.0f;
		menuSize.y = (menuSettings.size() + 1) * (buttonSizeY + menuButtonGap);

		menuPos.x = std::clamp(static_cast<float>(GetMouseX()), 0.0f, static_cast<float>(GetScreenWidth()) - menuSize.x);
		menuPos.y = std::clamp(static_cast<float>(GetMouseY()), 0.0f, static_cast<float>(GetScreenHeight()) - menuSize.y);

		menuSettings[0].pos = menuPos;
	}
}


void RightClickSettings::rightClickMenu(UpdateVariables& myVar, UpdateParameters& myParam) {

	rightClickMenuSpawnLogic(myVar.isMouseNotHoveringUI);


	if (isMenuActive) {

		menuSettings[0].pos.x = menuPos.x + menuButtonGap;

		menuSettings[0].pos.y = menuPos.y + menuButtonGap;

		menuSettings[0].size = { buttonSizeX, buttonSizeY };


		for (size_t i = 1; i < menuSettings.size(); i++) {


			menuSettings[i].pos.x = menuSettings[i - 1].pos.x;
			menuSettings[i].pos.y = menuSettings[i - 1].pos.y + menuSettings[i].size.y + 5;
			menuSettings[i].size = menuSettings[i - 1].size;

		}


		DrawRectangleV(menuPos, menuSize, menuColor);

		bool buttonSubdivideAllHovering = menuSettings[0].buttonLogic(myParam.subdivision.subdivideAll);
		bool buttonSubdivideSelectedHovering = menuSettings[1].buttonLogic(myParam.subdivision.subdivideSelected);

		bool buttonInvertSelectionHovering = menuSettings[2].buttonLogic(myParam.particleSelection.invertParticleSelection);
		bool buttonDeselectAllHovering = menuSettings[3].buttonLogic(myParam.particleSelection.deselectParticles);

		bool buttonFollowSelectionHovering = menuSettings[4].buttonLogic(myParam.myCamera.centerCamera);

		bool buttonClustersSelectionHovering = menuSettings[5].buttonLogic(myParam.particleSelection.selectManyClusters);

		bool buttonDeleteSelectionHovering = menuSettings[6].buttonLogic(myParam.particleDeletion.deleteSelection);
		bool buttonDeleteNonImportantParticlesHovering = menuSettings[7].buttonLogic(myParam.particleDeletion.deleteNonImportant);

		bool buttonDrawZCurvesHovering = menuSettings[8].buttonLogic(myVar.drawZCurves);
		bool buttonDrawQuadtreeHovering = menuSettings[9].buttonLogic(myVar.drawQuadtree);

		bool buttonRecordDiskModeHovering = false;
		if (!myParam.screenCapture.isFunctionRecording) { // If it is recording, lock the setting
			buttonRecordDiskModeHovering = menuSettings[10].buttonLogic(myParam.screenCapture.isDiskModeEnabled);
		}


		if (buttonSubdivideAllHovering ||
			buttonSubdivideSelectedHovering ||
			buttonInvertSelectionHovering ||
			buttonFollowSelectionHovering ||
			buttonDeleteSelectionHovering ||
			buttonClustersSelectionHovering ||
			buttonDeleteNonImportantParticlesHovering ||
			buttonDrawZCurvesHovering ||
			buttonDrawQuadtreeHovering ||
			buttonDeselectAllHovering ||
			buttonRecordDiskModeHovering
			) {
			myVar.isMouseNotHoveringUI = false;
			myVar.isDragging = false;
		}

		if (IsMouseButtonPressed(0)) {
			myVar.isMouseNotHoveringUI = false;
			myVar.isDragging = false;
			isMenuActive = false;
		}
	}
}
