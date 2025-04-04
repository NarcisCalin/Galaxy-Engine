#pragma once
#include <vector>
#include "raylib.h"
#include "button.h"
#include "planet.h"
#include "particleSubdivision.h"
#include "particleSelection.h"
#include "camera.h"



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
			menuPos = { GetMousePosition() };
			menuSettings[0].pos = menuPos;
		}
	}

	void rightClickMenu(bool& isMouseNotHoveringUI, bool& isDragging, ParticleSubdivision& subdivision, ParticleSelection& particleSelection,
		SceneCamera& myCamera) {

		menuSize = { buttonSizeX + 10.0f, buttonSizeY + 10.0f };

		rightClickMenuSpawnLogic(isMouseNotHoveringUI);


		if (isMenuActive) {

			menuButtonGap = (menuSize.x - buttonSizeX) / 2;


			menuSettings[0].pos = { menuPos.x + menuButtonGap ,
				menuPos.y + menuButtonGap };

			menuSettings[0].size = { buttonSizeX, buttonSizeY };

			for (size_t i = 1; i < menuSettings.size(); i++) {


				menuSettings[i].pos.x = menuSettings[i - 1].pos.x;
				menuSettings[i].pos.y = menuSettings[i - 1].pos.y + menuSettings[i].size.y + 10;
				menuSettings[i].size = menuSettings[i - 1].size;

			}

			menuSize.x = menuSettings[0].size.x + 10.0f;

			menuSize.y = menuSettings[menuSettings.size() - 1].pos.y - menuPos.y + menuSettings[0].size.y + menuButtonGap;


			DrawRectangleV(menuPos, menuSize, menuColor);

			bool buttonSubdivideAllHovering = menuSettings[0].buttonLogic(subdivision.subdivideAll);
			bool buttonSubdivideSelectedHovering = menuSettings[1].buttonLogic(subdivision.subdivideSelected);
			bool buttonInvertSelectionHovering = menuSettings[2].buttonLogic(particleSelection.invertParticleSelection);
			bool buttonFollowSelectionHovering = menuSettings[3].buttonLogic(myCamera.centerCamera);

			if (buttonSubdivideAllHovering ||
				buttonSubdivideSelectedHovering ||
				buttonInvertSelectionHovering ||
				buttonFollowSelectionHovering) {
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

	float menuButtonGap = 0.0f;

	std::array<Button, 4> menuSettings = {

Button({0.0f}, {0.0f}, "Subdivide All", true),

Button({0.0f}, {0.0f}, "Subdivide Selec.", true),

Button({0.0f}, {0.0f}, "Invert Selection", true),

Button({0.0f}, {0.0f}, "Follow Selection", true)

	};
};