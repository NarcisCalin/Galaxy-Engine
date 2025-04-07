#pragma once
#include "raylib.h"
#include "particle.h"
#include <vector>
#include <algorithm>

struct DensitySize {

	float sizeDensityRadius = 4.0f;
	int sizeMaxNeighbors = 60;

	float minSize = 0.1f;
	float maxSize = 0.25f;


	void sizeByDensity(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, bool& isDensitySizeEnabled,
		float& sizeMultiplier) {
		if (isDensitySizeEnabled) {
			float densityRadiusSq = sizeDensityRadius * sizeDensityRadius;
			std::vector<int> neighborCounts(pParticles.size(), 0);

#pragma omp parallel for schedule(dynamic)
			
			for (size_t i = 0; i < pParticles.size(); i++) {
				if (!rParticles[i].isSolid) {
					float maxAcc = 300.0f;
					float minAcc = 0.0f;

					float particleAccSq = pParticles[i].acc.x * pParticles[i].acc.x +
						pParticles[i].acc.y * pParticles[i].acc.y;

					float clampedAcc = std::clamp(particleAccSq, minAcc, maxAcc);
					float normalizedAcc = clampedAcc / maxAcc;

					rParticles[i].size = Lerp(maxSize * sizeMultiplier, minSize * sizeMultiplier, log(normalizedAcc));
				}
			}
		}
	}

};