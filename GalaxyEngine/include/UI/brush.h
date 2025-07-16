#pragma once

#include "Particles/particle.h"

#include "Physics/materialsSPH.h"

#include "UX/camera.h"

struct UpdateVariables;
struct UpdateParameters;

class Brush {
public:
	glm::vec2 mouseWorldPos;

	bool SPHWater = false;
	bool SPHRock = false;
	bool SPHIron = false;
	bool SPHSand = false;
	bool SPHSoil = false;
	bool SPHIce = false;
	bool SPHMud = false;
	bool SPHRubber = false;
	bool SPHGas = false;

	Brush(SceneCamera myCamera, float brushRadius);

	void brushLogic(UpdateParameters& myParam, bool& isSPHEnabled, bool& constraintAfterDrawing);

	void brushSize();

	void drawBrush(glm::vec2 mouseWorldPos);

	void eraseBrush(UpdateVariables& myVar, UpdateParameters& myParam);

	void particlesAttractor(UpdateVariables& myVar, UpdateParameters& myParam);

	void particlesSpinner(UpdateVariables& myVar, UpdateParameters& myParam);

	void particlesGrabber(UpdateVariables& myVar, UpdateParameters& myParam);

	void temperatureBrush(UpdateVariables& myVar, UpdateParameters& myParam);

	float brushRadius;

private:
	SceneCamera myCamera;

	float spinForce = 140.0f;

	glm::vec2 attractorForce = { 0.0f, 0.0f };

	bool dragging = false;
	glm::vec2 lastMouseVelocity = { 0.0f, 0.0f };

	std::vector<ParticlePhysics*> grabbedParticles;
};