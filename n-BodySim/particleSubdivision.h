#pragma once
#include <vector>
#include "raylib.h"
#include "particle.h"
#include "button.h"

struct ParticleSubdivision {

	int particlesThreshold = 80000;

	bool subdivideAll = false;
	bool subdivideSelected = false;

	void subdivideParticles(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, float& particleTextureSize,
		bool& isMouseNotHoveringUI, bool& isDragging) {

		if (subdivideAll || subdivideSelected) {

			if (subdivideSelected) {
				subdivideAll = false;
			}
			if (pParticles.size() > particlesThreshold) {
				Button confirm({ GetScreenWidth() / 2.0f - 5.0f - buttonSize.x, GetScreenHeight() / 2.0f }, { buttonSize }, "Subdivide", true);
				Button quit({ GetScreenWidth() / 2.0f + 5.0f, GetScreenHeight() / 2.0f }, { buttonSize }, "Quit", true);

				Vector2 textCompensation = MeasureTextEx(GetFontDefault(), warningText.c_str(), textSize, textSpacing);

				DrawTextEx(GetFontDefault(), warningText.c_str(), { static_cast<float>(GetScreenWidth() / 2 - (textCompensation.x / 2)),
					static_cast<float>(GetScreenHeight() / 2 - (textCompensation.y / 2) - 25.0f) }, textSize, textSpacing, WHITE);

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
				int originalSize = static_cast<int>(pParticles.size());
				for (int i = originalSize - 1; i >= 0; i--) {
					if ((subdivideAll || rParticles[i].isSelected) && rParticles[i].canBeSubdivided) {

						float halfOffset = rParticles[i].previousSize / 2.0f * particleTextureSize / 8.0f;
						float halfOffsetVisual = rParticles[i].previousSize / 2.0f;

						int multipliers[4][2] = { {-1, -1}, { 1, -1}, {-1, 1}, { 1, 1} };

						for (int j = 0; j < 4; j++) {
							float offsetX = multipliers[j][0] * halfOffset + (rand() % 3 - 1);
							float offsetY = multipliers[j][1] * halfOffset + (rand() % 3 - 1);

							Vector2 newPos{
								pParticles[i].pos.x + offsetX,
								pParticles[i].pos.y + offsetY
							};

							pParticles.emplace_back(newPos, pParticles[i].velocity, pParticles[i].mass / 4.0f);

							rParticles.emplace_back(rParticles[i].color, halfOffsetVisual, rParticles[i].uniqueColor,
								rParticles[i].drawPixel, rParticles[i].isSelected,
								rParticles[i].isSolid, rParticles[i].canBeSubdivided, rParticles[i].canBeResized);
						}

						pParticles[i] = std::move(pParticles.back());
						pParticles.pop_back();
						rParticles[i] = std::move(rParticles.back());
						rParticles.pop_back();
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