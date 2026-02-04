#pragma once

#include "Particles/particle.h"

struct DensitySize {

	float minSize = 0.17f;
	float maxSize = 0.62f;

	float sizeAcc = 22.0f;

	int maxNeighbors = 200;

	void sizeByDensity(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, 
		std::vector<ParticlePhysics3D>& pParticles3D, std::vector<ParticleRendering3D>& rParticles3D,
		bool& isDensitySizeEnabled, bool& isForceSizeEnabled,
		float& sizeMultiplier, bool is3DMode) {

		if (isForceSizeEnabled) {
			if (!is3DMode) {
#pragma omp parallel for schedule(dynamic)

				for (int64_t i = 0; i < pParticles.size(); i++) {
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
			else {
				for (int64_t i = 0; i < pParticles3D.size(); i++) {
					if (rParticles3D[i].isSolid || rParticles3D[i].isDarkMatter) {
						continue;
					}

					float particleAccSq = pParticles3D[i].acc.x * pParticles3D[i].acc.x +
						pParticles3D[i].acc.y * pParticles3D[i].acc.y;

					float clampedAcc = std::clamp(sqrt(particleAccSq), 0.0f, sizeAcc);
					float normalizedAcc = clampedAcc / sizeAcc;

					rParticles3D[i].size = Lerp(maxSize * sizeMultiplier, minSize * sizeMultiplier, normalizedAcc);
				}
			}
		}

		if (isDensitySizeEnabled) {

			if (!is3DMode) {
				std::vector<int> neighborCounts(pParticles.size(), 0);
#pragma omp parallel for schedule(dynamic)
				for (int64_t i = 0; i < pParticles.size(); i++) {

					if (rParticles[i].isDarkMatter || rParticles[i].isSolid) {
						continue;
					}

					float normalDensity = std::min(float(rParticles[i].neighbors) / maxNeighbors, 1.0f);

					rParticles[i].size = Lerp(maxSize * sizeMultiplier, minSize * sizeMultiplier, static_cast<float>(pow(normalDensity, 2)));
				}
			}
			else {
				std::vector<int> neighborCounts(pParticles3D.size(), 0);
#pragma omp parallel for schedule(dynamic)
				for (int64_t i = 0; i < pParticles3D.size(); i++) {

					if (rParticles3D[i].isDarkMatter || rParticles3D[i].isSolid) {
						continue;
					}

					float normalDensity = std::min(float(rParticles3D[i].neighbors) / maxNeighbors, 1.0f);

					rParticles3D[i].size = Lerp(maxSize * sizeMultiplier, minSize * sizeMultiplier, static_cast<float>(pow(normalDensity, 2)));
				}
			}
		}
	}

};