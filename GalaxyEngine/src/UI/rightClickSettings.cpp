#include "../../include/UI/rightClickSettings.h"
#include "../../include/parameters.h"

void RightClickSettings::rightClickMenuSpawnLogic(bool& isMouseNotHoveringUI,
	bool& isSpawningAllowed, bool& isDragging, bool& selectedColor)
{
	static bool     isMouseMoving = false;
	static Vector2  dragStartPos = { 0.0f, 0.0f };
	static bool     spawnBlocked = false;

	if (IsMouseButtonPressed(1) && isMouseNotHoveringUI) {
		dragStartPos = GetMousePosition();
		isMouseMoving = false;
	}

	if (IsMouseButtonDown(1) && isMouseNotHoveringUI) {
		Vector2 cur = GetMousePosition();
		float dx = cur.x - dragStartPos.x,
			dy = cur.y - dragStartPos.y;
		if (dx * dx + dy * dy > 5.0f * 5.0f)
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
		!IsMouseButtonDown(0)){
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
		spawnBlocked){
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

			ImVec4& col = param ? myVar.buttonEnabledColor : myVar.buttonDisabledColor;
			ImGui::PushStyleColor(ImGuiCol_Button, col);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(col.x + 0.1f, col.y + 0.1f, col.z + 0.1f, col.w));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(col.x - 0.1f, col.y - 0.1f, col.z - 0.1f, col.w));

			if (ImGui::Button(rightClickButtons[i].text.c_str(), ImVec2(ImGui::GetContentRegionAvail().x, 20.0f))) {
				param = !param;

				isMenuActive = false;
			}

			if (ImGui::IsItemHovered()) {
				ImGui::SetTooltip("%s", rightClickButtons[i].tooltip.c_str());
			}

			ImGui::PopStyleColor(3);
		}

		ImGui::Text("Primary Color");
		if (ImGui::ColorEdit4("##pCol", (float*)&pCol)) {
			myParam.colorVisuals.selectedColor = false;
		}

		ImGui::Text("Secondary Color");
		if (ImGui::ColorEdit4("##sCol", (float*)&sCol)) {
			myParam.colorVisuals.selectedColor = false;
		}

		ImGui::End();

		if (resetParticleColors) {
			for (size_t i = 0; i < myParam.rParticles.size(); i++) {
				myParam.rParticles[i].PRGBA = { 1.0f, 1.0f, 1.0f, 1.0f };
				myParam.rParticles[i].SRGBA = { 1.0f, 1.0f, 1.0f, 1.0f };
			}
			resetParticleColors = false;
		}

		if (myParam.rParticlesSelected.size() > 0 && !IsMouseButtonDown(0)) {

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

				if (rP.isSelected && (!rP.isDarkMatter || !rP.uniqueColor)) {


					pCol.x += rP.PRGBA.r;
					pCol.y += rP.PRGBA.g;
					pCol.z += rP.PRGBA.b;
					pCol.w += rP.PRGBA.a;

					sCol.x += rP.SRGBA.r;
					sCol.y += rP.SRGBA.g;
					sCol.z += rP.SRGBA.b;
					sCol.w += rP.SRGBA.a;

					visibleSelectedAmount++;
				}
			}

			pCol.x /= visibleSelectedAmount;
			pCol.y /= visibleSelectedAmount;
			pCol.z /= visibleSelectedAmount;
			pCol.w /= visibleSelectedAmount;

			sCol.x /= visibleSelectedAmount;
			sCol.y /= visibleSelectedAmount;
			sCol.z /= visibleSelectedAmount;
			sCol.w /= visibleSelectedAmount;
		}

		if (IsMouseButtonDown(0) && myParam.rParticlesSelected.size() > 0 && isMenuActive) {
			for (size_t i = 0; i < myParam.rParticles.size(); i++) {

				ParticleRendering& rP = myParam.rParticles[i];
				if (rP.isSelected && (!rP.isDarkMatter || !rP.uniqueColor)) {

					rP.PRGBA.r = pCol.x;
					rP.PRGBA.g = pCol.y;
					rP.PRGBA.b = pCol.z;
					rP.PRGBA.a = pCol.w;

					rP.SRGBA.r = sCol.x;
					rP.SRGBA.g = sCol.y;
					rP.SRGBA.b = sCol.z;
					rP.SRGBA.a = sCol.w;
				}
			}

		}
	}
}
