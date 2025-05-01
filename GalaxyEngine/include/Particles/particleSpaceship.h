#pragma once
#include "../raylib/raylib.h"
#include "particle.h"
#include <vector>

class ParticleSpaceship {
public:
	
	void spaceshipLogic(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, bool& isShipGasEnabled) {

        if (pParticles.empty()) {
            isShipEnabled = false;
            return;
        }

        isShipEnabled =  (IsKeyDown(KEY_UP)
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
                float normalRand = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

                    pParticles.emplace_back(ParticlePhysics(
                        Vector2{ pParticles[i].pos.x, pParticles[i].pos.y + (2.0f * normalRand) },
                        Vector2{ pParticles[i].vel.x, pParticles[i].vel.y + 10.0f },
                        10.0f
                    ));

                    rParticles.emplace_back(ParticleRendering(
                        Color{ 128,128,128,100 },
                        rParticles[i].size / 2.0f,
                        true,
                        false,
                        false,
                        true,
                        true,
                        false
                    ));
                }
            }
            if (IsKeyDown(KEY_RIGHT)) {
                pParticles[i].acc.x += acceleration;

                if (isShipGasEnabled) {
                    float normalRand = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

                    pParticles.emplace_back(ParticlePhysics(
                        Vector2{ pParticles[i].pos.x - (2.0f * normalRand), pParticles[i].pos.y },
                        Vector2{ pParticles[i].vel.x - 10.0f, pParticles[i].vel.y },
                        10.0f
                    ));

                    rParticles.emplace_back(ParticleRendering(
                        Color{ 128,128,128,100 },
                        rParticles[i].size / 2.0f,
                        true,
                        false,
                        false,
                        true,
                        true,
                        false
                    ));
                }
            }
            if (IsKeyDown(KEY_DOWN)) {
                pParticles[i].acc.y += acceleration;

                if (isShipGasEnabled) {
                    float normalRand = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

                    pParticles.emplace_back(ParticlePhysics(
                        Vector2{ pParticles[i].pos.x, pParticles[i].pos.y - (2.0f * normalRand) },
                        Vector2{ pParticles[i].vel.x, pParticles[i].vel.y - 10.0f },
                        10.0f
                    ));

                    rParticles.emplace_back(ParticleRendering(
                        Color{ 128,128,128,100 },
                        rParticles[i].size / 2.0f,
                        true,
                        false,
                        false,
                        true,
                        true,
                        false
                    ));
                }
            }
            if (IsKeyDown(KEY_LEFT)) {
                pParticles[i].acc.x -= acceleration;

                if (isShipGasEnabled) {
                    float normalRand = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

                    pParticles.emplace_back(ParticlePhysics(
                        Vector2{ pParticles[i].pos.x + (2.0f * normalRand), pParticles[i].pos.y },
                        Vector2{ pParticles[i].vel.x + 10.0f, pParticles[i].vel.y },
                        10.0f
                    ));

                    rParticles.emplace_back(ParticleRendering(
                        Color{ 128,128,128,100 },
                        rParticles[i].size / 2.0f,
                        true,
                        false,
                        false,
                        true,
                        true,
                        false
                    ));
                }
            }
        }
	}

private:
	bool isShipEnabled = false;
	float acceleration = 8.0f;

	int selectionIdx = 0;
};