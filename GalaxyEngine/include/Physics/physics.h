#pragma once

#include "Particles/particle.h"

#include "Physics/quadtree.h"

#include "Physics/constraint.h"

#include "parameters.h"

struct Physics {

	std::vector<ParticleConstraint> particleConstraints;
	std::unordered_map<uint64_t, ParticleConstraint*> constraintMap;

	uint64_t makeKey(uint32_t id1, uint32_t id2) {
		return id1 < id2 ? ((uint64_t)id1 << 32) | id2
			: ((uint64_t)id2 << 32) | id1;
	}

	const float globalConstraintDamping = 0.001f;

	const float stiffCorrectionRatio = 0.013333f; // Heuristic. This used to modify the stiffness of a constraint in a more intuitive way. DO NOT CHANGE

	glm::vec2 calculateForceFromGrid(const Quadtree& grid, std::vector<ParticlePhysics>& pParticles, UpdateVariables& myVar, 
		ParticlePhysics& pParticle);

	void temperatureCalculation(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, UpdateVariables& myVar);

	void createConstraints(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, bool& constraintCreateSpecialFlag,
		UpdateVariables& myVar);

	void constraints(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, UpdateVariables& myVar);

	void pausedConstraints(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, UpdateVariables& myVar);

	void mergerSolver(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, UpdateVariables& myVar);

	void physicsUpdate(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, UpdateVariables& myVar, bool& sphGround);

	void collisions(ParticlePhysics& pParticleA, ParticlePhysics& pParticleB,
		ParticleRendering& rParticleA, ParticleRendering& rParticleB, float& radius);

	void buildGrid(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles,
		Physics& physics, glm::vec2& domainSize, const int& iterations);
};