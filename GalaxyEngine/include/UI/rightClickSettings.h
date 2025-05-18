#pragma once

#include <vector>
#include <array>
#include "../raylib/raylib.h"
#include "../Particles/particle.h"
#include "../Particles/particleSubdivision.h"
#include "../Particles/particleSelection.h"
#include "../UX/camera.h"
#include "../Particles/particleDeletion.h"
#include "../../external/imgui/imgui.h"
#include "../../external/imgui/rlImGui.h"
#include "../../external/imgui/rlImGuiColors.h"
#include <algorithm>

struct UpdateVariables;
struct UpdateParameters;

struct rightClickParams {
	std::string text;
	bool& parameter;

	rightClickParams(const std::string& t, bool& p) : text(t), parameter(p) {}
};

class RightClickSettings {
public:
	Vector2 menuPos = { 0.0f, 0.0f };
	Vector2 menuSize = { 0.0f, 0.0f };

	void rightClickMenuSpawnLogic(bool& isMouseNotHoveringUI, bool& isSpawningAlone, bool& isDragging);

	void rightClickMenu(UpdateVariables& myVar, UpdateParameters& myParam);

private:
	bool isMenuActive = false;

	bool isMouseOnMenu = false;

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

	ImVec4 pCol = { 1.0f, 1.0f, 1.0f, 1.0f };
	ImVec4 sCol = { 1.0f, 1.0f, 1.0f, 1.0f };

	float sR = 1.0f;
	float sG = 1.0f;
	float sB = 1.0f;
	float sA = 1.0f;

	bool resetParticleColors = false;
};