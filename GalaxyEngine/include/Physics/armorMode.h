#pragma once

#include "Physics/physics.h"

struct UpdateVariables;
struct UpdateParameters;

struct ArmorMode {

	glm::vec2 a = { 0.0f, 0.0f };
	glm::vec2 b = { 0.0f, 0.0f };
	glm::vec2 c = { 0.0f, 0.0f };

	bool startThicknessFlag = false;
	bool setB = false;
	bool isDrawingRect = false;

	bool wasArmorEnabled = false;

	bool disableBoxFlag = false;

	bool originalConstraintsEnabled;
	float originalGravityMultiplier;

	float spacing = 2.4f;

	float spacingCircle = 2.4f;

	float circleRadius = 0.0f;
	bool setCircleRadius = false;
	bool spawnCircleFlag = false;

	bool disableCircleFlag = false;

	glm::vec2 circleA = { 0.0f, 0.0f };

	void boxDrawTool(UpdateParameters& myParam, UpdateVariables& myVar, Physics& physics);

	void circleTool(UpdateParameters& myParam, UpdateVariables& myVar, Physics& physics, float& brushSize);

	void armorLogic(UpdateParameters& myParam, UpdateVariables& myVar, Physics& physics, float& brushSize) {

		if (myVar.toolBoxDraw) {
			boxDrawTool(myParam, myVar, physics);
		}
		else {
			setB = false;
			isDrawingRect = false;
			startThicknessFlag = false;
			disableBoxFlag = false;
		}

		if (myVar.toolDrawCircle) {
			circleTool(myParam, myVar, physics, brushSize);
		}
		else {
			setCircleRadius = false;
			spawnCircleFlag = false;
			disableCircleFlag = false;
		}
	}
};