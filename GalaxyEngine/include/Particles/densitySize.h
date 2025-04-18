#pragma once

#include "../raylib/raylib.h"
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
				if (rParticles[i].isSolid) {
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

			float densityRadiusSq = 4.5f * 4.5f;

			std::vector<int> neighborCounts(pParticles.size(), 0);
#pragma omp parallel for schedule(dynamic)
			for (size_t i = 0; i < pParticles.size(); i++) {

				if (rParticles[i].isDarkMatter || rParticles[i].isSolid) {
					continue;
				}

				const auto& pParticle = pParticles[i];
				for (size_t j = i + 1; j < pParticles.size(); j++) {

					if (rParticles[i].isDarkMatter || rParticles[i].isSolid) {
						continue;
					}

					if (std::abs(pParticles[j].pos.x - pParticle.pos.x) > 4.5f) break;
					float dx = pParticle.pos.x - pParticles[j].pos.x;
					float dy = pParticle.pos.y - pParticles[j].pos.y;
					if (dx * dx + dy * dy < densityRadiusSq) {
						neighborCounts[i]++;
						neighborCounts[j]++;
					}
				}

				float normalDensity = std::min(float(neighborCounts[i]) / 25, 1.0f);
				float invertedDensity = 1.0f - normalDensity;

				rParticles[i].size = Lerp(maxSize * sizeMultiplier, minSize * sizeMultiplier, static_cast<float>(pow(normalDensity, 2)));
			}
		}
	}

};