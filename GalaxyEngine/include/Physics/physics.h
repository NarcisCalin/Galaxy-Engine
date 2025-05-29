#pragma once

#include "../../external/glm/glm/glm.hpp"
#include "../raylib/raylib.h"
#include "../Particles/particle.h"
#include "quadtree.h"
#include "../parameters.h"
#include <vector>
#include <cmath>
#include "omp.h"

struct Physics {

	glm::vec2 calculateForceFromGrid(const Quadtree& grid, std::vector<ParticlePhysics>& pParticles, UpdateVariables& myVar, ParticlePhysics& pParticle);

	void physicsUpdate(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, UpdateVariables& myVar, bool& sphGround);

	void collisions(ParticlePhysics& pParticleA, ParticlePhysics& pParticleB,
		ParticleRendering& rParticleA, ParticleRendering& rParticleB, UpdateVariables& myVar, float& dt);
};