#pragma once

#include "Particles/particle.h"
#include "Physics/materialsSPH.h"
#include "IO/io.h"

class ParticleSpaceship {
public:

	int gasMultiplier = 1;

	bool isShipEnabled = false;

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
							8500000000.0f * water.massMult * 0.1f,

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
							8500000000.0f * water.massMult * 0.1f,

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
							8500000000.0f * water.massMult * 0.1f,

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
							8500000000.0f * water.massMult * 0.1f,

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

	float arrowLength = 10.0f;

	void spaceshipLogic3D(std::vector<ParticlePhysics3D>& pParticles, std::vector<ParticleRendering3D>& rParticles, bool& isShipGasEnabled, Camera3D& cam3D) {

		float lifeDt = GetFrameTime();

		glm::vec3 camPos(cam3D.position.x, cam3D.position.y, cam3D.position.z);
		glm::vec3 camTarget(cam3D.target.x, cam3D.target.y, cam3D.target.z);
		glm::vec3 camUpGlobal(0.0f, 1.0f, 0.0f);

		glm::vec3 forwardDir = glm::normalize(camTarget - camPos);

		glm::vec3 rightDir = glm::normalize(glm::cross(forwardDir, camUpGlobal));

		glm::vec3 upDir = glm::normalize(glm::cross(rightDir, forwardDir));

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
			isShipGasEnabled = false;
			return;
		}

		if (!isShipEnabled) {
			return;
		}

		for (size_t i = 0; i < pParticles.size(); i++) {
			if (!rParticles[i].isSelected) {
				continue;
			}

			// Set mass to rock's mass
			pParticles[i].mass = 8500000000.0f * rock.massMult;
			pParticles[i].sphMass = rock.massMult;

			DrawLine3D({ pParticles[i].pos.x,pParticles[i].pos.y, pParticles[i].pos.z },
				{ pParticles[i].pos.x,pParticles[i].pos.y, pParticles[i].pos.z - arrowLength - arrowLength * 0.5f },
				RED);

			// UP ARROW - Front (forward in Z)
			if (IO::shortcutDown(KEY_UP)) {

				DrawCube({ pParticles[i].pos.x,pParticles[i].pos.y, pParticles[i].pos.z - arrowLength * 0.5f },
					rParticles[i].totalRadius * 0.5f,
					rParticles[i].totalRadius * 0.5f,
					arrowLength,
					RED);
				DrawCylinderEx({ pParticles[i].pos.x,pParticles[i].pos.y, pParticles[i].pos.z - arrowLength },
					{ pParticles[i].pos.x,pParticles[i].pos.y, pParticles[i].pos.z - arrowLength - arrowLength * 0.5f },
					rParticles[i].totalRadius * 2.0f,
					rParticles[i].totalRadius * 0.1f,
					12,
					RED);

				pParticles[i].acc.z -= acceleration;

				if (isShipGasEnabled) {
					for (int g = 0; g < gasMultiplier; g++) {
						float normalRand1 = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
						float normalRand2 = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

						pParticles.emplace_back(ParticlePhysics3D(
							glm::vec3{ pParticles[i].pos.x + (4.0f * normalRand1 - 2.0f), pParticles[i].pos.y + (4.0f * normalRand2 - 2.0f), pParticles[i].pos.z + 3.3f },
							glm::vec3{ pParticles[i].vel.x + (4.0f * normalRand1 - 2.0f), pParticles[i].vel.y + (4.0f * normalRand2 - 2.0f), pParticles[i].vel.z + 10.0f },
							8500000000.0f * water.massMult * 0.1f,
							water.restDens,
							water.stiff,
							water.visc,
							water.cohesion
						));

						rParticles.emplace_back(ParticleRendering3D(
							water.color,
							rParticles[i].size * 0.7f,
							false, false, false, true, true, false, true,
							3.0f,
							water.id
						));

						rParticles.back().sphColor = water.color;
						pParticles.back().temp = 440.0f;
					}
				}
			}

			// RIGHT ARROW - Right
			if (IO::shortcutDown(KEY_RIGHT)) {

				DrawCube({ pParticles[i].pos.x + arrowLength * 0.5f, pParticles[i].pos.y, pParticles[i].pos.z },
					arrowLength,
					rParticles[i].totalRadius * 0.5f,
					rParticles[i].totalRadius * 0.5f,
					GREEN);
				DrawCylinderEx({ pParticles[i].pos.x + arrowLength, pParticles[i].pos.y, pParticles[i].pos.z },
					{ pParticles[i].pos.x + arrowLength + arrowLength * 0.5f, pParticles[i].pos.y, pParticles[i].pos.z },
					rParticles[i].totalRadius * 2.0f,
					rParticles[i].totalRadius * 0.1f,
					12,
					GREEN);

				pParticles[i].acc.x += acceleration;

				if (isShipGasEnabled) {
					for (int g = 0; g < gasMultiplier; g++) {
						float normalRand1 = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
						float normalRand2 = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

						pParticles.emplace_back(ParticlePhysics3D(
							glm::vec3{ pParticles[i].pos.x - 3.3f, pParticles[i].pos.y + (4.0f * normalRand1 - 2.0f), pParticles[i].pos.z + (4.0f * normalRand2 - 2.0f) },
							glm::vec3{ pParticles[i].vel.x - 10.0f, pParticles[i].vel.y + (4.0f * normalRand1 - 2.0f), pParticles[i].vel.z + (4.0f * normalRand2 - 2.0f) },
							8500000000.0f * water.massMult * 0.1f,
							water.restDens,
							water.stiff,
							water.visc,
							water.cohesion
						));

						rParticles.emplace_back(ParticleRendering3D(
							water.color,
							rParticles[i].size * 0.7f,
							false, false, false, true, true, false, true,
							3.0f,
							water.id
						));

						rParticles.back().sphColor = water.color;
						pParticles.back().temp = 440.0f;
					}
				}
			}

			// DOWN ARROW - Back
			if (IO::shortcutDown(KEY_DOWN)) {

				DrawCube({ pParticles[i].pos.x, pParticles[i].pos.y, pParticles[i].pos.z + arrowLength * 0.5f },
					rParticles[i].totalRadius * 0.5f,
					rParticles[i].totalRadius * 0.5f,
					arrowLength,
					RED);
				DrawCylinderEx({ pParticles[i].pos.x, pParticles[i].pos.y, pParticles[i].pos.z + arrowLength },
					{ pParticles[i].pos.x, pParticles[i].pos.y, pParticles[i].pos.z + arrowLength + arrowLength * 0.5f },
					rParticles[i].totalRadius * 2.0f,
					rParticles[i].totalRadius * 0.1f,
					12,
					RED);

				pParticles[i].acc.z += acceleration;

				if (isShipGasEnabled) {
					for (int g = 0; g < gasMultiplier; g++) {
						float normalRand1 = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
						float normalRand2 = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

						pParticles.emplace_back(ParticlePhysics3D(
							glm::vec3{ pParticles[i].pos.x + (4.0f * normalRand1 - 2.0f), pParticles[i].pos.y + (4.0f * normalRand2 - 2.0f), pParticles[i].pos.z - 3.3f },
							glm::vec3{ pParticles[i].vel.x + (4.0f * normalRand1 - 2.0f), pParticles[i].vel.y + (4.0f * normalRand2 - 2.0f), pParticles[i].vel.z - 10.0f },
							8500000000.0f * water.massMult * 0.1f,
							water.restDens,
							water.stiff,
							water.visc,
							water.cohesion
						));

						rParticles.emplace_back(ParticleRendering3D(
							water.color,
							rParticles[i].size * 0.7f,
							false, false, false, true, true, false, true,
							3.0f,
							water.id
						));

						rParticles.back().sphColor = water.color;
						pParticles.back().temp = 440.0f;
					}
				}
			}

			// LEFT ARROW - Left
			if (IO::shortcutDown(KEY_LEFT)) {

				DrawCube({ pParticles[i].pos.x - arrowLength * 0.5f, pParticles[i].pos.y, pParticles[i].pos.z },
					arrowLength,
					rParticles[i].totalRadius * 0.5f,
					rParticles[i].totalRadius * 0.5f,
					GREEN);
				DrawCylinderEx({ pParticles[i].pos.x - arrowLength, pParticles[i].pos.y, pParticles[i].pos.z },
					{ pParticles[i].pos.x - arrowLength - arrowLength * 0.5f, pParticles[i].pos.y, pParticles[i].pos.z },
					rParticles[i].totalRadius * 2.0f,
					rParticles[i].totalRadius * 0.1f,
					12,
					GREEN);

				pParticles[i].acc.x -= acceleration;

				if (isShipGasEnabled) {
					for (int g = 0; g < gasMultiplier; g++) {
						float normalRand1 = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
						float normalRand2 = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

						pParticles.emplace_back(ParticlePhysics3D(
							glm::vec3{ pParticles[i].pos.x + 3.3f, pParticles[i].pos.y + (4.0f * normalRand1 - 2.0f), pParticles[i].pos.z + (4.0f * normalRand2 - 2.0f) },
							glm::vec3{ pParticles[i].vel.x + 10.0f, pParticles[i].vel.y + (4.0f * normalRand1 - 2.0f), pParticles[i].vel.z + (4.0f * normalRand2 - 2.0f) },
							8500000000.0f * water.massMult * 0.1f,
							water.restDens,
							water.stiff,
							water.visc,
							water.cohesion
						));

						rParticles.emplace_back(ParticleRendering3D(
							water.color,
							rParticles[i].size * 0.7f,
							false, false, false, true, true, false, true,
							3.0f,
							water.id
						));

						rParticles.back().sphColor = water.color;
						pParticles.back().temp = 440.0f;
					}
				}
			}

			// SHIFT - Up
			if ((IO::shortcutDown(KEY_LEFT_SHIFT) || IO::shortcutDown(KEY_RIGHT_SHIFT))
				&& (!IO::mouseDown(0) || !IO::mouseDown(1) || !IO::mouseDown(2))) {

				DrawCube({ pParticles[i].pos.x, pParticles[i].pos.y + arrowLength * 0.5f, pParticles[i].pos.z },
					rParticles[i].totalRadius * 0.5f,
					arrowLength,
					rParticles[i].totalRadius * 0.5f,
					BLUE);
				DrawCylinderEx({ pParticles[i].pos.x, pParticles[i].pos.y + arrowLength, pParticles[i].pos.z },
					{ pParticles[i].pos.x, pParticles[i].pos.y + arrowLength + arrowLength * 0.5f, pParticles[i].pos.z },
					rParticles[i].totalRadius * 2.0f,
					rParticles[i].totalRadius * 0.1f,
					12,
					BLUE);

				pParticles[i].acc.y += acceleration;

				if (isShipGasEnabled) {
					for (int g = 0; g < gasMultiplier; g++) {
						float normalRand1 = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
						float normalRand2 = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

						pParticles.emplace_back(ParticlePhysics3D(
							glm::vec3{ pParticles[i].pos.x + (4.0f * normalRand1 - 2.0f), pParticles[i].pos.y + 3.3f, pParticles[i].pos.z + (4.0f * normalRand2 - 2.0f) },
							glm::vec3{ pParticles[i].vel.x + (4.0f * normalRand1 - 2.0f), pParticles[i].vel.y + 10.0f, pParticles[i].vel.z + (4.0f * normalRand2 - 2.0f) },
							8500000000.0f * water.massMult * 0.1f,
							water.restDens,
							water.stiff,
							water.visc,
							water.cohesion
						));

						rParticles.emplace_back(ParticleRendering3D(
							water.color,
							rParticles[i].size * 0.7f,
							false, false, false, true, true, false, true,
							3.0f,
							water.id
						));

						rParticles.back().sphColor = water.color;
						pParticles.back().temp = 440.0f;
					}
				}
			}

			// CTRL - Down
			if ((IO::shortcutDown(KEY_LEFT_CONTROL) || IO::shortcutDown(KEY_RIGHT_CONTROL))
				&& (!IO::mouseDown(0) || !IO::mouseDown(1) || !IO::mouseDown(2))) {

				DrawCube({ pParticles[i].pos.x, pParticles[i].pos.y - arrowLength * 0.5f, pParticles[i].pos.z },
					rParticles[i].totalRadius * 0.5f,
					arrowLength,
					rParticles[i].totalRadius * 0.5f,
					BLUE);
				DrawCylinderEx({ pParticles[i].pos.x, pParticles[i].pos.y - arrowLength, pParticles[i].pos.z },
					{ pParticles[i].pos.x, pParticles[i].pos.y - arrowLength - arrowLength * 0.5f, pParticles[i].pos.z },
					rParticles[i].totalRadius * 2.0f,
					rParticles[i].totalRadius * 0.1f,
					12,
					BLUE);

				pParticles[i].acc.y -= acceleration;

				if (isShipGasEnabled) {
					for (int g = 0; g < gasMultiplier; g++) {
						float normalRand1 = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
						float normalRand2 = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

						pParticles.emplace_back(ParticlePhysics3D(
							glm::vec3{ pParticles[i].pos.x + (4.0f * normalRand1 - 2.0f), pParticles[i].pos.y - 3.3f, pParticles[i].pos.z + (4.0f * normalRand2 - 2.0f) },
							glm::vec3{ pParticles[i].vel.x + (4.0f * normalRand1 - 2.0f), pParticles[i].vel.y - 10.0f, pParticles[i].vel.z + (4.0f * normalRand2 - 2.0f) },
							8500000000.0f * water.massMult * 0.1f,
							water.restDens,
							water.stiff,
							water.visc,
							water.cohesion
						));

						rParticles.emplace_back(ParticleRendering3D(
							water.color,
							rParticles[i].size * 0.7f,
							false, false, false, true, true, false, true,
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

	float acceleration = 4.0f;

private:

	SPHWater water;
	SPHRock rock;

	int selectionIdx = 0;
};