#pragma once

#include "Particles/particle.h"

#include "Physics/materialsSPH.h"

#include "Physics/quadtree.h"

#include "UX/camera.h"

#include "Particles/clusterMouseHelper.h"

struct UpdateVariables;
struct UpdateParameters;

class Brush {
public:
	glm::vec2 mouseWorldPos;

	void brushLogic(UpdateParameters& myParam, bool& isSPHEnabled, bool& constraintAfterDrawing, float& massScatter, UpdateVariables& myVar);

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

	void brushLogic(UpdateParameters& myParam, bool& isSPHEnabled, bool& constraintAfterDrawing, float& massScatter, UpdateVariables& myVar);

	void brushSize();

	void drawBrush(float& domainHeight);

	glm::vec3 brushPosLogic(UpdateParameters& myParam, UpdateVariables& myVar);


	void eraseBrush(UpdateVariables& myVar, UpdateParameters& myParam);

	//void particlesAttractor(UpdateVariables& myVar, UpdateParameters& myParam);

	//void particlesSpinner(UpdateVariables& myVar, UpdateParameters& myParam);

	void particlesGrabber(UpdateVariables& myVar, UpdateParameters& myParam);

	//void temperatureBrush(UpdateVariables& myVar, UpdateParameters& myParam);

	float brushRadius = 25.0f;

	glm::vec3 brushPos = { 0.0f, 0.0f, 0.0f };

	float spawnDistance = 500.0f;
	float originalSpawnDistance = spawnDistance;

private:

	bool wasEmpty = true;

	float spinForce = 140.0f;

	glm::vec2 attractorForce = { 0.0f, 0.0f };

	bool dragging = false;
	glm::vec3 lastBrushVelocity = { 0.0f, 0.0f, 0.0f };

	glm::vec3 lastBrushPos = { 0.0f, 0.0f, 0.0f };

	std::vector<ParticlePhysics*> grabbedParticles;
};