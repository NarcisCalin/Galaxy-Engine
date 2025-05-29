#include "../../include/Particles/particleSelection.h"
#include "../../include/parameters.h"

ParticleSelection::ParticleSelection() {
}

void ParticleSelection::clusterSelection(UpdateVariables& myVar, UpdateParameters& myParam) {
	static bool isMouseMoving = false;
	static glm::vec2 dragStartPos = { 0.0f, 0.0f };

	if (IsMouseButtonPressed(0) && IsKeyDown(KEY_LEFT_CONTROL) && myVar.isMouseNotHoveringUI) {
		dragStartPos = glm::vec2(GetMousePosition().x, GetMousePosition().y);
		isMouseMoving = false;
	}

	if (IsMouseButtonDown(0) && IsKeyDown(KEY_LEFT_CONTROL) && myVar.isMouseNotHoveringUI) {
		glm::vec2 currentPos = glm::vec2(GetMousePosition().x, GetMousePosition().y);
		float dragThreshold = 5.0f;
		float dx = currentPos.x - dragStartPos.x;
		float dy = currentPos.y - dragStartPos.y;

		if (dx * dx + dy * dy > dragThreshold * dragThreshold) {
			isMouseMoving = true;
		}
	}

	if (IsMouseButtonReleased(0) && IsKeyDown(KEY_LEFT_CONTROL) && !isMouseMoving && myVar.isMouseNotHoveringUI) {
		float distanceThreshold = 10.0f;
		std::vector<int> neighborCountsSelect(myParam.pParticles.size(), 0);
		for (size_t i = 0; i < myParam.pParticles.size(); i++) {
			const auto& pParticle = myParam.pParticles[i];

			if (myParam.rParticles[i].isDarkMatter) {
				continue;
			}

			for (size_t j = i + 1; j < myParam.pParticles.size(); j++) {

				if (myParam.rParticles[j].isDarkMatter) {
					continue;
				}

				if (std::abs(myParam.pParticles[j].pos.x - pParticle.pos.x) > 2.4f) break;
				float dx = pParticle.pos.x - myParam.pParticles[j].pos.x;
				float dy = pParticle.pos.y - myParam.pParticles[j].pos.y;
				if (dx * dx + dy * dy < distanceThreshold * distanceThreshold) {
					neighborCountsSelect[i]++;
					neighborCountsSelect[j]++;
				}
			}
		}

		for (size_t i = 0; i < myParam.pParticles.size(); i++) {
			if (!IsKeyDown(KEY_LEFT_SHIFT)) {
				myParam.rParticles[i].isSelected = false;
				if (!myVar.isGlobalTrailsEnabled) {
					myParam.trails.trailDots.clear();
				}
			}
			float dx = myParam.pParticles[i].pos.x - myParam.myCamera.mouseWorldPos.x;
			float dy = myParam.pParticles[i].pos.y - myParam.myCamera.mouseWorldPos.y;
			float distanceSq = dx * dx + dy * dy;
			if (distanceSq < selectionThresholdSq && neighborCountsSelect[i] > 3 && !myParam.rParticles[i].isDarkMatter) {
				myParam.rParticles[i].isSelected = true;
			}
			else {
				if (!IsKeyDown(KEY_LEFT_SHIFT) && !myParam.pParticles.empty()) {
					myParam.rParticles[i].isSelected = false;
					if (!myVar.isGlobalTrailsEnabled) {
						myParam.trails.trailDots.clear();
					}
				}
			}
		}
	}
}

void ParticleSelection::particleSelection(UpdateVariables& myVar, UpdateParameters& myParam) {
	static bool isMouseMoving = false;
	static glm::vec2 dragStartPos = { 0.0f, 0.0f };

	if (IsMouseButtonPressed(0) && IsKeyDown(KEY_LEFT_ALT) && myVar.isMouseNotHoveringUI) {
		dragStartPos = glm::vec2(GetMousePosition().x, GetMousePosition().y);
		isMouseMoving = false;
	}

	if (IsMouseButtonDown(0) && IsKeyDown(KEY_LEFT_ALT) && myVar.isMouseNotHoveringUI) {
		glm::vec2 currentPos = glm::vec2(GetMousePosition().x, GetMousePosition().y);
		float dragThreshold = 5.0f;
		float dx = currentPos.x - dragStartPos.x;
		float dy = currentPos.y - dragStartPos.y;

		if (dx * dx + dy * dy > dragThreshold * dragThreshold) {
			isMouseMoving = true;
		}
	}

	if (IsMouseButtonReleased(0) && IsKeyDown(KEY_LEFT_ALT) && !isMouseMoving && myVar.isMouseNotHoveringUI) {
		size_t closestIndex = 0;
		float minDistanceSq = std::numeric_limits<float>::max();

		for (size_t i = 0; i < myParam.pParticles.size(); i++) {

			if (myParam.rParticles[i].isDarkMatter) {
				continue;
			}

			float dx = myParam.pParticles[i].pos.x - myParam.myCamera.mouseWorldPos.x;
			float dy = myParam.pParticles[i].pos.y - myParam.myCamera.mouseWorldPos.y;
			float currentDistanceSq = dx * dx + dy * dy;
			if (currentDistanceSq < minDistanceSq) {
				minDistanceSq = currentDistanceSq;
				closestIndex = i;
			}
		}

		if (!IsKeyDown(KEY_LEFT_SHIFT)) {
			bool wasClosestSelected = (minDistanceSq < selectionThresholdSq && !myParam.pParticles.empty()) ?
				myParam.rParticles[closestIndex].isSelected : false;

			for (size_t i = 0; i < myParam.pParticles.size(); i++) {
				myParam.rParticles[i].isSelected = false;
			}

			if (!myVar.isGlobalTrailsEnabled) {
				myParam.trails.trailDots.clear();
			}

			if (minDistanceSq < selectionThresholdSq && !myParam.pParticles.empty() && !wasClosestSelected && !myParam.rParticles[closestIndex].isDarkMatter) {
				myParam.rParticles[closestIndex].isSelected = true;
			}
		}
		else {
			if (minDistanceSq < selectionThresholdSq && !myParam.pParticles.empty()) {
				myParam.rParticles[closestIndex].isSelected = !myParam.rParticles[closestIndex].isSelected;
			}
		}
	}
}


void ParticleSelection::manyClustersSelection(UpdateVariables& myVar, UpdateParameters& myParam) {
	if (selectManyClusters) {
		if (!myVar.isGlobalTrailsEnabled) {
			myParam.trails.trailDots.clear();
		}
		float distanceThreshold = 10.0f;
		std::vector<int> neighborCountsSelect(myParam.pParticles.size(), 0);
		for (size_t i = 0; i < myParam.pParticles.size(); i++) {
			myParam.rParticles[i].isSelected = false;

			if (myParam.rParticles[i].isDarkMatter) {
				continue;
			}

			const auto& pParticle = myParam.pParticles[i];
			for (size_t j = i + 1; j < myParam.pParticles.size(); j++) {

				if (myParam.rParticles[j].isDarkMatter) {
					continue;
				}

				if (std::abs(myParam.pParticles[j].pos.x - pParticle.pos.x) > 2.4f) break;
				float dx = pParticle.pos.x - myParam.pParticles[j].pos.x;
				float dy = pParticle.pos.y - myParam.pParticles[j].pos.y;
				if (dx * dx + dy * dy < distanceThreshold * distanceThreshold) {
					neighborCountsSelect[i]++;
					neighborCountsSelect[j]++;
				}
			}

			if (neighborCountsSelect[i] > 3 && !myParam.rParticles[i].isDarkMatter) {
				myParam.rParticles[i].isSelected = true;
			}
		}
		selectManyClusters = false;
	}
}

void ParticleSelection::boxSelection(UpdateParameters& myParam) {

	if ((IsKeyDown(KEY_LEFT_CONTROL) && IsMouseButtonDown(2)) || (IsKeyDown(KEY_LEFT_ALT) && IsMouseButtonDown(2))) {
		if (IO::handleShortcut(KEY_LEFT_CONTROL) || IsMouseButtonPressed(2)) {
			boxInitialPos = { myParam.myCamera.mouseWorldPos.x, myParam.myCamera.mouseWorldPos.y };
			isBoxSelecting = true;
		}

		glm::vec2 currentMousePos = { myParam.myCamera.mouseWorldPos.x, myParam.myCamera.mouseWorldPos.y };
		float boxX = fmin(boxInitialPos.x, currentMousePos.x);
		float boxY = fmin(boxInitialPos.y, currentMousePos.y);
		float boxWidth = fabs(currentMousePos.x - boxInitialPos.x);
		float boxHeight = fabs(currentMousePos.y - boxInitialPos.y);
		DrawRectangleV({boxX, boxY }, { boxWidth, boxHeight }, {40,40,40,80});
		DrawRectangleLinesEx({ boxX, boxY , boxWidth, boxHeight }, 0.6f, WHITE);

	}

	if (IsKeyDown(KEY_LEFT_ALT) && isBoxSelecting) {
		isBoxDeselecting = true;
	}

	if ((IsKeyReleased(KEY_LEFT_CONTROL) || IsMouseButtonReleased(2)) && isBoxSelecting) {
		glm::vec2 mousePos = { myParam.myCamera.mouseWorldPos.x, myParam.myCamera.mouseWorldPos.y };
		float boxX1 = fmin(boxInitialPos.x, mousePos.x);
		float boxX2 = fmax(boxInitialPos.x, mousePos.x);
		float boxY1 = fmin(boxInitialPos.y, mousePos.y);
		float boxY2 = fmax(boxInitialPos.y, mousePos.y);

		if (!IsKeyDown(KEY_LEFT_SHIFT) && !isBoxDeselecting) {
			for (size_t i = 0; i < myParam.pParticles.size(); i++) {
				myParam.rParticles[i].isSelected = false;
			}
		}

		for (size_t i = 0; i < myParam.pParticles.size(); i++) {

			if (myParam.pParticles[i].pos.x >= boxX1 && myParam.pParticles[i].pos.x <= boxX2 &&
				myParam.pParticles[i].pos.y >= boxY1 && myParam.pParticles[i].pos.y <= boxY2) {
				if (!isBoxDeselecting) {
					myParam.rParticles[i].isSelected = true;

				}
				else if(myParam.rParticles[i].isSelected && isBoxDeselecting) {
					myParam.rParticles[i].isSelected = false;
				}

			}
		}

		isBoxSelecting = false;
		isBoxDeselecting = false;
	}
}

void ParticleSelection::invertSelection(std::vector<ParticleRendering>& rParticles){
	if (IO::handleShortcut(KEY_I)) {
		invertParticleSelection = true;
	}

	if (invertParticleSelection) {
		for (auto& rParticle : rParticles) {

			rParticle.isSelected = !rParticle.isSelected;
		}

		invertParticleSelection = false;
	}
}

void ParticleSelection::deselection(std::vector<ParticleRendering>& rParticles){

	if (deselectParticles || IO::handleShortcut(KEY_D)) {
		for (auto& rParticle : rParticles) {
			rParticle.isSelected = false;
		}
		deselectParticles = false;
	}
}

void ParticleSelection::selectedParticlesStoring(UpdateParameters& myParam) {
	myParam.rParticlesSelected.clear();
	myParam.pParticlesSelected.clear();
	for (size_t i = 0; i < myParam.pParticles.size(); i++) {
		if (myParam.rParticles[i].isSelected) {
			myParam.rParticlesSelected.push_back(myParam.rParticles[i]);
			myParam.pParticlesSelected.push_back(myParam.pParticles[i]);
		}
	}
}
