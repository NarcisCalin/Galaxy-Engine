#pragma once
#include "raylib.h"
#include "particle.h"
#include <vector>
#include <algorithm>

struct ColorVisuals {

	bool solidColor = false;
	bool densityColor = true;
	bool velocityColor = false;
	bool forceColor = false;

	bool selectedColor = true;

	int blendMode = 1;

	int primaryR = 0;
	int primaryG = 40;
	int primaryB = 120;
	int primaryA = 50;

	int secondaryR = 155;
	int secondaryG = 80;
	int secondaryB = 40;
	int secondaryA = 75;

	float hue = 180.0f;
	float saturation = 0.8f;
	float value = 0.5f;

	float densityRadius = 4.5f;
	int maxNeighbors = 60;

	float maxColorAcc = 40.0f;
	float minColorAcc = 0.0f;

	void particlesColorVisuals(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles) {

		if (solidColor) {
			for (size_t i = 0; i < pParticles.size(); i++) {
				if (!rParticles[i].uniqueColor) {
					rParticles[i].color.r = static_cast<unsigned char>(primaryR);
					rParticles[i].color.g = static_cast<unsigned char>(primaryG);
					rParticles[i].color.b = static_cast<unsigned char>(primaryB);
					rParticles[i].color.a = primaryA;
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
					Color lowDensityColor = {
						static_cast<unsigned char>(primaryR),
						static_cast<unsigned char>(primaryG),
						static_cast<unsigned char>(primaryB),
						static_cast<unsigned char>(primaryA) };

					Color highDensityColor = {
						static_cast<unsigned char>(secondaryR),
						static_cast<unsigned char>(secondaryG),
						static_cast<unsigned char>(secondaryB),
						static_cast<unsigned char>(secondaryA) };

					rParticles[i].color = ColorLerp(lowDensityColor, highDensityColor, normalDensity);
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

				hue = (1.0f - normalizedVel) * 240.0f;
				saturation = 1.0f;
				value = 1.0f;

				if (!rParticles[i].uniqueColor) {
					rParticles[i].color = ColorFromHSV(hue, saturation, value);
				}
			}

			blendMode = 0;
		}
		else if (forceColor) {
			for (size_t i = 0; i < pParticles.size(); i++) {

				float particleAccSq = pParticles[i].acc.x * pParticles[i].acc.x +
					pParticles[i].acc.y * pParticles[i].acc.y;

				float clampedAcc = std::clamp(sqrt(particleAccSq), minColorAcc, maxColorAcc);
				float normalizedAcc = clampedAcc / maxColorAcc;

				if (!rParticles[i].uniqueColor) {
					Color lowDensityColor = {
						static_cast<unsigned char>(primaryR),
						static_cast<unsigned char>(primaryG),
						static_cast<unsigned char>(primaryB),
						static_cast<unsigned char>(primaryA) };

					Color highDensityColor = {
						static_cast<unsigned char>(secondaryR),
						static_cast<unsigned char>(secondaryG),
						static_cast<unsigned char>(secondaryB),
						static_cast<unsigned char>(secondaryA) };

					rParticles[i].color = ColorLerp(lowDensityColor, highDensityColor, normalizedAcc);
				}
			}
		}
		if (selectedColor) {
			for (size_t i = 0; i < rParticles.size(); i++) {
				if (rParticles[i].isSelected) {
					rParticles[i].color = { 230, 128,128, 30 };
				}
			}
			blendMode = 1;
		}
	}
};