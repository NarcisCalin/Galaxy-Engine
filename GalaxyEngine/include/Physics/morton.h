#pragma once

#include "Particles/particle.h"

struct Morton {
	uint64_t scaleToGrid(float pos, float minVal, float maxVal = 3840);

	uint64_t spreadBits(uint64_t x);

	uint64_t morton2D(uint64_t x, uint64_t y);

	void computeMortonKeys(std::vector<ParticlePhysics>& pParticles, glm::vec3& posSize);

	static void sortParticlesByMortonKey(std::vector<ParticlePhysics>& pParticles,
		std::vector<ParticleRendering>& rParticles);

};