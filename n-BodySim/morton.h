#pragma once
#include <cstdint>
#include <vector>
#include "planet.h"

struct Morton {
	uint32_t scaleToGrid(float pos, float maxVal = 1023);

	uint32_t spreadBits(uint32_t x);

	uint32_t morton2D(uint32_t x, uint32_t y);

	void computeMortonKeys(std::vector<ParticlePhysics>& pParticles);

	static void sortParticlesByMortonKey(std::vector<ParticlePhysics>& pParticles,
		std::vector<ParticleRendering>& rParticles);

};