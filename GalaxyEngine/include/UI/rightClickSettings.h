#pragma once

#include <vector>
#include <array>
#include "../raylib/raylib.h"
#include "button.h"
#include "slider.h"
#include "../Particles/particle.h"
#include "../Particles/particleSubdivision.h"
#include "../Particles/particleSelection.h"
#include "../UX/camera.h"
#include "../Particles/particleDeletion.h"
#include <algorithm>

struct UpdateVariables;
struct UpdateParameters;

class RightClickSettings {
public:
	Vector2 menuPos = { 0.0f, 0.0f };
	Vector2 menuSize = { 0.0f, 0.0f };

	void rightClickMenuSpawnLogic(bool& isMouseNotHoveringUI);

	void rightClickMenu(UpdateVariables& myVar, UpdateParameters& myParam);

private:
	bool isMenuActive = false;

	Color menuColor = { 30,30,30,255 };

	float buttonSizeX = 195.0f;
	float buttonSizeY = 15.0f;

	float sliderSizeX = 175.0f;
	float sliderSizeY = 7.0f;

	float menuButtonGap = 2.0f;
	float menuSliderGap = 28.0f;

	bool selectedColorChanged = false;

	float pR = 1.0f;
	float pG = 1.0f;
	float pB = 1.0f;
	float pA = 1.0f;

	float sR = 1.0f;
	float sG = 1.0f;
	float sB = 1.0f;
	float sA = 1.0f;

	bool isMouseOnMenu = false;
	bool isMouseOnSlider = false;

	bool resetParticleColors = false;

	std::array<Button, 12> menuBottons = {

Button({0.0f, 0.0f}, {0.0f, 0.0f}, "Subdivide All", true),

Button({0.0f, 0.0f}, {0.0f, 0.0f}, "Subdivide Selec.", true),

Button({0.0f, 0.0f}, {0.0f, 0.0f}, "Invert Selection", true),

Button({0.0f, 0.0f}, {0.0f, 0.0f}, "Deselect All", true),

Button({0.0f, 0.0f}, {0.0f, 0.0f}, "Follow Selection", true),

Button({0.0f, 0.0f}, {0.0f, 0.0f}, "Select Clusters", true),

Button({0.0f, 0.0f}, {0.0f, 0.0f}, "Delete Selection", true),

Button({0.0f, 0.0f}, {0.0f, 0.0f}, "Delete Strays", true),

Button({0.0f, 0.0f}, {0.0f, 0.0f}, "Reset P. Colors", true),

Button({0.0f, 0.0f}, {0.0f, 0.0f}, "Debug Z Curves", true),

Button({0.0f, 0.0f}, {0.0f, 0.0f}, "Debug Quadtree", true),

Button({0.0f, 0.0f}, {0.0f, 0.0f}, "Record Disk Mode", true)
	};

	std::array<Slider, 8> menuSliders = {

Slider({20, 530.0f}, {230.0f, 7.0f}, {190, 100, 100, 255}, "Particle P Red"),

Slider({20, 530.0f}, {230.0f, 7.0f}, {100, 190, 100, 255}, "Particle P Green"),

Slider({20, 530.0f}, {230.0f, 7.0f}, {100, 100, 190, 255}, "Particle P Blue"),

Slider({20, 530.0f}, {230.0f, 7.0f}, {128, 128, 128, 255}, "Particle P Alpha"),

Slider({20, 530.0f}, {230.0f, 7.0f}, {150, 80, 80, 255}, "Particle S Red"),

Slider({20, 530.0f}, {230.0f, 7.0f}, {80, 150, 80, 255}, "Particle S Green"),

Slider({20, 530.0f}, {230.0f, 7.0f}, {80, 80, 150, 255}, "Particle S Blue"),

Slider({20, 530.0f}, {230.0f, 7.0f}, {108, 108, 108, 255}, "Particle S Alpha")
	};
};