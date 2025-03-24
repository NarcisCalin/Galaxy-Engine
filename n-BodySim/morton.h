#pragma once
#include <cstdint>
#include <vector>
#include "planet.h"

struct Morton {
	uint64_t scaleToGrid(float pos, float maxVal = 1023);

	uint64_t spreadBits(uint64_t x);

	uint64_t morton2D(uint64_t x, uint64_t y);

	void computeMortonKeys(std::vector<ParticlePhysics>& pParticles, float screenSizeX, float screenSizeY);

	static void sortParticlesByMortonKey(std::vector<ParticlePhysics>& pParticles,
		std::vector<ParticleRendering>& rParticles);

};