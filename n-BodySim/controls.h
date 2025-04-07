#pragma once
#include "raylib.h"
#include "button.h"
#include <array>
#include <string>

struct Controls {
	std::array<std::string, 10> controlsArray = {
	"1. Hold LMB and Drag: Throw heavy particle",
	"2. Hold MMB: Paint particles",
	"3. Press 1: Create big galaxy",
	"4. Press 2: Create scattered particles",
	"5. Hold 3 and Drag: Create small galaxy",
	"6. T: Toggle global trails",
	"7. LCTRL + T: Toggle local trails",
	"8. P: Toggle pixel drawing",
	"9. C: Clear all particles",
	"10. U: Toggle UI",
	};

	std::array<std::string, 26> extendedControlsArray = {
	"1. Hold LMB and Drag: Throw heavy particle",
	"2. Hold MMB: Paint particles",
	"3. Press 1: Create big galaxy",
	"4. Press 2: Create scattered particles",
	"5. Hold 3 and Drag: Create small galaxy",
	"6. T: Toggle global trails",
	"7. LCTRL + T: Toggle local trails",
	"8. P: Toggle pixel drawing",
	"9. C: Clear all particles",
	"10. U: Toggle UI",
	"11. RMB on slider to set it to default",
	"12. Move with RMB",
	"13. Zoom with mouse wheel",
	"14. LCTRL + Scroll wheel: Brush size",
	"15. LCTRL + RMB on cluster to follow it",
	"16. LALT + RMB on particle to follow it",
	"17. LCTRL + LMB on cluster to select it",
	"18. LATL + LMB on particle to select it",
	"19. LCTRL + hold and drag MMB to box select",
	"20. LALT + hold and drag MMB to box deselect",
	"21. Select on empty space to deselect all",
	"22. Hold SHIFT to add to selection",
	"23. I: Invert selection",
	"24. Z: Center camera on selected particles",
	"25. F: Reset camera ",
	"26. Right click to open extra settings"
	};

	bool isShowControlsEnabled = true;
	bool isShowExtendedControlsEnabled = false;

	int controlsBoxSizeX = 435;
	int controlsBoxSizeY = 810;

	void showControls(bool& isMouseNotHoveringUI, bool& isDragging, int& screenWidth, int& screenHeight) {

		Button extendedControlsButton = { Vector2{ 20.0f, 230.0f }, Vector2{ 200.0f, 30.0f }, "Extended Controls", true };

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
				DrawRectangle(screenWidth / 2 - controlsBoxSizeX / 2, screenHeight / 2 - controlsBoxSizeY / 2, controlsBoxSizeX, controlsBoxSizeY, { 128,128,128,128 });
				for (size_t i = 0; i < extendedControlsArray.size(); i++) {
					DrawText(TextFormat("%s", extendedControlsArray[i].c_str()), 
						(screenWidth / 2 - controlsBoxSizeX / 2) + 20,
						((screenHeight / 2 - controlsBoxSizeY / 2) + 20) + 30 * static_cast<int>(i), 18, WHITE);
				}
			}
		}
	}
};