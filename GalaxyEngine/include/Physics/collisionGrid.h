#pragma once

#include <glm.hpp>
#include <raylib.h>
#include <vector>
#include "../Particles/particle.h"
#include "physics.h"

struct UpdateVariables;

struct CollisionGrid {


	float cellSize = 0.0f;

	void buildGrid(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles,
		Physics& physics, UpdateVariables& myVar, glm::vec2& gridSize, float& dt);

};