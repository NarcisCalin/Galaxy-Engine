#pragma once
#include "raylib.h"
#include "particle.h"
#include <vector>

struct DensitySize {

	float sizeDensityRadius = 4.0f;
	int sizeMaxNeightbors = 60;

	void sizeByDensity(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, bool& isDensitySizeEnabled,
		float& sizeMultiplier) {
		if (isDensitySizeEnabled) {
			float densityRadiusSq = sizeDensityRadius * sizeDensityRadius;
			std::vector<int> neighborCounts(pParticles.size(), 0);

#pragma omp parallel for schedule(dynamic)
			for (size_t i = 0; i < pParticles.size(); i++) {
				if (!rParticles[i].isSolid) {
					const auto& pParticle = pParticles[i];
					for (size_t j = i + 1; j < pParticles.size(); j++) {
						if (std::abs(pParticles[j].pos.x - pParticle.pos.x) > sizeDensityRadius) break;
						float dx = pParticle.pos.x - pParticles[j].pos.x;
						float dy = pParticle.pos.y - pParticles[j].pos.y;
						if (dx * dx + dy * dy < densityRadiusSq) {
							neighborCounts[i]++;
							neighborCounts[j]++;
						}
					}

					float normalDensity = std::min(float(neighborCounts[i]) / sizeMaxNeightbors, 1.0f);
					float invertedDensity = std::max(1.0f - normalDensity, 0.22f);

					rParticles[i].size = invertedDensity * sizeMultiplier * 0.65f;
				}
			}
		}
	}

};