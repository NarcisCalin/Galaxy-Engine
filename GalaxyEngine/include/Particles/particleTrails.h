#pragma once

#include "Particles/particle.h"

struct UpdateVariables;
struct UpdateParameters;


class ParticleTrails {
public:
	float size = 5;

	float trailThickness = 0.5f;

	bool whiteTrails = false;

	struct Segment {
		glm::vec2 start;
		glm::vec2 end;
		glm::vec2 offset;
		glm::vec2 prevOffset;
		Color color;
	};

	std::vector<Segment> segments;

	glm::vec2 selectedParticlesAveragePos = { 0.0f, 0.0f };
	glm::vec2 selectedParticlesAveragePrevPos = { 0.0f, 0.0f };

	void trailLogic(UpdateVariables& myVar, UpdateParameters& myParam);

	void drawTrail(std::vector<ParticleRendering>& rParticles, Texture2D& particleBlur);

private:
	bool wasLocalTrailsEnabled = false;
};