#pragma once

#include "Particles/particle.h"

#include "Physics/quadtree.h"

#include "parameters.h"

struct ParticleConstraint {
	uint32_t id1;
	uint32_t id2;
	float restLength;
	float stiffness;
	bool isBroken;
};

struct Physics {

	std::vector<ParticleConstraint> particleConstraints;

	const float constraintStiffness = 60.0f;
	const float constraintDamping = 0.001f;
	float globalStiffness = 10.0f;

	float stiffCorrectionRatio = 0.013333f; // Heuristic. This used to modify the stiffness of a constraint in a more intuitive way

	glm::vec2 calculateForceFromGrid(const Quadtree& grid, std::vector<ParticlePhysics>& pParticles, UpdateVariables& myVar, 
		ParticlePhysics& pParticle);

	void temperatureCalculation(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, UpdateVariables& myVar);

	void createConstraints(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles);

	void constraints(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles,
		bool& isPeriodicBoundaryEnabled, glm::vec2& domainSize);

	void physicsUpdate(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, UpdateVariables& myVar, bool& sphGround);

	void collisions(ParticlePhysics& pParticleA, ParticlePhysics& pParticleB,
		ParticleRendering& rParticleA, ParticleRendering& rParticleB, UpdateVariables& myVar, float& dt);
};