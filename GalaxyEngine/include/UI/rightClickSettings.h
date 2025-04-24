#pragma once

#include <vector>
#include <array>
#include "../raylib/raylib.h"
#include "button.h"
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

	float buttonSizeX = 175.0f;
	float buttonSizeY = 20.0f;

	float menuButtonGap = 3.0f;

	std::array<Button, 11> menuSettings = {

Button({0.0f, 0.0f}, {0.0f, 0.0f}, "Subdivide All", true),

Button({0.0f, 0.0f}, {0.0f, 0.0f}, "Subdivide Selec.", true),

Button({0.0f, 0.0f}, {0.0f, 0.0f}, "Invert Selection", true),

Button({0.0f, 0.0f}, {0.0f, 0.0f}, "Deselect All", true),

Button({0.0f, 0.0f}, {0.0f, 0.0f}, "Follow Selection", true),

Button({0.0f, 0.0f}, {0.0f, 0.0f}, "Select Clusters", true),

Button({0.0f, 0.0f}, {0.0f, 0.0f}, "Delete Selection", true),

Button({0.0f, 0.0f}, {0.0f, 0.0f}, "Delete Strays", true),

Button({0.0f, 0.0f}, {0.0f, 0.0f}, "Debug Z Curves", true),

Button({0.0f, 0.0f}, {0.0f, 0.0f}, "Debug Quadtree", true),

Button({0.0f, 0.0f}, {0.0f, 0.0f}, "Record Disk Mode", true)
	};
};