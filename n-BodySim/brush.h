#pragma once
#include <vector>
#include "raylib.h"
#include "particle.h"
#include "camera.h"

struct UpdateVariables;
struct UpdateParameters;

class Brush {
public:
	Vector2 mouseWorldPos;

	Brush(SceneCamera myCamera, float brushRadius);

	void brushLogic(UpdateParameters& myParam);

	void brushSize(Vector2 mouseWorldPos);

	void drawBrush(Vector2 mouseWorldPos);

	void eraseBrush(UpdateVariables& myVar, UpdateParameters& myParam);

	void particlesAttractor(UpdateVariables& myVar, UpdateParameters& myParam);

	void particlesSpinner(UpdateVariables& myVar, UpdateParameters& myParam);

	void particlesGrabber(UpdateVariables& myVar, UpdateParameters& myParam);

private:
	float brushRadius;
	SceneCamera myCamera;

	float spinForce = 140.0f;

	Vector2 attractorForce = { 0.0f };

	bool dragging = false;
	Vector2 lastMouseVelocity = { 0.0f, 0.0f };
};