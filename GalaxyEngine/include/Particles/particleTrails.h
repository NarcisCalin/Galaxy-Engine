#pragma once

#include <iostream>
#include <vector>
#include "../raylib/raylib.h"
#include "particle.h"

struct UpdateVariables;
struct UpdateParameters;


class ParticleTrails {
public:
	float size = 5;

	struct TrailDot {
		Vector2 pos;
		Vector2 offset;
		Color color;
	};

	float trailThickness = 0.07f;

	bool whiteTrails = false;

	std::vector<TrailDot> trailDots;

	Vector2 selectedParticlesAveragePos = { 0.0f, 0.0f };

	void trailLogic(UpdateVariables& myVar, UpdateParameters& myParam);

	void drawTrail(std::vector<ParticleRendering>& rParticles, Texture2D& particleBlur);

private:
	bool wasLocalTrailsEnabled = false;
};