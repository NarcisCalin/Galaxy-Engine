#pragma once
#include "../raylib/raylib.h"
#include "../Particles/particle.h"
#include "quadtree.h"
#include "../parameters.h"
#include <vector>
#include <cmath>
#include "omp.h"

struct Physics {

	float bounciness = 0.6f;

	Vector2 calculateForceFromGrid(const Quadtree& grid, std::vector<ParticlePhysics>& pParticles, UpdateVariables& myVar, ParticlePhysics& pParticle);
	void pairWiseGravity(std::vector<ParticlePhysics>& pParticles, UpdateVariables& myVar);
	void physicsUpdate(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, UpdateVariables& myVar, float& dt);
	void collisions(ParticlePhysics& pParticleA, ParticlePhysics& pParticleB,
		ParticleRendering& rParticleA, ParticleRendering& rParticleB, float& softening, float& particleTextureHalfSize, float& dt);
};