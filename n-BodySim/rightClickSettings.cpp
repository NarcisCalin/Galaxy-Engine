#include "rightClickSettings.h"
#include "parameters.h"

void RightClickSettings::rightClickMenuSpawnLogic(bool& isMouseNotHoveringUI) {

	static bool isMouseMoving = false;
	static Vector2 dragStartPos = { 0 };

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

		bool buttonSubdivideAllHovering = menuSettings[0].buttonLogic(myParam.subdivision.subdivideAll, myVar);
		bool buttonSubdivideSelectedHovering = menuSettings[1].buttonLogic(myParam.subdivision.subdivideSelected, myVar);
		bool buttonInvertSelectionHovering = menuSettings[2].buttonLogic(myParam.particleSelection.invertParticleSelection, myVar);
		bool buttonDeselectAllHovering = menuSettings[3].buttonLogic(myParam.particleSelection.deselectParticles, myVar);
		bool buttonFollowSelectionHovering = menuSettings[4].buttonLogic(myParam.myCamera.centerCamera, myVar);
		bool buttonClustersSelectionHovering = menuSettings[5].buttonLogic(myParam.particleSelection.selectManyClusters, myVar);
		bool buttonDeleteSelectionHovering = menuSettings[6].buttonLogic(myParam.particleDeletion.deleteSelection, myVar);
		bool buttonDeleteNonImportantParticlesHovering = menuSettings[7].buttonLogic(myParam.particleDeletion.deleteNonImportant, myVar);
		bool buttonDrawZCurvesHovering = menuSettings[8].buttonLogic(myVar.drawZCurves, myVar);
		bool buttonDrawQuadtreeHovering = menuSettings[9].buttonLogic(myVar.drawQuadtree, myVar);
		bool buttonStartRecordingHovering = menuSettings[10].buttonLogic(myParam.screenCapture.isFunctionRecording, myVar);


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
			buttonStartRecordingHovering
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
