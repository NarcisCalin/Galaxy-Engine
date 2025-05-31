#pragma once

#include <raylib.h>
#include <glm/glm.hpp>
#include "particle.h"
#include <vector>
#include <algorithm>

struct DensitySize {

	float minSize = 0.17f;
	float maxSize = 0.62f;

	float sizeAcc = 22.0f;

	void sizeByDensity(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles,
		bool& isDensitySizeEnabled, bool& isForceSizeEnabled,
		float& sizeMultiplier) {

		if (isForceSizeEnabled) {
#pragma omp parallel for schedule(dynamic)

			for (size_t i = 0; i < pParticles.size(); i++) {
				if (rParticles[i].isSolid || rParticles[i].isDarkMatter) {
					continue;
				}

				float particleAccSq = pParticles[i].acc.x * pParticles[i].acc.x +
					pParticles[i].acc.y * pParticles[i].acc.y;

				float clampedAcc = std::clamp(sqrt(particleAccSq), 0.0f, sizeAcc);
				float normalizedAcc = clampedAcc / sizeAcc;

				rParticles[i].size = Lerp(maxSize * sizeMultiplier, minSize * sizeMultiplier, normalizedAcc);
			}
		}

		if (isDensitySizeEnabled) {

			std::vector<int> neighborCounts(pParticles.size(), 0);
#pragma omp parallel for schedule(dynamic)
			for (size_t i = 0; i < pParticles.size(); i++) {

				if (rParticles[i].isDarkMatter || rParticles[i].isSolid) {
					continue;
				}

				float normalDensity = std::min(float(rParticles[i].neighbors) / 25, 1.0f);

				rParticles[i].size = Lerp(maxSize * sizeMultiplier, minSize * sizeMultiplier, static_cast<float>(pow(normalDensity, 2)));
			}
		}
	}

};