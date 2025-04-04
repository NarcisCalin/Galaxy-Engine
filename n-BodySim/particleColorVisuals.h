#pragma once
#include "raylib.h"
#include "planet.h"
#include <vector>
#include <algorithm>

struct ColorVisuals {

	bool solidColor = false;
	bool densityColor = true;
	bool velocityColor = false;

	bool selectedColor = false;

	int blendMode = 1;

	int densityR = 225;
	int densityG = 120;
	int densityB = 125;
	int densityA = 200;

	float densityRadius = 2.4f;
	int maxNeighbors = 60;

	void particlesColorVisuals(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles) {

		if (solidColor) {
			for (size_t i = 0; i < pParticles.size(); i++) {
				if (!rParticles[i].uniqueColor) {
					rParticles[i].color.r = static_cast<unsigned char>(densityR);
					rParticles[i].color.g = static_cast<unsigned char>(densityG);
					rParticles[i].color.b = static_cast<unsigned char>(densityB);
					rParticles[i].color.a = densityA;
				}
			}
			blendMode = 1;
		}
		else if (densityColor) {

			float densityRadiusSq = densityRadius * densityRadius;

			std::vector<int> neighborCounts(pParticles.size(), 0);

#pragma omp parallel for schedule(dynamic)
			for (size_t i = 0; i < pParticles.size(); i++) {
				const auto& pParticle = pParticles[i];
				for (size_t j = i + 1; j < pParticles.size(); j++) {
					if (std::abs(pParticles[j].pos.x - pParticle.pos.x) > densityRadius) break;
					float dx = pParticle.pos.x - pParticles[j].pos.x;
					float dy = pParticle.pos.y - pParticles[j].pos.y;
					if (dx * dx + dy * dy < densityRadiusSq) {
						neighborCounts[i]++;
						neighborCounts[j]++;
					}
				}

				float normalDensity = std::min(float(neighborCounts[i]) / maxNeighbors, 1.0f);
				float invertedDensity = 1.0f - normalDensity;

				if (!rParticles[i].uniqueColor) {
					rParticles[i].color.r = static_cast<unsigned char>(normalDensity * densityR);
					rParticles[i].color.g = static_cast<unsigned char>(normalDensity * densityG);
					rParticles[i].color.b = static_cast<unsigned char>(invertedDensity * densityB);
					rParticles[i].color.a = densityA;
				}
			}
			blendMode = 1;
		}
		else if (velocityColor) {
#pragma omp parallel for schedule(dynamic)
			for (size_t i = 0; i < pParticles.size(); i++) {
				float maxVel = 11000.0f;
				float minVel = 0.0f;


				float particleVelSq = pParticles[i].velocity.x * pParticles[i].velocity.x +
					pParticles[i].velocity.y * pParticles[i].velocity.y;

				float clampedVel = std::clamp(particleVelSq, minVel, maxVel);
				float normalizedVel = clampedVel / maxVel;

				float hue = (1.0f - normalizedVel) * 240.0f;
				float saturation = 1.0f;
				float value = 1.0f;

				if (!rParticles[i].uniqueColor) {
					rParticles[i].color = ColorFromHSV(hue, saturation, value);
				}
			}

			blendMode = 0;
		}

		if (selectedColor) {
			for (size_t i = 0; i < rParticles.size(); i++) {
				if (rParticles[i].isSelected) {
					rParticles[i].color = { 230, 128,128, 180 };
				}
			}

		}
	}
};