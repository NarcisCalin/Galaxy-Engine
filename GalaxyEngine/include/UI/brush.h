#pragma once

#include <vector>
#include "../raylib/raylib.h"
#include "../Particles/particle.h"
#include "../UX/camera.h"
#include "../../include/Physics/materialsSPH.h"

struct UpdateVariables;
struct UpdateParameters;

class Brush {
public:
	Vector2 mouseWorldPos;

	bool SPHWater = false;
	bool SPHRock = false;
	bool SPHSand = false;
	bool SPHMud = false;

	Brush(SceneCamera myCamera, float brushRadius);

	void brushLogic(UpdateParameters& myParam, bool& isSPHEnabled);

	void brushSize();

	void drawBrush(Vector2 mouseWorldPos);

	void eraseBrush(UpdateParameters& myParam);

	void particlesAttractor(UpdateVariables& myVar, UpdateParameters& myParam);

	void particlesSpinner(UpdateVariables& myVar, UpdateParameters& myParam);

	void particlesGrabber(UpdateParameters& myParam);

private:
	float brushRadius;
	SceneCamera myCamera;

	float spinForce = 140.0f;

	Vector2 attractorForce = { 0.0f, 0.0f };

	bool dragging = false;
	Vector2 lastMouseVelocity = { 0.0f, 0.0f };

	std::vector<ParticlePhysics*> grabbedParticles;
};