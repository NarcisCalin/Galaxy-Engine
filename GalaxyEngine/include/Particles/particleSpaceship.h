#pragma once

#include "Particles/particle.h"
#include "Physics/materialsSPH.h"
#include "IO/io.h"

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

		isShipEnabled = (IO::shortcutDown(KEY_UP)
			|| IO::shortcutDown(KEY_RIGHT)
			|| IO::shortcutDown(KEY_DOWN)
			|| IO::shortcutDown(KEY_LEFT));

		if (!isShipEnabled) {
			return;
		};

		for (size_t i = 0; i < pParticles.size(); i++) {
			if (!rParticles[i].isSelected) {
				continue;
			}
			else {

				// I set it to rock's mass

				pParticles[i].mass = 8500000000.0f * rock.massMult;
				pParticles[i].sphMass = rock.massMult;
			}


			if (IO::shortcutDown(KEY_UP)) {
				pParticles[i].acc.y -= acceleration;

				if (isShipGasEnabled) {
					for (int g = 0; g < gasMultiplier; g++) {
						float normalRand = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

						pParticles.emplace_back(ParticlePhysics(
							glm::vec2{ pParticles[i].pos.x + (4.0f * normalRand - 2.0f), pParticles[i].pos.y + 3.3f },
							glm::vec2{ pParticles[i].vel.x + (4.0f * normalRand - 2.0f), pParticles[i].vel.y + 10.0f },
							8500000000.0f * water.massMult,

							water.restDens,
							water.stiff,
							water.visc,
							water.cohesion
						));

						rParticles.emplace_back(ParticleRendering(
							water.color,
							rParticles[i].size * 0.7f,
							false,
							false,
							false,
							true,
							true,
							false,
							true,
							3.0f,
							water.id
						));

						rParticles.back().sphColor = water.color;
						pParticles.back().temp = 440.0f;
					}
				}
			}
			if (IO::shortcutDown(KEY_RIGHT)) {
				pParticles[i].acc.x += acceleration;

				if (isShipGasEnabled) {
					for (int g = 0; g < gasMultiplier; g++) {
						float normalRand = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

						pParticles.emplace_back(ParticlePhysics(
							glm::vec2{ pParticles[i].pos.x - 3.3f, pParticles[i].pos.y + (4.0f * normalRand - 2.0f) },
							glm::vec2{ pParticles[i].vel.x - 10.0f, pParticles[i].vel.y + (4.0f * normalRand - 2.0f) },
							8500000000.0f * water.massMult,

							water.restDens,
							water.stiff,
							water.visc,
							water.cohesion
						));

						rParticles.emplace_back(ParticleRendering(
							water.color,
							rParticles[i].size * 0.7f,
							false,
							false,
							false,
							true,
							true,
							false,
							true,
							3.0f,
							water.id
						));

						rParticles.back().sphColor = water.color;
						pParticles.back().temp = 440.0f;
					}
				}
			}
			if (IO::shortcutDown(KEY_DOWN)) {
				pParticles[i].acc.y += acceleration;

				if (isShipGasEnabled) {
					for (int g = 0; g < gasMultiplier; g++) {
						float normalRand = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

						pParticles.emplace_back(ParticlePhysics(
							glm::vec2{ pParticles[i].pos.x + (4.0f * normalRand - 2.0f), pParticles[i].pos.y - 3.3f },
							glm::vec2{ pParticles[i].vel.x + (4.0f * normalRand - 2.0f), pParticles[i].vel.y - 10.0f },
							8500000000.0f * water.massMult,

							water.restDens,
							water.stiff,
							water.visc,
							water.cohesion
						));

						rParticles.emplace_back(ParticleRendering(
							water.color,
							rParticles[i].size * 0.7f,
							false,
							false,
							false,
							true,
							true,
							false,
							true,
							3.0f,
							water.id
						));

						rParticles.back().sphColor = water.color;
						pParticles.back().temp = 440.0f;
					}
				}
			}
			if (IO::shortcutDown(KEY_LEFT)) {
				pParticles[i].acc.x -= acceleration;

				if (isShipGasEnabled) {
					for (int g = 0; g < gasMultiplier; g++) {
						float normalRand = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

						pParticles.emplace_back(ParticlePhysics(
							glm::vec2{ pParticles[i].pos.x + 3.3f, pParticles[i].pos.y + (4.0f * normalRand - 2.0f) },
							glm::vec2{ pParticles[i].vel.x + 10.0f, pParticles[i].vel.y + (4.0f * normalRand - 2.0f) },
							8500000000.0f * water.massMult,

							water.restDens,
							water.stiff,
							water.visc,
							water.cohesion
						));

						rParticles.emplace_back(ParticleRendering(
							water.color,
							rParticles[i].size * 0.7f,
							false,
							false,
							false,
							true,
							true,
							false,
							true,
							3.0f,
							water.id
						));

						rParticles.back().sphColor = water.color;
						pParticles.back().temp = 440.0f;
					}
				}
			}
		}
	}

private:

	SPHWater water;
	SPHRock rock;

	bool isShipEnabled = false;
	float acceleration = 8.0f;

	int selectionIdx = 0;
};