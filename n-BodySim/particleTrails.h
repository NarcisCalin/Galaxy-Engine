#pragma once

#include <iostream>
#include <vector>
#include "raylib.h"
#include "planet.h"


class ParticleTrails {
public:
	float size = 5;

	struct trailParams {
		Vector2 pos;
		Color color;
	};

	std::vector<trailParams> trailsParameters;

	ParticleTrails();

	void trailLogic(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, 
		bool& isTrailsEnabled, bool& isSelectedTrailsEnabled, int& maxLength);

	void drawTrail(std::vector<ParticleRendering>& rParticles, Texture2D& particleBlur);

private:
};