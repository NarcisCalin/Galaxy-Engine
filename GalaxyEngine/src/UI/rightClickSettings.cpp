#include "../../include/UI/rightClickSettings.h"
#include "../../include/parameters.h"

void RightClickSettings::rightClickMenuSpawnLogic(bool& isMouseNotHoveringUI,
	bool& isSpawningAllowed, bool& isDragging, bool& selectedColor)
{
	static bool     isMouseMoving = false;
	static glm::vec2  dragStartPos = { 0.0f, 0.0f };
	static bool     spawnBlocked = false;

	if (IsMouseButtonPressed(1) && isMouseNotHoveringUI) {
		dragStartPos = glm::vec2(GetMousePosition().x, GetMousePosition().y);
		isMouseMoving = false;
	}

	if (IsMouseButtonDown(1) && isMouseNotHoveringUI) {
		glm::vec2 cur = glm::vec2(GetMousePosition().x, GetMousePosition().y);
		glm::vec2 d = cur - dragStartPos;
		if (d.x * d.x + d.y * d.y > 5.0f * 5.0f)
			isMouseMoving = true;
	}

	if (IsMouseButtonPressed(1)) {
		isMenuActive = false;
	}

	if (IsMouseButtonReleased(1) &&
		!IsKeyDown(KEY_LEFT_CONTROL) &&
		!IsKeyDown(KEY_LEFT_ALT) &&
		!isMouseMoving &&
		isMouseNotHoveringUI &&
		!IsMouseButtonDown(0)) {
		isMenuActive = true;
		spawnBlocked = false;
		selectedColorOriginal = selectedColor;
	}

	if (IsMouseButtonPressed(0) &&
		isMouseNotHoveringUI &&
		isMenuActive &&
		!isMouseOnMenu)
	{
		isMenuActive = false;
		isSpawningAllowed = false;
		isDragging = false;
		spawnBlocked = true;
		selectedColor = selectedColorOriginal;
		selectedColorChanged = false;
	}

	else if (IsMouseButtonPressed(0) &&
		isMouseNotHoveringUI &&
		!isMenuActive &&
		spawnBlocked) {
		isSpawningAllowed = true;
		spawnBlocked = false;
	}
}


void RightClickSettings::rightClickMenu(UpdateVariables& myVar, UpdateParameters& myParam) {

	rightClickMenuSpawnLogic(myVar.isMouseNotHoveringUI, myParam.particlesSpawning.isSpawningAllowed, myVar.isDragging, myParam.colorVisuals.selectedColor);

	if (isMenuActive) {

		static std::array<rightClickParams, 13> rightClickButtons = {
		rightClickParams("Subdivide All", "Subdivide all normal particles", myParam.subdivision.subdivideAll),
		rightClickParams("Subdivide Selected", "Subdivide all selected normal particles", myParam.subdivision.subdivideSelected),
		rightClickParams("Invert Particle Selec.", "Invert the particle selection", myParam.particleSelection.invertParticleSelection),
		rightClickParams("Deselect All", "Deselects all particles", myParam.particleSelection.deselectParticles),
		rightClickParams("Follow selection", "Make the camera follow the selected particles", myParam.myCamera.centerCamera),
		rightClickParams("Select Clusters", "Selects multiple clusters of particles", myParam.particleSelection.selectManyClusters),
		rightClickParams("Delete Selection", "Deletes selected particles", myParam.particleDeletion.deleteSelection),
		rightClickParams("Delete Stray Particles", "Deletes all particles that are not in groups", myParam.particleDeletion.deleteNonImportant),
		rightClickParams("Reset Custom Colors", "Resets custom colors changed in the right click menu", resetParticleColors),
		rightClickParams("Draw Z-Curves", "Display the particles indices, sorted by Z-Curves", myVar.drawZCurves),
		rightClickParams("Draw Quadtree", "Display Barnes-Hut algorithm quadtree", myVar.drawQuadtree),
		rightClickParams("Enable Frames Export", "Exports recorded frames to disk", myParam.screenCapture.isExportFramesEnabled),
		rightClickParams("Safe Frames Export", "Store frames directly to the disk. Disabling it will make recording faster, but might crash if you run out of memory", myParam.screenCapture.isSafeFramesEnabled),

		};

		ImGui::SetNextWindowSize(ImVec2(200.0f, 425.0f), ImGuiCond_Once);
		ImGui::SetNextWindowPos(ImVec2(GetMousePosition().x, GetMousePosition().y), ImGuiCond_Appearing);

		if (ImGui::Begin("Right Click Menu", nullptr, ImGuiWindowFlags_NoCollapse)) {

			bool hovered = ImGui::IsWindowHovered(
				ImGuiHoveredFlags_AllowWhenBlockedByPopup |
				ImGuiHoveredFlags_AllowWhenBlockedByActiveItem
			);
			isMouseOnMenu = hovered;
		}

		bool hovered = ImGui::IsWindowHovered(
			ImGuiHoveredFlags_AllowWhenBlockedByPopup |
			ImGuiHoveredFlags_AllowWhenBlockedByActiveItem
		);

		for (size_t i = 0; i < rightClickButtons.size(); i++) {

			bool& param = rightClickButtons[i].parameter;

			if (ImGui::Button(rightClickButtons[i].text.c_str(), ImVec2(ImGui::GetContentRegionAvail().x, 20.0f))) {
				param = !param;

				isMenuActive = false;
			}

			if (ImGui::IsItemHovered()) {
				ImGui::SetTooltip("%s", rightClickButtons[i].tooltip.c_str());
			}
		}

		bool pColChanged = false;
		bool sColChanged = false;

		ImGui::Text("Primary Color");
		if (ImGui::ColorEdit4("##pCol", (float*)&pCol, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_DisplayRGB)) {
			myParam.colorVisuals.selectedColor = false;
			pColChanged = true;
			for (size_t i = 0; i < myParam.rParticles.size(); i++) {
				if (myParam.rParticles[i].isSelected) {
					myParam.rParticles[i].uniqueColor = true;
				}
			}
		}
		ImGui::Text("Secondary Color");
		if (ImGui::ColorEdit4("##sCol", (float*)&sCol, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_DisplayRGB)) {
			myParam.colorVisuals.selectedColor = false;
			sColChanged = true;
			for (size_t i = 0; i < myParam.rParticles.size(); i++) {
				if (myParam.rParticles[i].isSelected) {
					myParam.rParticles[i].uniqueColor = true;
				}
			}
		}
		ImGui::End();

		if (resetParticleColors) {
			for (size_t i = 0; i < myParam.rParticles.size(); i++) {
				myParam.rParticles[i].pColor = { 255, 255, 255, 255 };
				myParam.rParticles[i].sColor = { 255, 255, 255, 255 };
				myParam.rParticles[i].uniqueColor = false;
			}
			resetParticleColors = false;
		}

		if (myParam.rParticlesSelected.size() > 0 && !pColChanged && !sColChanged &&
			!ImGui::IsItemActive()) {

			pCol.x = 0.0f;
			pCol.y = 0.0f;
			pCol.z = 0.0f;
			pCol.w = 0.0f;

			sCol.x = 0.0f;
			sCol.y = 0.0f;
			sCol.z = 0.0f;
			sCol.w = 0.0f;

			int visibleSelectedAmount = 0;

			for (size_t i = 0; i < myParam.rParticles.size(); i++) {
				ParticleRendering& rP = myParam.rParticles[i];

				if (rP.isSelected && !rP.isDarkMatter) {

					ImVec4 rToVecPColor = rlImGuiColors::Convert(rP.pColor);
					ImVec4 rToVecSColor = rlImGuiColors::Convert(rP.sColor);

					pCol.x += rToVecPColor.x;
					pCol.y += rToVecPColor.y;
					pCol.z += rToVecPColor.z;
					pCol.w += rToVecPColor.w;

					sCol.x += rToVecSColor.x;
					sCol.y += rToVecSColor.y;
					sCol.z += rToVecSColor.z;
					sCol.w += rToVecSColor.w;

					visibleSelectedAmount++;
				}
			}

			if (visibleSelectedAmount > 0) {
				pCol.x /= visibleSelectedAmount;
				pCol.y /= visibleSelectedAmount;
				pCol.z /= visibleSelectedAmount;
				pCol.w /= visibleSelectedAmount;

				sCol.x /= visibleSelectedAmount;
				sCol.y /= visibleSelectedAmount;
				sCol.z /= visibleSelectedAmount;
				sCol.w /= visibleSelectedAmount;
			}
		}

		if ((pColChanged || sColChanged) && myParam.rParticlesSelected.size() > 0 && isMenuActive) {
			vecToRPColor = rlImGuiColors::Convert(pCol);
			vecToRSColor = rlImGuiColors::Convert(sCol);

			for (size_t i = 0; i < myParam.rParticles.size(); i++) {
				ParticleRendering& rP = myParam.rParticles[i];
				if (rP.isSelected && !rP.isDarkMatter) {

					rP.uniqueColor = true;
					rP.pColor.r = vecToRPColor.r;
					rP.pColor.g = vecToRPColor.g;
					rP.pColor.b = vecToRPColor.b;
					rP.pColor.a = vecToRPColor.a;

					rP.sColor.r = vecToRSColor.r;
					rP.sColor.g = vecToRSColor.g;
					rP.sColor.b = vecToRSColor.b;
					rP.sColor.a = vecToRSColor.a;
				}
			}
		}
	}
}
