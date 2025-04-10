#pragma once

#include <iostream>
#include <vector>
#include "raylib.h"
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

	float trailThickness = 0.04f;

	bool whiteTrails = false;

	std::vector<TrailDot> trailDots;

	Vector2 selectedParticlesAveragePos = { 0.0f };

	ParticleTrails();

	void trailLogic(UpdateVariables& myVar, UpdateParameters& myParam);

	void drawTrail(std::vector<ParticleRendering>& rParticles, Texture2D& particleBlur);

private:
	bool wasLocalTrailsEnabled = false;
};