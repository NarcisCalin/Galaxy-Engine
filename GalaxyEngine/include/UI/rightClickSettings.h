#pragma once

#include "Particles/particle.h"
#include "Particles/particleSubdivision.h"
#include "Particles/particleSelection.h"
#include "Particles/particleDeletion.h"

#include "UX/camera.h"

struct UpdateVariables;
struct UpdateParameters;

struct rightClickParams {
	std::string text;
	std::string tooltip;
	bool& parameter;

	rightClickParams(const std::string& t, const std::string& tool, bool& p) : text(t), tooltip(tool), parameter(p) {}
};

class RightClickSettings {
public:
	glm::vec2 menuPos = { 0.0f, 0.0f };
	glm::vec2 menuSize = { 0.0f, 0.0f };

	bool isMenuActive = false;

	void rightClickMenuSpawnLogic(bool& isMouseNotHoveringUI, bool& isSpawningAlone, bool& isDragging, bool& selectedColor);

	void rightClickMenu(UpdateVariables& myVar, UpdateParameters& myParam);

private:

	bool isMouseOnMenu = false;

	float buttonSizeY = 20.0f;

	bool selectedColorChanged = false;
	bool selectedColorOriginal = false;

	ImVec4 pCol = { 1.0f, 1.0f, 1.0f, 1.0f };
	ImVec4 sCol = { 1.0f, 1.0f, 1.0f, 1.0f };

	Color vecToRPColor = { 255,255,255,255 };
	Color vecToRSColor = { 255,255,255,255 };

	bool resetParticleColors = false;
};