#include "Particles/particleSubdivision.h"

#include "parameters.h"

void ParticleSubdivision::subdivideParticles(UpdateVariables& myVar, UpdateParameters& myParam) {

	if (subdivideAll || subdivideSelected) {

		if (subdivideSelected) {
			subdivideAll = false;
		}
		if (myParam.pParticles.size() >= particlesThreshold) {

			float screenW = static_cast<float>(GetScreenWidth());
			float screenH = static_cast<float>(GetScreenHeight());

			ImVec2 subdivisionMenuSize = { 550.0f, 150.0f };

			ImGui::SetNextWindowSize(subdivisionMenuSize, ImGuiCond_Once);
			ImGui::SetNextWindowPos(ImVec2(screenW * 0.5f - subdivisionMenuSize.x * 0.5f, screenH * 0.5f - subdivisionMenuSize.y * 0.5f), ImGuiCond_Appearing);

			ImGui::Begin("##SubdivisionWarning", nullptr, ImGuiWindowFlags_NoCollapse);

			ImGui::PushFont(myVar.robotoMediumFont);

			std::string warning = "SUBDIVIDING FURTHER MIGHT HEAVILY SLOW DOWN PERFORMANCE";

			float windowWidth = ImGui::GetWindowSize().x;
			float textWidth = ImGui::CalcTextSize(warning.c_str()).x;

			ImGui::SetWindowFontScale(1.2f);

			ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
			ImGui::TextColored(ImVec4(0.9f, 0.0f, 0.0f, 1.0f), "%s", warning.c_str());

			if (ImGui::Button("Confirm", ImVec2(ImGui::GetContentRegionAvail().x, 40.0f))) {
				confirmState = !confirmState;
			}

			ImGui::PushStyleColor(ImGuiCol_Button, UpdateVariables::colButtonRedActive);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, UpdateVariables::colButtonRedActiveHover);
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, UpdateVariables::colButtonRedActivePress);
			if (ImGui::Button("Quit", ImVec2(ImGui::GetContentRegionAvail().x, 40.0f))) {
				quitState = !quitState;
			}
			ImGui::PopStyleColor(3);

			ImGui::PopFont();

			ImGui::End();
		}

		if (quitState) {
			subdivideAll = false;
		}

		if (myParam.pParticles.size() < particlesThreshold || confirmState) {
			int originalSize = static_cast<int>(myParam.pParticles.size());
			for (int i = originalSize - 1; i >= 0; i--) {
				if ((subdivideAll || myParam.rParticles[i].isSelected) && myParam.rParticles[i].canBeSubdivided) {

					float halfOffset = myParam.rParticles[i].previousSize / 2.0f * myVar.particleTextureHalfSize * 0.25f;
					float halfOffsetVisual = myParam.rParticles[i].previousSize / 2.0f;

					int multipliers[4][2] = { {-1, -1}, { 1, -1}, {-1, 1}, { 1, 1} };

					size_t firstNewParticleIndex = myParam.pParticles.size();

					for (int j = 0; j < 4; j++) {
						float offsetX = multipliers[j][0] * halfOffset + (rand() % 3 - 1);
						float offsetY = multipliers[j][1] * halfOffset + (rand() % 3 - 1);

						glm::vec2 newPos{
							myParam.pParticles[i].pos.x + offsetX,
							myParam.pParticles[i].pos.y + offsetY
						};

						myParam.pParticles.emplace_back(
							newPos,
							myParam.pParticles[i].vel,
							myParam.pParticles[i].mass / 4.0f,
							myParam.pParticles[i].restDens,
							myParam.pParticles[i].stiff,
							myParam.pParticles[i].visc,
							myParam.pParticles[i].cohesion
						);


						myParam.rParticles.emplace_back(
							myParam.rParticles[i].color,
							halfOffsetVisual,
							myParam.rParticles[i].uniqueColor,
							myParam.rParticles[i].isSelected,
							myParam.rParticles[i].isSolid,
							myParam.rParticles[i].canBeSubdivided,
							myParam.rParticles[i].canBeResized,
							myParam.rParticles[i].isDarkMatter,
							myParam.rParticles[i].isSPH,
							myParam.rParticles[i].lifeSpan,
							myParam.rParticles[i].sphLabel
						);
					}

					for (int j = 0; j < 4; ++j) {
						myParam.pParticles[firstNewParticleIndex + j].id = globalId++;
						myParam.pParticles[firstNewParticleIndex + j].temp = myParam.pParticles[i].temp;
					}

					for (int j = 0; j < 4; ++j) {
						myParam.rParticles[firstNewParticleIndex + j].pColor = myParam.rParticles[i].pColor;
						myParam.rParticles[firstNewParticleIndex + j].sColor = myParam.rParticles[i].sColor;
						myParam.rParticles[firstNewParticleIndex + j].sphColor = myParam.rParticles[i].sphColor;
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

void ParticleSubdivision::subdivideParticles3D(UpdateVariables& myVar, UpdateParameters& myParam) {

    if (subdivideAll || subdivideSelected) {

        if (subdivideSelected) {
            subdivideAll = false;
        }

        if (myParam.pParticles3D.size() >= particlesThreshold) {

            float screenW = static_cast<float>(GetScreenWidth());
            float screenH = static_cast<float>(GetScreenHeight());

            ImVec2 subdivisionMenuSize = { 550.0f, 150.0f };

            ImGui::SetNextWindowSize(subdivisionMenuSize, ImGuiCond_Once);
            ImGui::SetNextWindowPos(ImVec2(screenW * 0.5f - subdivisionMenuSize.x * 0.5f, screenH * 0.5f - subdivisionMenuSize.y * 0.5f), ImGuiCond_Appearing);

            ImGui::Begin("##SubdivisionWarning", nullptr, ImGuiWindowFlags_NoCollapse);
            ImGui::PushFont(myVar.robotoMediumFont);

            std::string warning = "SUBDIVIDING FURTHER MIGHT HEAVILY SLOW DOWN PERFORMANCE";

            float windowWidth = ImGui::GetWindowSize().x;
            float textWidth = ImGui::CalcTextSize(warning.c_str()).x;

            ImGui::SetWindowFontScale(1.2f);
            ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
            ImGui::TextColored(ImVec4(0.9f, 0.0f, 0.0f, 1.0f), "%s", warning.c_str());

            if (ImGui::Button("Confirm", ImVec2(ImGui::GetContentRegionAvail().x, 40.0f))) {
                confirmState = !confirmState;
            }

            ImGui::PushStyleColor(ImGuiCol_Button, UpdateVariables::colButtonRedActive);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, UpdateVariables::colButtonRedActiveHover);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, UpdateVariables::colButtonRedActivePress);

            if (ImGui::Button("Quit", ImVec2(ImGui::GetContentRegionAvail().x, 40.0f))) {
                quitState = !quitState;
            }
            ImGui::PopStyleColor(3);
            ImGui::PopFont();
            ImGui::End();
        }

        if (quitState) {
            subdivideAll = false;
        }

        if (myParam.pParticles3D.size() < particlesThreshold || confirmState) {

            int originalSize = static_cast<int>(myParam.pParticles3D.size());

            for (int i = originalSize - 1; i >= 0; i--) {

                bool isTarget = (subdivideAll || myParam.rParticles3D[i].isSelected);

                if (isTarget && myParam.rParticles3D[i].canBeSubdivided) {

                    float halfOffset = myParam.rParticles3D[i].previousSize / 2.0f * myVar.particleTextureHalfSize * 0.25f;
                    float halfOffsetVisual = myParam.rParticles3D[i].previousSize / 2.0f;

                    int multipliers[8][3] = {
                        {-1, -1, -1}, { 1, -1, -1}, {-1, 1, -1}, { 1, 1, -1},
                        {-1, -1,  1}, { 1, -1,  1}, {-1, 1,  1}, { 1, 1,  1}
                    };

                    size_t firstNewParticleIndex = myParam.pParticles3D.size();

                    for (int j = 0; j < 8; j++) {

                        float offsetX = multipliers[j][0] * halfOffset + (rand() % 3 - 1);
                        float offsetY = multipliers[j][1] * halfOffset + (rand() % 3 - 1);
                        float offsetZ = multipliers[j][2] * halfOffset + (rand() % 3 - 1);

                        glm::vec3 newPos{
                            myParam.pParticles3D[i].pos.x + offsetX,
                            myParam.pParticles3D[i].pos.y + offsetY,
                            myParam.pParticles3D[i].pos.z + offsetZ
                        };

                        myParam.pParticles3D.emplace_back(
                            newPos,
                            myParam.pParticles3D[i].vel,
                            myParam.pParticles3D[i].mass / 8.0f,
                            myParam.pParticles3D[i].restDens,
                            myParam.pParticles3D[i].stiff,
                            myParam.pParticles3D[i].visc,
                            myParam.pParticles3D[i].cohesion
                        );

                        myParam.rParticles3D.emplace_back(
                            myParam.rParticles3D[i].color,
                            halfOffsetVisual,
                            myParam.rParticles3D[i].uniqueColor,
                            myParam.rParticles3D[i].isSelected,
                            myParam.rParticles3D[i].isSolid,
                            myParam.rParticles3D[i].canBeSubdivided,
                            myParam.rParticles3D[i].canBeResized,
                            myParam.rParticles3D[i].isDarkMatter,
                            myParam.rParticles3D[i].isSPH,
                            myParam.rParticles3D[i].lifeSpan,
                            myParam.rParticles3D[i].sphLabel
                        );
                    }

                    for (int j = 0; j < 8; ++j) {
                        myParam.pParticles3D[firstNewParticleIndex + j].id = globalId++;
                        myParam.pParticles3D[firstNewParticleIndex + j].temp = myParam.pParticles3D[i].temp;
                    }

                    for (int j = 0; j < 8; ++j) {
                        myParam.rParticles3D[firstNewParticleIndex + j].pColor = myParam.rParticles3D[i].pColor;
                        myParam.rParticles3D[firstNewParticleIndex + j].sColor = myParam.rParticles3D[i].sColor;
                        myParam.rParticles3D[firstNewParticleIndex + j].sphColor = myParam.rParticles3D[i].sphColor;
                    }

                    if (i < myParam.pParticles3D.size() - 1) {
                        myParam.pParticles3D[i] = std::move(myParam.pParticles3D.back());
                        myParam.rParticles3D[i] = std::move(myParam.rParticles3D.back());
                    }

                    myParam.pParticles3D.pop_back();
                    myParam.rParticles3D.pop_back();
                }
            }

            subdivideAll = false;
            subdivideSelected = false;
        }
        confirmState = false;
        quitState = false;
    }
}
