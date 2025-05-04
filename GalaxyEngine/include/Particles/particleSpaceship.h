#pragma once
#include "../raylib/raylib.h"
#include "particle.h"
#include <vector>

class ParticleSpaceship {
public:

	int gasMultiplier = 2;

	void spaceshipLogic(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, bool& isShipGasEnabled) {

		float lifeDt = GetFrameTime();

		for (size_t i = 0; i < pParticles.size(); ++i) {
			if (rParticles[i].lifeSpan > 0.0f) {
				rParticles[i].lifeSpan -= lifeDt;
			}
		}

		for (size_t i = 0; i < pParticles.size();) {
			if (rParticles[i].lifeSpan <= 0.0f && rParticles[i].lifeSpan != -1.0f) {
				pParticles.erase(pParticles.begin() + i);
				rParticles.erase(rParticles.begin() + i);
			}
			else {
				++i;
			}
		}

		if (pParticles.empty()) {
			isShipEnabled = false;
			return;
		}

		isShipEnabled = (IsKeyDown(KEY_UP)
			|| IsKeyDown(KEY_RIGHT)
			|| IsKeyDown(KEY_DOWN)
			|| IsKeyDown(KEY_LEFT));

		if (!isShipEnabled) return;

		for (size_t i = 0; i < pParticles.size(); i++) {
			if (!rParticles[i].isSelected) {
				continue;
			}

			if (IsKeyDown(KEY_UP)) {
				pParticles[i].acc.y -= acceleration;

				if (isShipGasEnabled) {
					for (int g = 0; g < gasMultiplier; g++) {
						float normalRand = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

						pParticles.emplace_back(ParticlePhysics(
							Vector2{ pParticles[i].pos.x + (4.0f * normalRand - 2.0f), pParticles[i].pos.y + 3.3f },
							Vector2{ pParticles[i].vel.x + (4.0f * normalRand - 2.0f), pParticles[i].vel.y + 10.0f },
							90000000.0f,

							0.24f,
							0.1f,
							1.0f,
							0.02f
						));

						rParticles.emplace_back(ParticleRendering(
							Color{ 128,128,128,100 },
							rParticles[i].size / 2.0f,
							true,
							false,
							false,
							true,
							true,
							false,
							true,
							3.0f
						));
					}
				}
			}
			if (IsKeyDown(KEY_RIGHT)) {
				pParticles[i].acc.x += acceleration;

				if (isShipGasEnabled) {
					for (int g = 0; g < gasMultiplier; g++) {
						float normalRand = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

						pParticles.emplace_back(ParticlePhysics(
							Vector2{ pParticles[i].pos.x - 3.3f, pParticles[i].pos.y + (4.0f * normalRand - 2.0f) },
							Vector2{ pParticles[i].vel.x - 10.0f, pParticles[i].vel.y + (4.0f * normalRand - 2.0f) },
							90000000.0f,

							0.24f,
							0.1f,
							1.0f,
							0.02f
						));

						rParticles.emplace_back(ParticleRendering(
							Color{ 128,128,128,100 },
							rParticles[i].size / 2.0f,
							true,
							false,
							false,
							true,
							true,
							false,
							true,
							3.0f
						));
					}
				}
			}
			if (IsKeyDown(KEY_DOWN)) {
				pParticles[i].acc.y += acceleration;

				if (isShipGasEnabled) {
					for (int g = 0; g < gasMultiplier; g++) {
						float normalRand = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

						pParticles.emplace_back(ParticlePhysics(
							Vector2{ pParticles[i].pos.x + (4.0f * normalRand - 2.0f), pParticles[i].pos.y - 3.3f },
							Vector2{ pParticles[i].vel.x + (4.0f * normalRand - 2.0f), pParticles[i].vel.y - 10.0f },
							90000000.0f,

							0.24f,
							0.1f,
							1.0f,
							0.02f
						));

						rParticles.emplace_back(ParticleRendering(
							Color{ 128,128,128,100 },
							rParticles[i].size / 2.0f,
							true,
							false,
							false,
							true,
							true,
							false,
							true,
							3.0f
						));
					}
				}
			}
			if (IsKeyDown(KEY_LEFT)) {
				pParticles[i].acc.x -= acceleration;

				if (isShipGasEnabled) {
					for (int g = 0; g < gasMultiplier; g++) {
						float normalRand = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

						pParticles.emplace_back(ParticlePhysics(
							Vector2{ pParticles[i].pos.x + 3.3f, pParticles[i].pos.y + (4.0f * normalRand - 2.0f) },
							Vector2{ pParticles[i].vel.x + 10.0f, pParticles[i].vel.y + (4.0f * normalRand - 2.0f) },
							90000000.0f,

							0.24f,
							0.1f,
							1.0f,
							0.02f
						));

						rParticles.emplace_back(ParticleRendering(
							Color{ 128,128,128,100 },
							rParticles[i].size / 2.0f,
							true,
							false,
							false,
							true,
							true,
							false,
							true,
							3.0f

						));
					}
				}
			}
		}
	}

private:
	bool isShipEnabled = false;
	float acceleration = 8.0f;

	int selectionIdx = 0;
};