#pragma once
#include "raylib.h"
#include "planet.h"
#include <vector>

struct DensitySize {

	float sizeDensityRadius = 12.0f;
	int sizeMaxNeightbors = 170;

	void sizeByDensity(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles) {
		float densityRadiusSq = sizeDensityRadius * sizeDensityRadius;
		std::vector<int> neighborCounts(pParticles.size(), 0);

#pragma omp parallel for schedule(dynamic)
		for (size_t i = 0; i < pParticles.size(); i++) {
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
		}

		for (size_t i = 0; i < pParticles.size(); i++) {
			float normalDensity = std::min(float(neighborCounts[i]) / sizeMaxNeightbors, 1.0f);
			float invertedDensity = std::max(1.0f - normalDensity, 0.35f);

			rParticles[i].size = invertedDensity * 0.5f;
		}
	}

};