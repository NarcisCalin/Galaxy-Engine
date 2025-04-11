#pragma once

#include "../raylib/raylib.h"
#include "button.h"
#include <array>
#include <string>

struct UpdateVariables;

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

	std::array<std::string, 21> extendedControlsArrayCamAndSelec = {
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
	"19. F: Reset camera ",
	"20. D: Deselect all particles"
	};

	std::array<std::string, 15> extendedControlsArrayUtility = {
	"UTILITY",
	"21. T: Toggle global trails",
	"22. LCTRL + T: Toggle local trails",
	"23. P: Toggle pixel drawing",
	"24. U: Toggle UI",
	"25. RMB on slider to set it to default",
	"26. Right click to open extra settings",
	"27. LCTRL + Scroll wheel : Brush size",
	"28. B: Brush attract particles",
	"29. N: Brush spin particles",
	"30. M: Brush grab particles",
	"31. Hold CTRL to invert brush effects",
	"32. R: Record frames",
	"33. S: Take screenshot",
	"34. X + MMB: Eraser"
	};

	std::array<std::string, 23> informationArray = {
		"INFORMATION",
		"This is a personal project done for learning purposes",
		"by Narcis Calin. The project was entirely made with raylib",
		"and C++. The code is available for anyone on Github.",
		"Down below you can find some useful information:",
		"1. Theta: is controls the quality of the Barnes-Hut simulation",
		"2. Barnes-Hut: This is the main simulation algorithm. Disabling",
		"it will switch to a pairwise algorithm which is technically more",
		"accurate but also much slower.",
		"3. Dark Matter: Currently this is a very basic implementation",
		"which simulates a dark matter halo in the center of the domain.",
		"It is intented to be used with the big galaxy. Dark matter only",
		"works with Barnes-Hut enabled",
		"4. Multi-Threading: The default value for multi-threading",
		"is 16 threads, but it should dynamically change.",
		"5. Screen Capture: You can start recording with R and you can",
		"take screenshots with S. Once you are done recording, you will",
		"see an export button at the top of the window. This should export",
		"all your frames to the executable's directory",
		"6. Density Size: This will give the simulation a gas-like look.",
		"7. Collisions: Currently collisions are experimental. They do not",
		"respect conservation of energy when they are enabled with gravity.",
		"They work as intended when gravity is disabled."
	};

	bool isShowControlsEnabled = true;
	bool isShowExtendedControlsEnabled = false;
	bool nextPage = false;

	int controlsBoxSizeX = 435;
	int controlsBoxSizeY = 810;

	void showControls(UpdateVariables& myVar);

	void showMoreInfo(UpdateVariables& myVar);
};