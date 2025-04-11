#pragma once

#include "../raylib/raylib.h"
#include "particle.h"
#include "../UX/camera.h"
#include "../Physics/slingshot.h"
#include "../UI/brush.h"
#include <vector>

struct UpdateVariables;
struct UpdateParameters;

struct Physics;
struct Quadtree;

class ParticlesSpawning {
public:

	float heavyParticleWeightMultiplier = 1.0f;
	int predictPathLength = 1000;

	bool enablePathPrediction = false;

	void particlesInitialConditions(Quadtree& quadtree, Physics& physics, UpdateVariables& myVar, UpdateParameters& myParam);

	void predictTrajectory(const std::vector<ParticlePhysics>& actualParticles, SceneCamera& myCamera, Physics physics,
		Quadtree& quadtree, UpdateVariables& myVar, Slingshot& slingshot);

private:
	bool isSpawningAllowed = true;
	float heavyParticleInitMass = 300000000000000.0f;
};