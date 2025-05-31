#pragma once

#include <glm.hpp>
#include <raylib.h>
#include "particle.h"
#include <vector>
#include <algorithm>

struct ColorVisuals {

	bool solidColor = false;
	bool densityColor = true;
	bool velocityColor = false;
	bool shockwaveColor = false;
	bool forceColor = false;
	bool pressureColor = false;
	bool SPHColor = false;

	bool showDarkMatterEnabled = false;

	bool selectedColor = true;

	int blendMode = 1;

	Color pColor = { 0, 40, 68, 100 };
	Color sColor = { 155, 80, 40, 75 };

	float hue = 180.0f;
	float saturation = 0.8f;
	float value = 0.5f;

	int maxNeighbors = 60;

	float maxColorAcc = 40.0f;
	float minColorAcc = 0.0f;

	float ShockwaveMaxAcc = 18.0f;
	float ShockwaveMinAcc = 0.0f;

	float maxVel = 100.0f;
	float minVel = 0.0f;

	float maxPress = 1000.0f;
	float minPress = 0.0f;

	glm::vec2 prevVel = { 0.0f, 0.0f };

	void particlesColorVisuals(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles) {

		if (solidColor) {
			for (size_t i = 0; i < pParticles.size(); i++) {
				if (!rParticles[i].uniqueColor) {
					rParticles[i].pColor = pColor;
					rParticles[i].color = rParticles[i].pColor;
				}
				else {
					rParticles[i].color = rParticles[i].pColor;
				}
			}
			blendMode = 1;
		}

		if (densityColor) {

			const float invMaxNeighbors = 1.0f / maxNeighbors;

#pragma omp parallel for schedule(dynamic)
			for (int64_t i = 0; i < pParticles.size(); i++) {
				if (rParticles[i].isDarkMatter) {
					continue;
				}

				if (!rParticles[i].uniqueColor) {
					rParticles[i].pColor = pColor;
					rParticles[i].sColor = sColor;
				}

				Color lowDensityColor = rParticles[i].pColor;

				Color highDensityColor = rParticles[i].sColor;

				float normalDensity = std::min(static_cast<float>(rParticles[i].neighbors) * invMaxNeighbors, 1.0f);
				rParticles[i].color = ColorLerp(lowDensityColor, highDensityColor, normalDensity);
			}

			blendMode = 1;
		}

		if (velocityColor) {
#pragma omp parallel for schedule(dynamic)
			for (int64_t i = 0; i < pParticles.size(); i++) {

				float particleVelSq = pParticles[i].vel.x * pParticles[i].vel.x +
					pParticles[i].vel.y * pParticles[i].vel.y;

				float clampedVel = std::clamp(particleVelSq, minVel, maxVel);
				float normalizedVel = clampedVel / maxVel;

				hue = (1.0f - normalizedVel) * 240.0f;
				saturation = 1.0f;
				value = 1.0f;

				rParticles[i].color = ColorFromHSV(hue, saturation, value);
			}

			blendMode = 0;
		}

		if (forceColor) {
			for (size_t i = 0; i < pParticles.size(); i++) {

				if (rParticles[i].isDarkMatter) {
					continue;
				}

				float particleAccSq = pParticles[i].acc.x * pParticles[i].acc.x +
					pParticles[i].acc.y * pParticles[i].acc.y;

				float clampedAcc = std::clamp(sqrt(particleAccSq), minColorAcc, maxColorAcc);
				float normalizedAcc = clampedAcc / maxColorAcc;

				if (!rParticles[i].uniqueColor) {
					rParticles[i].pColor = pColor;
					rParticles[i].sColor = sColor;
				}

				Color lowDensityColor = rParticles[i].pColor;

				Color highDensityColor = rParticles[i].sColor;

				rParticles[i].color = ColorLerp(lowDensityColor, highDensityColor, normalizedAcc);

			}
			blendMode = 1;
		}

		if (shockwaveColor) {
			for (size_t i = 0; i < pParticles.size(); i++) {

				glm::vec2 shockwave = pParticles[i].acc;

				float shockMag = std::sqrt(shockwave.x * shockwave.x + shockwave.y * shockwave.y);

				float clampedShock = std::clamp(shockMag, ShockwaveMinAcc, ShockwaveMaxAcc);
				float normalizedShock = clampedShock / ShockwaveMaxAcc;

				hue = (1.0f - normalizedShock) * 240.0f;
				saturation = 1.0f;
				value = 1.0f;

				rParticles[i].color = ColorFromHSV(hue, saturation, value);
			}
			blendMode = 0;
		}

		if (pressureColor) {
#pragma omp parallel for schedule(dynamic)
			for (int64_t i = 0; i < pParticles.size(); i++) {

				ParticlePhysics& p = pParticles[i];

				float clampedPress = std::clamp(p.press, minPress, maxPress);
				float normalizedPress = clampedPress / maxPress;

				hue = (1.0f - normalizedPress) * 240.0f;
				saturation = 1.0f;
				value = 1.0f;

				rParticles[i].color = ColorFromHSV(hue, saturation, value);
			}

			blendMode = 0;
		}

		if (SPHColor) {
			for (size_t i = 0; i < rParticles.size(); i++) {
				if (!rParticles[i].uniqueColor) {
					rParticles[i].color = rParticles[i].sphColor;
				}
				else {
					rParticles[i].color = rParticles[i].pColor;
				}
			}

			blendMode = 0;
		}




		if (selectedColor) {
			for (size_t i = 0; i < rParticles.size(); i++) {
				if (rParticles[i].isSelected) {
					rParticles[i].color = { 255, 20,20, 255 };
				}
			}
		}

		if (showDarkMatterEnabled) {
			for (size_t i = 0; i < rParticles.size(); i++) {
				if (rParticles[i].isDarkMatter) {
					rParticles[i].color = { 128, 128, 128, 170 };
				}
			}
		}
		else {
			for (size_t i = 0; i < rParticles.size(); i++) {
				if (rParticles[i].isDarkMatter) {
					rParticles[i].color = { 0, 0, 0, 0 };
				}
			}
		}
	}
};