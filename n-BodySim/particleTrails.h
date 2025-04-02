#pragma once

#include <iostream>
#include <vector>
#include "raylib.h"
#include "planet.h"


class ParticleTrails {
public:
	float size = 5;

	struct trailDot {
		Vector2 pos;
		Vector2 offset;
		Color color;
	};

	std::vector<trailDot> trailDots;

	Vector2 selectedParticlesAveragePos = { 0.0f };

	ParticleTrails();

	void trailLogic(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles,
		std::vector<ParticlePhysics>& pParticlesSelected, std::vector<ParticleRendering>& rParticlesSelected, bool& isTrailsEnabled, 
		bool& isSelectedTrailsEnabled, int& maxLength, float& timeFactor, bool& isLocalTrailsEnabled);

	void drawTrail(std::vector<ParticleRendering>& rParticles, Texture2D& particleBlur);

private:
	bool wasLocalTrailsEnabled = false;
};