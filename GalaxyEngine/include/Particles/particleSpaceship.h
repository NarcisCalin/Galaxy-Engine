#pragma once

#include "Particles/particle.h"
#include "Physics/materialsSPH.h"

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


			if (IsKeyDown(KEY_UP)) {
				pParticles[i].acc.y -= acceleration;

				if (isShipGasEnabled) {
					for (int g = 0; g < gasMultiplier; g++) {
						float normalRand = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

						pParticles.emplace_back(ParticlePhysics(
							glm::vec2{ pParticles[i].pos.x + (4.0f * normalRand - 2.0f), pParticles[i].pos.y + 3.3f },
							glm::vec2{ pParticles[i].vel.x + (4.0f * normalRand - 2.0f), pParticles[i].vel.y + 10.0f },
							8500000000.0f * gas.massMult,

							gas.restDens,
							gas.stiff,
							gas.visc,
							gas.cohesion
						));

						rParticles.emplace_back(ParticleRendering(
							gas.color,
							rParticles[i].size * 0.7f,
							false,
							false,
							false,
							true,
							true,
							false,
							true,
							3.0f,
							gas.sphLabel
						));

						rParticles.back().sphColor = gas.color;
					}
				}
			}
			if (IsKeyDown(KEY_RIGHT)) {
				pParticles[i].acc.x += acceleration;

				if (isShipGasEnabled) {
					for (int g = 0; g < gasMultiplier; g++) {
						float normalRand = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

						pParticles.emplace_back(ParticlePhysics(
							glm::vec2{ pParticles[i].pos.x - 3.3f, pParticles[i].pos.y + (4.0f * normalRand - 2.0f) },
							glm::vec2{ pParticles[i].vel.x - 10.0f, pParticles[i].vel.y + (4.0f * normalRand - 2.0f) },
							8500000000.0f * gas.massMult,

							gas.restDens,
							gas.stiff,
							gas.visc,
							gas.cohesion
						));

						rParticles.emplace_back(ParticleRendering(
							gas.color,
							rParticles[i].size * 0.7f,
							false,
							false,
							false,
							true,
							true,
							false,
							true,
							3.0f,
							gas.sphLabel
						));

						rParticles.back().sphColor = gas.color;
					}
				}
			}
			if (IsKeyDown(KEY_DOWN)) {
				pParticles[i].acc.y += acceleration;

				if (isShipGasEnabled) {
					for (int g = 0; g < gasMultiplier; g++) {
						float normalRand = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

						pParticles.emplace_back(ParticlePhysics(
							glm::vec2{ pParticles[i].pos.x + (4.0f * normalRand - 2.0f), pParticles[i].pos.y - 3.3f },
							glm::vec2{ pParticles[i].vel.x + (4.0f * normalRand - 2.0f), pParticles[i].vel.y - 10.0f },
							8500000000.0f * gas.massMult,

							gas.restDens,
							gas.stiff,
							gas.visc,
							gas.cohesion
						));

						rParticles.emplace_back(ParticleRendering(
							gas.color,
							rParticles[i].size * 0.7f,
							false,
							false,
							false,
							true,
							true,
							false,
							true,
							3.0f,
							gas.sphLabel
						));

						rParticles.back().sphColor = gas.color;
					}
				}
			}
			if (IsKeyDown(KEY_LEFT)) {
				pParticles[i].acc.x -= acceleration;

				if (isShipGasEnabled) {
					for (int g = 0; g < gasMultiplier; g++) {
						float normalRand = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

						pParticles.emplace_back(ParticlePhysics(
							glm::vec2{ pParticles[i].pos.x + 3.3f, pParticles[i].pos.y + (4.0f * normalRand - 2.0f) },
							glm::vec2{ pParticles[i].vel.x + 10.0f, pParticles[i].vel.y + (4.0f * normalRand - 2.0f) },
							8500000000.0f * gas.massMult,

							gas.restDens,
							gas.stiff,
							gas.visc,
							gas.cohesion
						));

						rParticles.emplace_back(ParticleRendering(
							gas.color,
							rParticles[i].size * 0.7f,
							false,
							false,
							false,
							true,
							true,
							false,
							true,
							3.0f,
							gas.sphLabel
						));

						rParticles.back().sphColor = gas.color;
					}
				}
			}
		}
	}

private:

	SPHGas gas;
	SPHRock rock;

	bool isShipEnabled = false;
	float acceleration = 8.0f;

	int selectionIdx = 0;
};