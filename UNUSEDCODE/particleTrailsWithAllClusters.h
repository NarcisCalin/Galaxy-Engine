#pragma once

#ifndef PARTICLE_TRAILS_H
#define PARTICLE_TRAILS_H

#include <iostream>
#include <vector>
#include "raylib.h"
#include "planet.h"
#include "particlesSelection.h"


class ParticleTrails {
public:
	float size = 5;
	ParticleTrails();

	template <typename T, typename B>
	void trailLogic(std::vector<T>& pParticles, std::vector<B>& rParticles, bool& isTrailsEnabled, int& maxLength);

	template <typename T>
	void drawTrail(std::vector<T>& rParticles, Texture2D& particleBlur);

private:
	struct trailParams {
		Vector2 pos;
		Color color;
	};
	std::vector<trailParams> trailsParameters;
};

#endif