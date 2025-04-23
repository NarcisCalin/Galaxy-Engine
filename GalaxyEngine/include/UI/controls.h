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
	"3. Hold 1 and Drag: Create big galaxy",
	"4. Hold 2 and Drag: Create small galaxy",
	"5. Hold 3 and Drag: Create star",
	"6. C: Clear all particles",
	};

	std::array<std::string, 8> extendedControlsArrayParticles = {
	"PARTICLES CREATION",
	"1. Hold LMB and Drag: Throw heavy particle",
	"2. Hold MMB: Paint particles",
	"3. Hold 1 and Drag: Create big galaxy",
	"4. Hold 2 and Drag: Create small galaxy",
	"5. Hold 3 and Drag: Create star",
	"6. Press 4: Create Big Bang",
	"7. C: Clear all particles"
	};

	std::array<std::string, 21> extendedControlsArrayCamAndSelec = {
	"CAMERA AND SELECTION",
	"1. Move with RMB",
	"2. Zoom with mouse wheel",
	"3. LCTRL + RMB on cluster to follow it",
	"4. LALT + RMB on particle to follow it",
	"5. LCTRL + LMB on cluster to select it",
	"6. LALT + LMB on particle to select it",
	"7. LCTRL + hold and drag MMB to box select",
	"8. LALT + hold and drag MMB to box deselect",
	"9. Select on empty space to deselect all",
	"10. Hold SHIFT to add to selection",
	"11. I: Invert selection",
	"12. Z: Center camera on selected particles",
	"13. F: Reset camera ",
	"14. D: Deselect all particles"
	};

	std::array<std::string, 14> extendedControlsArrayUtility = {
	"UTILITY",
	"1. T: Toggle global trails",
	"2. LCTRL + T: Toggle local trails",
	"3. U: Toggle UI",
	"4. RMB on slider to set it to default",
	"5. Right click to open extra settings",
	"6. LCTRL + Scroll wheel : Brush size",
	"7. B: Brush attract particles",
	"8. N: Brush spin particles",
	"9. M: Brush grab particles",
	"10. Hold CTRL to invert brush effects",
	"11. R: Record frames",
	"12. S: Take screenshot",
	"13. X + MMB: Eraser"
	};

	std::array<std::string, 27> informationArray = {
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
		"They work as intended when gravity is disabled.",
		"8. Recording Disk Mode: This is the safe mode of recording, It is",
		"enabled by default. It stores your frames directly in your folder",
		"after they have been processed. Runs slower but it doesn't fill up",
		"your memory."
	};

	bool isShowControlsEnabled = true;
	bool isShowExtendedControlsEnabled = false;
	bool nextPage = false;

	int controlsBoxSizeX = 435;
	int controlsBoxSizeY = 810;

	void showControls(UpdateVariables& myVar);

	void showMoreInfo(UpdateVariables& myVar);
};