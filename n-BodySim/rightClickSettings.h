#pragma once
#include <vector>
#include <array>
#include "raylib.h"
#include "button.h"
#include "particle.h"
#include "particleSubdivision.h"
#include "particleSelection.h"
#include "camera.h"
#include "particleDeletion.h"
#include <algorithm>

class RightClickSettings {
public:
	Vector2 menuPos = {0.0f};
	Vector2 menuSize = { 0.0f };

	void rightClickMenuSpawnLogic(bool& isMouseNotHoveringUI) {

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

		if (IsMouseButtonReleased(1) && (!IsKeyDown(KEY_LEFT_CONTROL) && !IsKeyDown(KEY_LEFT_ALT)) && !isMouseMoving && isMouseNotHoveringUI) {
			isMenuActive = true;

			menuSize.x = buttonSizeX + 10.0f;
			menuSize.y = (menuSettings.size() + 1) * (buttonSizeY + menuButtonGap);

			menuPos.x = std::clamp(static_cast<float>(GetMouseX()), 0.0f, static_cast<float>(GetScreenWidth()) - menuSize.x);
			menuPos.y = std::clamp(static_cast<float>(GetMouseY()), 0.0f, static_cast<float>(GetScreenHeight()) - menuSize.y);

			menuSettings[0].pos = menuPos;
		}
	}

	void rightClickMenu(bool& isMouseNotHoveringUI, bool& isDragging, ParticleSubdivision& subdivision, ParticleSelection& particleSelection,
		SceneCamera& myCamera, ParticleDeletion& particleDeletion, bool& drawZCurves, bool& drawQuadtree, bool& isRecording) {

		rightClickMenuSpawnLogic(isMouseNotHoveringUI);


		if (isMenuActive) {

			menuSettings[0].pos.x = menuPos.x + menuButtonGap;

			menuSettings[0].pos.y = menuPos.y + menuButtonGap;

			menuSettings[0].size = { buttonSizeX, buttonSizeY };


			for (size_t i = 1; i < menuSettings.size(); i++) {


				menuSettings[i].pos.x = menuSettings[i - 1].pos.x;
				menuSettings[i].pos.y = menuSettings[i - 1].pos.y + menuSettings[i].size.y + 10;
				menuSettings[i].size = menuSettings[i - 1].size;

			}


			DrawRectangleV(menuPos, menuSize, menuColor);

			bool buttonSubdivideAllHovering = menuSettings[0].buttonLogic(subdivision.subdivideAll);
			bool buttonSubdivideSelectedHovering = menuSettings[1].buttonLogic(subdivision.subdivideSelected);
			bool buttonInvertSelectionHovering = menuSettings[2].buttonLogic(particleSelection.invertParticleSelection);
			bool buttonDeselectAllHovering = menuSettings[3].buttonLogic(particleSelection.deselectParticles);
			bool buttonFollowSelectionHovering = menuSettings[4].buttonLogic(myCamera.centerCamera);
			bool buttonClustersSelectionHovering = menuSettings[5].buttonLogic(particleSelection.selectManyClusters);
			bool buttonDeleteSelectionHovering = menuSettings[6].buttonLogic(particleDeletion.deleteSelection);
			bool buttonDeleteNonImportantParticlesHovering = menuSettings[7].buttonLogic(particleDeletion.deleteNonImportant);
			bool buttonDrawZCurvesHovering = menuSettings[8].buttonLogic(drawZCurves);
			bool buttonDrawQuadtreeHovering = menuSettings[9].buttonLogic(drawQuadtree);
			bool buttonStartRecordingHovering = menuSettings[10].buttonLogic(isRecording);

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
				isMouseNotHoveringUI = false;
				isDragging = false;
			}

			if (IsMouseButtonPressed(0)) {
				isMouseNotHoveringUI = false;
				isDragging = false;
				isMenuActive = false;
			}
		}

	}

private:
	bool isMenuActive = false;

	Color menuColor = { 30,30,30,255 };

	float buttonSizeX = 175.0f;
	float buttonSizeY = 35.0f;

	float menuButtonGap = 5.5f;

	std::array<Button, 11> menuSettings = {

Button({0.0f}, {0.0f}, "Subdivide All", true),

Button({0.0f}, {0.0f}, "Subdivide Selec.", true),

Button({0.0f}, {0.0f}, "Invert Selection", true),

Button({0.0f}, {0.0f}, "Deselect All", true),

Button({0.0f}, {0.0f}, "Follow Selection", true),

Button({0.0f}, {0.0f}, "Select Clusters", true),

Button({0.0f}, {0.0f}, "Delete Selection", true),

Button({0.0f}, {0.0f}, "Delete Strays", true),

Button({0.0f}, {0.0f}, "Debug Z Curves", true),

Button({0.0f}, {0.0f}, "Debug Quadtree", true),

Button({0.0f}, {0.0f}, "Start Recording", true)


	};
};