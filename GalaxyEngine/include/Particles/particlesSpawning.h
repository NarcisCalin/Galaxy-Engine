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

	void particlesInitialConditions(Quadtree* quadtree, Physics& physics, UpdateVariables& myVar, UpdateParameters& myParam);

	void copyPaste(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, 
		bool& isDragging, SceneCamera& myCamera, std::vector<ParticlePhysics>& pParticlesSelected, Physics& physics, 
		UpdateVariables& myVar, UpdateParameters& myParam);

	void predictTrajectory(const std::vector<ParticlePhysics>& actualParticles, 
		SceneCamera& myCamera, Physics physics,
		Quadtree* quadtree, UpdateVariables& myVar, Slingshot& slingshot);

private:
	glm::vec2 avgPos = { 0.0f, 0.0f };
	std::vector<ParticlePhysics> pParticlesCopied;
	std::vector<ParticleRendering> rParticlesCopied;

	std::vector<ParticleConstraint> constraintsCopied;

	float heavyParticleInitMass = 300000000000000.0f;
};