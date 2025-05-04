#pragma once

#include "../raylib/raylib.h"
#include "particle.h"
#include <vector>
#include <algorithm>

struct ColorVisuals {

	bool solidColor = false;
	bool densityColor = true;
	bool velocityColor = false;
	bool deltaVColor = false;
	bool forceColor = false;

	bool showDarkMatterEnabled = false;
	bool previousDarkMatterState = false;

	bool selectedColor = true;

	int blendMode = 1;

	int primaryR = 0;
	int primaryG = 40;
	int primaryB = 68;
	int primaryA = 100;

	int secondaryR = 155;
	int secondaryG = 80;
	int secondaryB = 40;
	int secondaryA = 75;

	float hue = 180.0f;
	float saturation = 0.8f;
	float value = 0.5f;

	int maxNeighbors = 60;

	float maxColorAcc = 40.0f;
	float minColorAcc = 0.0f;

	Vector2 prevVel = { 0.0f, 0.0f };


	Color ColorLerp(const Color& a, const Color& b, float t) {
		return {
			static_cast<unsigned char>(a.r + (b.r - a.r) * t),
			static_cast<unsigned char>(a.g + (b.g - a.g) * t),
			static_cast<unsigned char>(a.b + (b.b - a.b) * t),
			static_cast<unsigned char>(a.a + (b.a - a.a) * t)
		};
	}

	void particlesColorVisuals(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, 
		float& particleSizeMultiplier, float& particleTextureHalfSize) {

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

			Color lowDensityColor = {
				static_cast<unsigned char>(primaryR),
				static_cast<unsigned char>(primaryG),
				static_cast<unsigned char>(primaryB),
				static_cast<unsigned char>(primaryA)
			};

			Color highDensityColor = {
				static_cast<unsigned char>(secondaryR),
				static_cast<unsigned char>(secondaryG),
				static_cast<unsigned char>(secondaryB),
				static_cast<unsigned char>(secondaryA)
			};

			const float invMaxNeighbors = 1.0f / maxNeighbors;

#pragma omp parallel for schedule(dynamic)
			for (size_t i = 0; i < pParticles.size(); i++) {
				if (rParticles[i].isDarkMatter || rParticles[i].uniqueColor) {
					continue;
				}
				const ParticlePhysics pParticle = pParticles[i];
				float normalDensity = std::min(static_cast<float>(rParticles[i].neighbors) * invMaxNeighbors, 1.0f);
				rParticles[i].color = ColorLerp(lowDensityColor, highDensityColor, normalDensity);
			}

			blendMode = 1;
		}
		else if (velocityColor) {
#pragma omp parallel for schedule(dynamic)
			for (size_t i = 0; i < pParticles.size(); i++) {
				float maxVel = 11000.0f;
				float minVel = 0.0f;


				float particleVelSq = pParticles[i].vel.x * pParticles[i].vel.x +
					pParticles[i].vel.y * pParticles[i].vel.y;

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

				if (rParticles[i].uniqueColor) {
					continue;
				}

				float particleAccSq = pParticles[i].acc.x * pParticles[i].acc.x +
					pParticles[i].acc.y * pParticles[i].acc.y;

				float clampedAcc = std::clamp(sqrt(particleAccSq), minColorAcc, maxColorAcc);
				float normalizedAcc = clampedAcc / maxColorAcc;

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
			blendMode = 1;
		}
		else if (deltaVColor) {
			for (size_t i = 0; i < pParticles.size(); i++) {
				float maxAccel = 5.0f;
				float minAccel = 0.0f;

				Vector2 delta = pParticles[i].vel - pParticles[i].prevVel;

				float deltaMag = std::sqrt(delta.x * delta.x + delta.y * delta.y);

				float clampedDelta = std::clamp(deltaMag, minAccel, maxAccel);
				float normalizedVel = clampedDelta / maxAccel;

				hue = (1.0f - normalizedVel) * 240.0f;
				saturation = 1.0f;
				value = 1.0f;

				if (!rParticles[i].uniqueColor) {
					rParticles[i].color = ColorFromHSV(hue, saturation, value);
				}

				pParticles[i].prevVel = pParticles[i].vel;

				blendMode = 0;
			}
		}


		if (selectedColor) {
			for (size_t i = 0; i < rParticles.size(); i++) {
				if (rParticles[i].isSelected && !rParticles[i].uniqueColor) {
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

		if (showDarkMatterEnabled != previousDarkMatterState) {
			previousDarkMatterState = showDarkMatterEnabled;

			for (size_t i = 0; i < rParticles.size(); i++) {
				if (rParticles[i].isDarkMatter) {
					if (!showDarkMatterEnabled) {

						rParticles[i].color = { 0, 0, 0, 0 };
					}
				}
			}
		}
	}
};