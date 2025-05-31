#pragma once
#include <cstdint>
#include <vector>
#include <glm/glm.hpp>
#include "../Particles/particle.h"

struct Morton {
	uint32_t scaleToGrid(float pos, float minVal, float maxVal = 1023);

	uint32_t spreadBits(uint32_t x);

	uint32_t morton2D(uint32_t x, uint32_t y);

	void computeMortonKeys(std::vector<ParticlePhysics>& pParticles, glm::vec2& minPosition, float& size);

	static void sortParticlesByMortonKey(std::vector<ParticlePhysics>& pParticles,
		std::vector<ParticleRendering>& rParticles);

};