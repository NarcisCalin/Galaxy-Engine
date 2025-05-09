#pragma once

#include "../raylib/raylib.h"
#include "button.h"
#include <array>
#include <string>

struct UpdateVariables;

struct Controls {

	std::array<std::string, 8> controlsArrayParticles = {
	"PARTICLES CREATION",
	"1. Hold LMB and Drag: Throw heavy particle",
	"2. Hold MMB: Paint particles",
	"3. Hold 1 and Drag: Create big galaxy",
	"4. Hold 2 and Drag: Create small galaxy",
	"5. Hold 3 and Drag: Create star",
	"6. Press 4: Create Big Bang",
	"7. C: Clear all particles"
	};

	std::array<std::string, 21> controlsArrayCamAndSelec = {
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

	std::array<std::string, 18> controlsArrayUtility = {
	"UTILITY",
	"1. TAB: Toggle fullscreen",
	"2. T: Toggle global trails",
	"3. LCTRL + T: Toggle local trails",
	"4. U: Toggle UI",
	"5. RMB on slider to set it to default",
	"6. Right click to open extra settings",
	"7. LCTRL + Scroll wheel : Brush size",
	"8. B: Brush attract particles",
	"9. N: Brush spin particles",
	"10. M: Brush grab particles",
	"11. Hold CTRL to invert brush effects",
	"12. R: Record frames",
	"13. S: Take screenshot",
	"14. X + MMB: Eraser",
	"15. H: Copy selected",
	"16. Hold J and drag: Throw copied",
	"17. Arrows: Control selected particles"
	};

	std::array<std::string, 20> informationArray = {
		"INFORMATION",
		"This is a personal project done for learning purposes",
		"by Narcis Calin. The project was entirely made with Raylib",
		"and C++. The code is available for anyone on GitHub.",
		"Below you can find some useful information:",
		"1. Theta: Controls the quality of the Barnes-Hut simulation",
		"2. Barnes-Hut: This is the main gravity algorithm.",
		"3. Dark Matter: Galaxy Engine simulates dark matter with",
		"invisible particles, which are 5 times heavier than visible ones",
		"4. Multi-Threading: Parallelizes the simulation across multiple",
		"threads. The default is 16 threads, but it is possible to modify this.",
		"5. Collisions: Currently, collisions are experimental. They do not",
		"respect conservation of energy when they are enabled with gravity.",
		"They work as intended when gravity is disabled.",
		"6. SPH: This enables fluids for planetary simulation. Each SPH",
		"material has different parameters like stiffness, viscosity,",
		"cohesion, and more.",
		"7. Recording Disk Mode: This is the safe mode of recording. It is",
		"enabled by default. It stores your frames directly to disk.",
		"It runs slower but doesn't fill up memory"
	};

	bool isShowControlsEnabled = false;
	bool nextPage = false;

	bool isInformationEnabled = false;

	float controlsBoxSizeX = 435.0f;
	float controlsBoxSizeY = 810.0f;

	float fontSize = 18.0f;
	float fontSeparation = 1.0f;

	float fontYBias = 0.0f;

	float fontYSpacing = 30.0f;

	void showControls(bool& isMouseNotHoveringUI, bool& isDragging);

	void showMoreInfo();
};