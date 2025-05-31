#pragma once

#include "Particles/particle.h"

#include "Physics/quadtree.h"

#include "parameters.h"

struct Physics {

	glm::vec2 calculateForceFromGrid(const Quadtree& grid, std::vector<ParticlePhysics>& pParticles, UpdateVariables& myVar, ParticlePhysics& pParticle);

	void physicsUpdate(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, UpdateVariables& myVar, bool& sphGround);

	void collisions(ParticlePhysics& pParticleA, ParticlePhysics& pParticleB,
		ParticleRendering& rParticleA, ParticleRendering& rParticleB, UpdateVariables& myVar, float& dt);
};