#pragma once

struct UpdateVariables;
class UI;

struct Controls {


	bool isShowControlsEnabled = false;

	bool isInformationEnabled = false;

	ImVec2 controlsSize = { 270.0f, 400.0f };

	void showControls(float& parametersWindowSizeX);

	void showInfo(bool& fullscreen);

	std::array<std::string, 32> controlsArray = {
	"----UTILITY----",
	"1. Clear Scene: C",
	"2. Global trails: T",
	"3. Local trails: LCTRL + T",
	"4. Reset Slider: RMB",
	"5. Brush size: LCTRL + Mouse wheel",
	"6. Record: R",
	"7. Take screenshot: S",
	"8. Copy selection: H",
	"9. Paste selection: J (Can Drag)",
	"10. Solidify all: P",
	"",
	"----CAMERA AND SELECTION----",
	"1. Pan Camera: RMB",
	"2. Zoom: Mouse wheel ",
	"3. Reset camera: F",
	"4. Follow cluster: LCTRL + RMB",
	"5. Follow particle: LALT + RMB",
	"6. Select cluster: LCTRL + MMB",
	"7. Select particle: LALT + MMB",
	"8. Box select: MMB",
	"9. Box deselect: LALT + MMB",
	"10. Deselect all: Click MMB or D",
	"11. Add selection: Hold SHIFT",
	"12. Invert selection: I",
	"13. Center selected particles: Z",
	"",
	"----3D CAMERA----",
	"1. Orbit: RMB",
	"2. Pan: RMB + ALT",
	"3. Zoom: Mouse wheel",
	"4. Use arrows to move"
	};

	std::array<std::string, 21> infoArray = {
		"----INFORMATION----",
		"",
		"Galaxy Engine is a personal project done for learning purposes",
		"by Narcis Calin. The project was entirely made with Raylib",
		"and C++ and it uses external libraries, including ImGui and FFmpeg.",
		"Galaxy Engine is Open Source and the code is available to anyone on GitHub.",
		"Below you can find some useful information:",
		"",
		"1. Dark Matter: Galaxy Engine simulates dark matter with",
		"invisible particles, which are 5 times heavier than visible ones",
		"",
		"2. Multi-Threading: Parallelizes the simulation across multiple",
		"threads. The default is half the max amount of physical cores your CPU has,",
		"but it is possible to modify this number.",
		"",
		"3. Frames Export Safe Mode: It is enabled by default when export",
		"frames is enabled. It stores your frames directly to disk, avoiding",
		"filling up your memory. Disabling it will make the render process",
		"much faster, but the program might crash once you run out of memory",
		"",
		"You can report any bugs you may find on our Discord Community."
	};
};