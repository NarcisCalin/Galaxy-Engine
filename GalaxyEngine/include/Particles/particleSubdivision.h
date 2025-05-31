#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <raylib.h>
#include "particle.h"

struct UpdateVariables;
struct UpdateParameters;

struct ParticleSubdivision {
	int particlesThreshold = 80000;

	bool subdivideAll = false;
	bool subdivideSelected = false;

	void subdivideParticles(UpdateVariables& myVar, UpdateParameters& myParam);

private:
	bool confirmState = false;
	bool quitState = false;

	std::string warningText = "Subdividing further might slow down the program a lot";

	float textSize = 25.0f;
	float textSpacing = 6.0f;

	glm::vec2 buttonSize = { 200.0f, 50.0f };
};