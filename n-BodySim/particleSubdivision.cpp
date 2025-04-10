#include "particleSubdivision.h"
#include "parameters.h"

void ParticleSubdivision::subdivideParticles(UpdateVariables& myVar, UpdateParameters& myParam) {

	if (subdivideAll || subdivideSelected) {

		if (subdivideSelected) {
			subdivideAll = false;
		}
		if (myParam.pParticles.size() > particlesThreshold) {
			Button confirm({ GetScreenWidth() / 2.0f - 5.0f - buttonSize.x, GetScreenHeight() / 2.0f }, { buttonSize }, "Subdivide", true);
			Button quit({ GetScreenWidth() / 2.0f + 5.0f, GetScreenHeight() / 2.0f }, { buttonSize }, "Quit", true);

			Vector2 textCompensation = MeasureTextEx(GetFontDefault(), warningText.c_str(), textSize, textSpacing);

			DrawTextEx(GetFontDefault(), warningText.c_str(), { static_cast<float>(GetScreenWidth() / 2 - (textCompensation.x / 2)),
				static_cast<float>(GetScreenHeight() / 2 - (textCompensation.y / 2) - 25.0f) }, textSize, textSpacing, WHITE);

			bool confirmHovering = confirm.buttonLogic(confirmState, myVar);
			bool quitHovering = quit.buttonLogic(quitState, myVar);
		}

		if (quitState) {
			subdivideAll = false;
		}

		if (myParam.pParticles.size() < particlesThreshold || confirmState) {
			int originalSize = static_cast<int>(myParam.pParticles.size());
			for (int i = originalSize - 1; i >= 0; i--) {
				if ((subdivideAll || myParam.rParticles[i].isSelected) && myParam.rParticles[i].canBeSubdivided) {

					float halfOffset = myParam.rParticles[i].previousSize / 2.0f * myVar.particleTextureSize / 8.0f;
					float halfOffsetVisual = myParam.rParticles[i].previousSize / 2.0f;

					int multipliers[4][2] = { {-1, -1}, { 1, -1}, {-1, 1}, { 1, 1} };

					for (int j = 0; j < 4; j++) {
						float offsetX = multipliers[j][0] * halfOffset + (rand() % 3 - 1);
						float offsetY = multipliers[j][1] * halfOffset + (rand() % 3 - 1);

						Vector2 newPos{
							myParam.pParticles[i].pos.x + offsetX,
							myParam.pParticles[i].pos.y + offsetY
						};

						myParam.pParticles.emplace_back(newPos, myParam.pParticles[i].velocity, myParam.pParticles[i].mass / 4.0f);

						myParam.rParticles.emplace_back(myParam.rParticles[i].color, halfOffsetVisual, myParam.rParticles[i].uniqueColor,
							myParam.rParticles[i].drawPixel, myParam.rParticles[i].isSelected,
							myParam.rParticles[i].isSolid, myParam.rParticles[i].canBeSubdivided, myParam.rParticles[i].canBeResized);
					}

					myParam.pParticles[i] = std::move(myParam.pParticles.back());
					myParam.pParticles.pop_back();
					myParam.rParticles[i] = std::move(myParam.rParticles.back());
					myParam.rParticles.pop_back();
				}
			}
			subdivideAll = false;
			subdivideSelected = false;
		}
		confirmState = false;
		quitState = false;
	}
}
