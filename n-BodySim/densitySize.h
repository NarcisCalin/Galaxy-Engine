#pragma once
#include "raylib.h"
#include "particle.h"
#include <vector>
#include <algorithm>

struct DensitySize {

	float minSize = 0.17f;
	float maxSize = 0.42f;

	float maxSizeAcc = 60.0f;
	float minSizeAcc = 0.0f;


	void sizeByDensity(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, bool& isDensitySizeEnabled,
		float& sizeMultiplier) {

		if (isDensitySizeEnabled) {
#pragma omp parallel for schedule(dynamic)
			
			for (size_t i = 0; i < pParticles.size(); i++) {
				if (!rParticles[i].isSolid) {

					float particleAccSq = pParticles[i].acc.x * pParticles[i].acc.x +
						pParticles[i].acc.y * pParticles[i].acc.y;

					float clampedAcc = std::clamp(sqrt(particleAccSq), minSizeAcc, maxSizeAcc);
					float normalizedAcc = clampedAcc / maxSizeAcc;

					rParticles[i].size = Lerp(maxSize * sizeMultiplier, minSize * sizeMultiplier, normalizedAcc);
				}
			}
		}
	}

};