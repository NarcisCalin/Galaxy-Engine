#include "particleSelection.h"

ParticleSelection::ParticleSelection() {
}

void ParticleSelection::clusterSelection(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, SceneCamera& myCamera,
	bool& isMouseNotHoveringUI, ParticleTrails& trails, bool& isGlobalTrailsEnabled) {
	static bool isMouseMoving = false;
	static Vector2 dragStartPos = { 0 };

	if (IsMouseButtonPressed(0) && IsKeyDown(KEY_LEFT_CONTROL) && isMouseNotHoveringUI) {
		dragStartPos = GetMousePosition();
		isMouseMoving = false;
	}

	if (IsMouseButtonDown(0) && IsKeyDown(KEY_LEFT_CONTROL) && isMouseNotHoveringUI) {
		Vector2 currentPos = GetMousePosition();
		float dragThreshold = 5.0f;
		float dx = currentPos.x - dragStartPos.x;
		float dy = currentPos.y - dragStartPos.y;

		if (dx * dx + dy * dy > dragThreshold * dragThreshold) {
			isMouseMoving = true;
		}
	}

	if (IsMouseButtonReleased(0) && IsKeyDown(KEY_LEFT_CONTROL) && !isMouseMoving && isMouseNotHoveringUI) {
		float distanceThreshold = 10.0f;
		std::vector<int> neighborCountsSelect(pParticles.size(), 0);
		for (size_t i = 0; i < pParticles.size(); i++) {
			const auto& pParticle = pParticles[i];
			for (size_t j = i + 1; j < pParticles.size(); j++) {
				if (std::abs(pParticles[j].pos.x - pParticle.pos.x) > 2.4f) break;
				float dx = pParticle.pos.x - pParticles[j].pos.x;
				float dy = pParticle.pos.y - pParticles[j].pos.y;
				if (dx * dx + dy * dy < distanceThreshold * distanceThreshold) {
					neighborCountsSelect[i]++;
					neighborCountsSelect[j]++;
				}
			}
		}

		for (size_t i = 0; i < pParticles.size(); i++) {
			if (!IsKeyDown(KEY_LEFT_SHIFT)) {
				rParticles[i].isSelected = false;
				if (!isGlobalTrailsEnabled) {
					trails.trailDots.clear();
				}
			}
			float dx = pParticles[i].pos.x - myCamera.mouseWorldPos.x;
			float dy = pParticles[i].pos.y - myCamera.mouseWorldPos.y;
			float distanceSq = dx * dx + dy * dy;
			if (distanceSq < selectionThresholdSq && neighborCountsSelect[i] > 3) {
				rParticles[i].isSelected = true;
			}
			else {
				if (!IsKeyDown(KEY_LEFT_SHIFT) && !pParticles.empty()) {
					rParticles[i].isSelected = false;
					if (!isGlobalTrailsEnabled) {
						trails.trailDots.clear();
					}
				}
			}
		}
	}
}

void ParticleSelection::particleSelection(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, SceneCamera& myCamera,
	bool& isMouseNotHoveringUI, ParticleTrails& trails, bool& isGlobalTrailsEnabled) {
	static bool isMouseMoving = false;
	static Vector2 dragStartPos = { 0 };

	if (IsMouseButtonPressed(0) && IsKeyDown(KEY_LEFT_ALT) && isMouseNotHoveringUI) {
		dragStartPos = GetMousePosition();
		isMouseMoving = false;
	}

	if (IsMouseButtonDown(0) && IsKeyDown(KEY_LEFT_ALT) && isMouseNotHoveringUI) {
		Vector2 currentPos = GetMousePosition();
		float dragThreshold = 5.0f;
		float dx = currentPos.x - dragStartPos.x;
		float dy = currentPos.y - dragStartPos.y;

		if (dx * dx + dy * dy > dragThreshold * dragThreshold) {
			isMouseMoving = true;
		}
	}

	if (IsMouseButtonReleased(0) && IsKeyDown(KEY_LEFT_ALT) && !isMouseMoving && isMouseNotHoveringUI) {
		size_t closestIndex = 0;
		float minDistanceSq = std::numeric_limits<float>::max();

		for (size_t i = 0; i < pParticles.size(); i++) {
			float dx = pParticles[i].pos.x - myCamera.mouseWorldPos.x;
			float dy = pParticles[i].pos.y - myCamera.mouseWorldPos.y;
			float currentDistanceSq = dx * dx + dy * dy;
			if (currentDistanceSq < minDistanceSq) {
				minDistanceSq = currentDistanceSq;
				closestIndex = i;
			}
		}

		if (!IsKeyDown(KEY_LEFT_SHIFT)) {
			bool wasClosestSelected = (minDistanceSq < selectionThresholdSq && !pParticles.empty()) ?
				rParticles[closestIndex].isSelected : false;

			for (size_t i = 0; i < pParticles.size(); i++) {
				rParticles[i].isSelected = false;
			}

			if (!isGlobalTrailsEnabled) {
				trails.trailDots.clear();
			}

			if (minDistanceSq < selectionThresholdSq && !pParticles.empty() && !wasClosestSelected) {
				rParticles[closestIndex].isSelected = true;
			}
		}
		else {
			if (minDistanceSq < selectionThresholdSq && !pParticles.empty()) {
				rParticles[closestIndex].isSelected = !rParticles[closestIndex].isSelected;
			}
		}
	}
}


void ParticleSelection::manyClustersSelection(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, ParticleTrails& trails,
	bool& isGlobalTrailsEnabled) {
	if (IsKeyPressed(KEY_D)) {
		if (!isGlobalTrailsEnabled) {
			trails.trailDots.clear();
		}
		float distanceThreshold = 10.0f;
		std::vector<int> neighborCountsSelect(pParticles.size(), 0);
		for (size_t i = 0; i < pParticles.size(); i++) {
			rParticles[i].isSelected = false;
			const auto& pParticle = pParticles[i];
			for (size_t j = i + 1; j < pParticles.size(); j++) {
				if (std::abs(pParticles[j].pos.x - pParticle.pos.x) > 2.4f) break;
				float dx = pParticle.pos.x - pParticles[j].pos.x;
				float dy = pParticle.pos.y - pParticles[j].pos.y;
				if (dx * dx + dy * dy < distanceThreshold * distanceThreshold) {
					neighborCountsSelect[i]++;
					neighborCountsSelect[j]++;
				}
			}

			if (neighborCountsSelect[i] > 3) {
				rParticles[i].isSelected = true;
			}
		}
	}
}

void ParticleSelection::boxSelection(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, SceneCamera& myCamera) {

	if (IsKeyDown(KEY_LEFT_CONTROL) && IsMouseButtonDown(2) || IsKeyDown(KEY_LEFT_ALT) && IsMouseButtonDown(2)) {
		if (IsKeyPressed(KEY_LEFT_CONTROL) || IsMouseButtonPressed(2)) {
			boxInitialPos = { myCamera.mouseWorldPos.x, myCamera.mouseWorldPos.y };
			isBoxSelecting = true;
		}

		Vector2 currentMousePos = { myCamera.mouseWorldPos.x, myCamera.mouseWorldPos.y };
		float boxX = fmin(boxInitialPos.x, currentMousePos.x);
		float boxY = fmin(boxInitialPos.y, currentMousePos.y);
		float boxWidth = fabs(currentMousePos.x - boxInitialPos.x);
		float boxHeight = fabs(currentMousePos.y - boxInitialPos.y);
		DrawRectangleV({ boxX, boxY }, { boxWidth, boxHeight }, RED);
	}

	if (IsKeyDown(KEY_LEFT_ALT) && isBoxSelecting) {
		isBoxDeselecting = true;
	}

	if ((IsKeyReleased(KEY_LEFT_CONTROL) || IsMouseButtonReleased(2)) && isBoxSelecting) {
		Vector2 mousePos = { myCamera.mouseWorldPos.x, myCamera.mouseWorldPos.y };
		float boxX1 = fmin(boxInitialPos.x, mousePos.x);
		float boxX2 = fmax(boxInitialPos.x, mousePos.x);
		float boxY1 = fmin(boxInitialPos.y, mousePos.y);
		float boxY2 = fmax(boxInitialPos.y, mousePos.y);

		if (!IsKeyDown(KEY_LEFT_SHIFT) && !isBoxDeselecting) {
			for (size_t i = 0; i < pParticles.size(); i++) {
				rParticles[i].isSelected = false;
			}
		}

		for (size_t i = 0; i < pParticles.size(); i++) {
			if (pParticles[i].pos.x >= boxX1 && pParticles[i].pos.x <= boxX2 &&
				pParticles[i].pos.y >= boxY1 && pParticles[i].pos.y <= boxY2) {
				if (!isBoxDeselecting) {
					rParticles[i].isSelected = true;

				}
				else if(rParticles[i].isSelected && isBoxDeselecting) {
					rParticles[i].isSelected = false;
				}

			}
		}

		isBoxSelecting = false;
		isBoxDeselecting = false;
	}
}

void ParticleSelection::invertSelection(std::vector<ParticleRendering>& rParticles){
	if (IsKeyPressed(KEY_I)) {
		invertParticleSelection = true;
	}

	if (invertParticleSelection) {
		for (auto& rParticle : rParticles) {
			rParticle.isSelected = !rParticle.isSelected;
		}

		invertParticleSelection = false;
	}
}

void ParticleSelection::selectedParticlesStoring(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, std::vector<ParticleRendering>& rParticlesSelected,
	std::vector<ParticlePhysics>& pParticlesSelected) {
	rParticlesSelected.clear();
	pParticlesSelected.clear();
	for (size_t i = 0; i < pParticles.size(); i++) {
		if (rParticles[i].isSelected) {
			rParticlesSelected.push_back(rParticles[i]);
			pParticlesSelected.push_back(pParticles[i]);
		}
	}
}
