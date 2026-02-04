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

	void brushLogic(UpdateParameters& myParam, bool& isSPHEnabled, bool& constraintAfterDrawing, float& massScatter);

	void brushSize();

	void drawBrush(glm::vec2 mouseWorldPos);

	void eraseBrush(UpdateVariables& myVar, UpdateParameters& myParam);

	void particlesAttractor(UpdateVariables& myVar, UpdateParameters& myParam);

	void particlesSpinner(UpdateVariables& myVar, UpdateParameters& myParam);

	void particlesGrabber(UpdateVariables& myVar, UpdateParameters& myParam);

	void temperatureBrush(UpdateVariables& myVar, UpdateParameters& myParam);

	float brushRadius = 25.0f;

private:

	float spinForce = 140.0f;

	glm::vec2 attractorForce = { 0.0f, 0.0f };

	bool dragging = false;
	glm::vec2 lastMouseVelocity = { 0.0f, 0.0f };

	std::vector<ParticlePhysics*> grabbedParticles;
};

class Brush3D {
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

	void brushLogic(UpdateParameters& myParam, bool& isSPHEnabled, bool& constraintAfterDrawing, float& massScatter);

	void brushSize();

	void drawBrush(float& domainHeight);

	void brushPosLogic(UpdateParameters& myParam);


	//void eraseBrush(UpdateVariables& myVar, UpdateParameters& myParam);

	//void particlesAttractor(UpdateVariables& myVar, UpdateParameters& myParam);

	//void particlesSpinner(UpdateVariables& myVar, UpdateParameters& myParam);

	//void particlesGrabber(UpdateVariables& myVar, UpdateParameters& myParam);

	//void temperatureBrush(UpdateVariables& myVar, UpdateParameters& myParam);

	float brushRadius = 25.0f;

	glm::vec3 brushPos = { 0.0f, 0.0f, 0.0f };

	float spawnDistance = 500.0f;
	float originalSpawnDistance = spawnDistance;

private:

	float spinForce = 140.0f;

	glm::vec2 attractorForce = { 0.0f, 0.0f };

	bool dragging = false;
	glm::vec2 lastMouseVelocity = { 0.0f, 0.0f };

	std::vector<ParticlePhysics*> grabbedParticles;
};