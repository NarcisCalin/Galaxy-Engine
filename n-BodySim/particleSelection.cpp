#include "particleSelection.h"

ParticleSelection::ParticleSelection() {
}

void ParticleSelection::clusterSelection(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, SceneCamera& myCamera,
	bool& isMouseNotHoveringUI, ParticleTrails& trails, bool& isGlobalTrailsEnabled) {
	static bool isDragging = false;
	static Vector2 dragStartPos = { 0 };

	if (IsMouseButtonPressed(0) && IsKeyDown(KEY_LEFT_CONTROL) && isMouseNotHoveringUI) {
		dragStartPos = GetMousePosition();
		isDragging = false;
	}

	if (IsMouseButtonDown(0) && IsKeyDown(KEY_LEFT_CONTROL) && isMouseNotHoveringUI) {
		Vector2 currentPos = GetMousePosition();
		float dragThreshold = 5.0f;
		float dx = currentPos.x - dragStartPos.x;
		float dy = currentPos.y - dragStartPos.y;

		if (dx * dx + dy * dy > dragThreshold * dragThreshold) {
			isDragging = true;
		}
	}

	if (IsMouseButtonReleased(0) && IsKeyDown(KEY_LEFT_CONTROL) && !isDragging && isMouseNotHoveringUI) {
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
				rParticles[i].color = { 255,128,128,255 };
			}
			else {
				if (!IsKeyDown(KEY_LEFT_SHIFT)) {
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
	static bool isDragging = false;
	static Vector2 dragStartPos = { 0 };

	if (IsMouseButtonPressed(0) && IsKeyDown(KEY_LEFT_ALT) && isMouseNotHoveringUI) {
		dragStartPos = GetMousePosition();
		isDragging = false;
	}

	if (IsMouseButtonDown(0) && IsKeyDown(KEY_LEFT_ALT) && isMouseNotHoveringUI) {
		Vector2 currentPos = GetMousePosition();
		float dragThreshold = 5.0f;
		float dx = currentPos.x - dragStartPos.x;
		float dy = currentPos.y - dragStartPos.y;

		if (dx * dx + dy * dy > dragThreshold * dragThreshold) {
			isDragging = true;
		}
	}

	if (IsMouseButtonReleased(0) && IsKeyDown(KEY_LEFT_ALT) && !isDragging && isMouseNotHoveringUI) {
		size_t closestIndex = 0;
		float minDistanceSq = std::numeric_limits<float>::max();

		for (size_t i = 0; i < pParticles.size(); i++) {
			if (!IsKeyDown(KEY_LEFT_SHIFT)) {
				rParticles[i].isSelected = false;

				if (!isGlobalTrailsEnabled) {
					trails.trailDots.clear();
				}
			}
			float dx = pParticles[i].pos.x - myCamera.mouseWorldPos.x;
			float dy = pParticles[i].pos.y - myCamera.mouseWorldPos.y;
			float currentDistanceSq = dx * dx + dy * dy;
			if (currentDistanceSq < minDistanceSq) {
				minDistanceSq = currentDistanceSq;
				closestIndex = i;
			}
		}

		if (minDistanceSq < selectionThresholdSq && !pParticles.empty()) {
			rParticles[closestIndex].isSelected = true;
			rParticles[closestIndex].color = { 255,128,128,255 };
		}
		else {
			if (!IsKeyDown(KEY_LEFT_SHIFT)) {
				rParticles[closestIndex].isSelected = false;
				if (!isGlobalTrailsEnabled) {
					trails.trailDots.clear();
				}
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
				rParticles[i].color = { 255,128,128,255 };
			}
		}
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
