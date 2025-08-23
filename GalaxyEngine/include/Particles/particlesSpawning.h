#pragma once

#include "Particles/particle.h"

#include "Physics/slingshot.h"

#include "Physics/constraint.h"

#include "UI/brush.h"

#include "UX/camera.h"

struct UpdateVariables;
struct UpdateParameters;

struct Physics;
struct Quadtree;

class ParticlesSpawning {
public:

	float heavyParticleWeightMultiplier = 1.0f;
	int predictPathLength = 1000;

	float particleAmountMultiplier = 1.0;
	float DMAmountMultiplier = 1.0f;

	const int correctionSubsteps = 24;
	bool particlesIterating = false;

	bool enablePathPrediction = false;
	bool isSpawningAllowed = true;

	bool massMultiplierEnabled = true;

	void particlesInitialConditions(Physics& physics, UpdateVariables& myVar, UpdateParameters& myParam);

	void predictTrajectory(const std::vector<ParticlePhysics>& actualParticles, 
		SceneCamera& myCamera, Physics physics, UpdateVariables& myVar, Slingshot& slingshot);

private:

	float heavyParticleInitMass = 300000000000000.0f;
};