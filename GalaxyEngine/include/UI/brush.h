#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <raylib.h>
#include "../Particles/particle.h"
#include "../UX/camera.h"
#include "../../include/Physics/materialsSPH.h"

struct UpdateVariables;
struct UpdateParameters;

class Brush {
public:
	glm::vec2 mouseWorldPos;

	bool SPHWater = false;
	bool SPHRock = false;
	bool SPHSand = false;
	bool SPHSoil = false;
	bool SPHIce = false;
	bool SPHMud = false;

	Brush(SceneCamera myCamera, float brushRadius);

	void brushLogic(UpdateParameters& myParam, bool& isSPHEnabled);

	void brushSize();

	void drawBrush(glm::vec2 mouseWorldPos);

	void eraseBrush(UpdateParameters& myParam);

	void particlesAttractor(UpdateVariables& myVar, UpdateParameters& myParam);

	void particlesSpinner(UpdateVariables& myVar, UpdateParameters& myParam);

	void particlesGrabber(UpdateParameters& myParam);

private:
	float brushRadius;
	SceneCamera myCamera;

	float spinForce = 140.0f;

	glm::vec2 attractorForce = { 0.0f, 0.0f };

	bool dragging = false;
	glm::vec2 lastMouseVelocity = { 0.0f, 0.0f };

	std::vector<ParticlePhysics*> grabbedParticles;
};