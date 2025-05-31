#pragma once

#include "Particles/particle.h"

struct UpdateVariables;
struct UpdateParameters;


class ParticleTrails {
public:
	float size = 5;

	struct TrailDot {
		glm::vec2 pos;
		glm::vec2 offset;
		Color color;
	};

	float trailThickness = 0.07f;

	bool whiteTrails = false;

	std::vector<TrailDot> trailDots;

	glm::vec2 selectedParticlesAveragePos = { 0.0f, 0.0f };

	void trailLogic(UpdateVariables& myVar, UpdateParameters& myParam);

	void drawTrail(std::vector<ParticleRendering>& rParticles, Texture2D& particleBlur);

private:
	bool wasLocalTrailsEnabled = false;
};