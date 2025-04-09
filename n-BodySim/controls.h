#pragma once
#include "raylib.h"
#include "button.h"
#include <array>
#include <string>

struct Controls {
	std::array<std::string, 6> controlsArray = {
	"1. Hold LMB and Drag: Throw heavy particle",
	"2. Hold MMB: Paint particles",
	"3. Press 1: Create big galaxy",
	"4. Press 2: Create scattered particles",
	"5. Hold 3 and Drag: Create small galaxy",
	"6. U: Toggle UI",
	};

	std::array<std::string, 7> extendedControlsArrayParticles = {
	"PARTICLES CREATION",
	"1. Hold LMB and Drag: Throw heavy particle",
	"2. Hold MMB: Paint particles",
	"3. Press 1: Create big galaxy",
	"4. Press 2: Create scattered particles",
	"5. Hold 3 and Drag: Create small galaxy",
	"6. C: Clear all particles"
	};

	std::array<std::string, 20> extendedControlsArrayCamAndSelec = {
	"CAMERA AND SELECTION",
	"7. Move with RMB",
	"8. Zoom with mouse wheel",
	"9. LCTRL + RMB on cluster to follow it",
	"10. LALT + RMB on particle to follow it",
	"11. LCTRL + LMB on cluster to select it",
	"12. LATL + LMB on particle to select it",
	"13. LCTRL + hold and drag MMB to box select",
	"14. LALT + hold and drag MMB to box deselect",
	"15. Select on empty space to deselect all",
	"16. Hold SHIFT to add to selection",
	"17. I: Invert selection",
	"18. Z: Center camera on selected particles",
	"19. F: Reset camera "
	};

	std::array<std::string, 15> extendedControlsArrayUtility = {
	"UTILITY",
	"20. T: Toggle global trails",
	"21. LCTRL + T: Toggle local trails",
	"22. P: Toggle pixel drawing",
	"23. U: Toggle UI",
	"24. RMB on slider to set it to default",
	"25. Right click to open extra settings",
	"26. LCTRL + Scroll wheel : Brush size",
	"27. B: Brush attract particles",
	"28. N: Brush spin particles",
	"29. M: Brush grab particles",
	"30. Hold CTRL to invert brush effects",
	"31. R: Record frames",
	"32. S: Take screenshot",
	"33. X + MMB: Eraser"
	};

	bool isShowControlsEnabled = true;
	bool isShowExtendedControlsEnabled = false;
	bool nextPage = false;

	int controlsBoxSizeX = 435;
	int controlsBoxSizeY = 810;

	void showControls(bool& isMouseNotHoveringUI, bool& isDragging, int& screenWidth, int& screenHeight) {

		Button extendedControlsButton = { Vector2{ 20.0f, 140.0f }, Vector2{ 200.0f, 30.0f }, "Extended Controls", true };

		if (isShowControlsEnabled) {
			for (size_t i = 0; i < controlsArray.size(); i++) {
				DrawText(TextFormat("%s", controlsArray[i].c_str()), 25, 20 + 20 * static_cast<int>(i), 15, WHITE);
			}
			bool buttonExtendedControlsHovering = extendedControlsButton.buttonLogic(isShowExtendedControlsEnabled);

			if (buttonExtendedControlsHovering) {
				isMouseNotHoveringUI = false;
				isDragging = false;
			}

			if (isShowExtendedControlsEnabled) {
				Vector2 pageButtonPos = {
					static_cast<float>(screenWidth) / 2.0f,
					static_cast<float>(screenHeight) / 2.0f - controlsBoxSizeY / 2.0f - 14.0f };
				Button changePageButton(pageButtonPos,
					{ 14.0f, 14.0f }, "", false);

				bool buttonNextPageHovering = changePageButton.buttonLogic(nextPage);

				if (buttonNextPageHovering) {
					isMouseNotHoveringUI = false;
					isDragging = false;
				}

				DrawTriangle(
					{ pageButtonPos.x + 3.0f, pageButtonPos.y + 5.0f },
					{ pageButtonPos.x + 7.0f, pageButtonPos.y + 11.0f },
					{ pageButtonPos.x + 11.0f ,pageButtonPos.y + 5.0f }, WHITE);

				DrawRectangle(screenWidth / 2 - controlsBoxSizeX / 2,
					screenHeight / 2 - controlsBoxSizeY / 2,
					controlsBoxSizeX, controlsBoxSizeY,
					{ 170,170,170,170 });

				if (!nextPage) {
					DrawText("PAGE 1.", screenWidth / 2 - controlsBoxSizeX / 2 + 20, screenHeight / 2 - controlsBoxSizeY / 2 + 20, 30, WHITE);
					for (size_t i = 0; i < extendedControlsArrayParticles.size(); i++) {
						DrawText(TextFormat("%s", extendedControlsArrayParticles[i].c_str()),
							(screenWidth / 2 - controlsBoxSizeX / 2) + 20,
							((screenHeight / 2 - controlsBoxSizeY / 2) + 70) + 30 * static_cast<int>(i), 18, WHITE);
					}

					for (size_t i = 0; i < extendedControlsArrayCamAndSelec.size(); i++) {
						DrawText(TextFormat("%s", extendedControlsArrayCamAndSelec[i].c_str()),
							(screenWidth / 2 - controlsBoxSizeX / 2) + 20,
							((screenHeight / 2 - controlsBoxSizeY / 2) + 340) + 30 * static_cast<int>(i), 18, WHITE);
					}
				}
				else {
					DrawText("PAGE 2.", screenWidth / 2 - controlsBoxSizeX / 2 + 20, screenHeight / 2 - controlsBoxSizeY / 2 + 20, 30, WHITE);
					for (size_t i = 0; i < extendedControlsArrayUtility.size(); i++) {
						DrawText(TextFormat("%s", extendedControlsArrayUtility[i].c_str()),
							(screenWidth / 2 - controlsBoxSizeX / 2) + 20,
							((screenHeight / 2 - controlsBoxSizeY / 2) + 70) + 30 * static_cast<int>(i), 18, WHITE);
					}
				}

			}
		}
	}
};