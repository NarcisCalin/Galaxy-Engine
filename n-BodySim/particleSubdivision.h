#pragma once
#include <vector>
#include "raylib.h"
#include "planet.h"
#include "button.h"

struct ParticleSubdivision {

	int particlesThreshold = 15000;

	void subdivideParticles(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, float& particleTextureSize,
		bool& subdivideAll, bool& subdivideSelected, bool& isMouseNotHoveringUI, bool& isDragging) {

		if (subdivideAll || subdivideSelected) {

			if (subdivideSelected) {
				subdivideAll = false;
			}
			if (pParticles.size() > particlesThreshold) {
				Button confirm({ GetScreenWidth() / 2.0f - 5.0f - buttonSize.x, GetScreenHeight() / 2.0f }, { buttonSize }, "Subdivide", true);
				Button quit({ GetScreenWidth() / 2.0f + 5.0f, GetScreenHeight() / 2.0f }, { buttonSize }, "Quit", true);

				Vector2 textCompensation = MeasureTextEx(GetFontDefault(), warningText.c_str(), textSize, textSpacing);

				DrawTextEx(GetFontDefault(), warningText.c_str(), { static_cast<float>(GetScreenWidth() / 2 - (textCompensation.x / 2)), 
					static_cast<float>(GetScreenHeight() / 2 - (textCompensation.y / 2) - 25.0f)}, textSize, textSpacing, WHITE);

				bool confirmHovering = confirm.buttonLogic(confirmState);
				bool quitHovering = quit.buttonLogic(quitState);

				if (confirmHovering || quitHovering) {
					isMouseNotHoveringUI = false;
					isDragging = false;
				}
			}

			if (quitState) {
				subdivideAll = false;
			}

			if (pParticles.size() < particlesThreshold || confirmState) {
				size_t originalSize = pParticles.size();

				for (int i = originalSize - 1; i >= 0; i--) {
					if ((subdivideAll || rParticles[i].isSelected) && rParticles[i].canBeSubdivided) {

					pParticles.emplace_back
					(
						Vector2{ pParticles[i].pos.x - rParticles[i].size / 2.0f * particleTextureSize / 8.0f + rand() % 3 - 1, 
						pParticles[i].pos.y - rParticles[i].size / 2.0f * particleTextureSize / 8.0f + rand() % 3 - 1 },
						Vector2{ pParticles[i].velocity.x, pParticles[i].velocity.y },
						pParticles[i].mass / 4.0f
					);

					pParticles.emplace_back
					(
						Vector2{ pParticles[i].pos.x + rParticles[i].size / 2.0f * particleTextureSize / 8.0f + rand() % 3 - 1,
						pParticles[i].pos.y - rParticles[i].size / 2.0f * particleTextureSize / 8.0f + rand() % 3 - 1 },
						Vector2{ pParticles[i].velocity.x, pParticles[i].velocity.y },
						pParticles[i].mass / 4.0f
					);

					pParticles.emplace_back
					(
						Vector2{ pParticles[i].pos.x - rParticles[i].size / 2.0f * particleTextureSize / 8.0f + rand() % 3 - 1,
						pParticles[i].pos.y + rParticles[i].size / 2.0f * particleTextureSize / 8.0f + rand() % 3 - 1 },
						Vector2{ pParticles[i].velocity.x, pParticles[i].velocity.y },
						pParticles[i].mass / 4.0f
					);

					pParticles.emplace_back
					(
						Vector2{ pParticles[i].pos.x + rParticles[i].size / 2.0f * particleTextureSize / 8.0f + rand() % 3 - 1,
						pParticles[i].pos.y + rParticles[i].size / 2.0f * particleTextureSize / 8.0f + rand() % 3 - 1 },
						Vector2{ pParticles[i].velocity.x, pParticles[i].velocity.y },
						pParticles[i].mass / 4.0f
					);

					rParticles.emplace_back
					(
						Color{ rParticles[i].color },
						rParticles[i].size / 2.0f,
						rParticles[i].uniqueColor,
						rParticles[i].drawPixel,
						rParticles[i].isSelected,
						rParticles[i].isSolid,
						rParticles[i].canBeSubdivided
					);

					rParticles.emplace_back
					(
						Color{ rParticles[i].color },
						rParticles[i].size / 2.0f,
						rParticles[i].uniqueColor,
						rParticles[i].drawPixel,
						rParticles[i].isSelected,
						rParticles[i].isSolid,
						rParticles[i].canBeSubdivided
					);

					rParticles.emplace_back
					(
						Color{ rParticles[i].color },
						rParticles[i].size / 2.0f,
						rParticles[i].uniqueColor,
						rParticles[i].drawPixel,
						rParticles[i].isSelected,
						rParticles[i].isSolid,
						rParticles[i].canBeSubdivided
					);

					rParticles.emplace_back
					(
						Color{ rParticles[i].color },
						rParticles[i].size / 2.0f,
						rParticles[i].uniqueColor,
						rParticles[i].drawPixel,
						rParticles[i].isSelected,
						rParticles[i].isSolid,
						rParticles[i].canBeSubdivided
					);

					pParticles.erase(pParticles.begin() + i);
					rParticles.erase(rParticles.begin() + i);
					}
				}

				subdivideAll = false;
				subdivideSelected = false;
			}
				confirmState = false;
				quitState = false;
		}
	}

private:
	bool confirmState = false;
	bool quitState = false;

	std::string warningText = "Subdividing further might slow down the program a lot";

	float textSize = 25.0f;
	float textSpacing = 6.0f;

	Vector2 buttonSize = { 200.0f, 50.0f };
};