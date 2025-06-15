#pragma once

#include "Particles/particle.h"

#include "Physics/quadtree.h"

#include "parameters.h"

struct ParticleConstraint {
	uint32_t id1;
	uint32_t id2;
	float restLength;
	float stiffness;
	float hardness;
	bool isBroken;
};

struct Physics {

	std::vector<ParticleConstraint> particleConstraints;
	std::unordered_map<uint64_t, ParticleConstraint*> constraintMap;

	uint64_t makeKey(uint32_t id1, uint32_t id2) {
		return id1 < id2 ? ((uint64_t)id1 << 32) | id2
			: ((uint64_t)id2 << 32) | id1;
	}

	const float constraintStiffness = 60.0f;
	const float constraintDamping = 0.001f;
	float globalStiffness = 10.0f;

	float stiffCorrectionRatio = 0.013333f; // Heuristic. This used to modify the stiffness of a constraint in a more intuitive way

	glm::vec2 calculateForceFromGrid(const Quadtree& grid, std::vector<ParticlePhysics>& pParticles, UpdateVariables& myVar, 
		ParticlePhysics& pParticle);

	void temperatureCalculation(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, UpdateVariables& myVar);

	void createConstraints(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, bool& constraintAllSolids);

	void constraints(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles,
		bool& isPeriodicBoundaryEnabled, glm::vec2& domainSize);

	void physicsUpdate(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, UpdateVariables& myVar, bool& sphGround);

	void collisions(ParticlePhysics& pParticleA, ParticlePhysics& pParticleB,
		ParticleRendering& rParticleA, ParticleRendering& rParticleB, UpdateVariables& myVar, float& dt);
};