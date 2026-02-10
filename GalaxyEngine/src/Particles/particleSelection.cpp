#include "Particles/particleSelection.h"

#include "parameters.h"

ParticleSelection::ParticleSelection() {
}

void ParticleSelection::clusterSelection(UpdateVariables& myVar, UpdateParameters& myParam, bool externalTrigger) {
	static bool isMouseMoving = false;
	static glm::vec2 dragStartPos = { 0.0f, 0.0f };

	if ((IsMouseButtonPressed(0) && IsKeyDown(KEY_LEFT_CONTROL) && myVar.isMouseNotHoveringUI) || externalTrigger) {
		dragStartPos = glm::vec2(GetMousePosition().x, GetMousePosition().y);
		isMouseMoving = false;
	}

	if ((IsMouseButtonDown(0) && IsKeyDown(KEY_LEFT_CONTROL) && myVar.isMouseNotHoveringUI) || externalTrigger) {
		glm::vec2 currentPos = glm::vec2(GetMousePosition().x, GetMousePosition().y);
		float dragThreshold = 5.0f;
		float dx = currentPos.x - dragStartPos.x;
		float dy = currentPos.y - dragStartPos.y;

		if (dx * dx + dy * dy > dragThreshold * dragThreshold) {
			isMouseMoving = true;
		}
	}

	if ((IsMouseButtonReleased(0) && IsKeyDown(KEY_LEFT_CONTROL) && !isMouseMoving && myVar.isMouseNotHoveringUI) || externalTrigger) {
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

		if (!IsKeyDown(KEY_LEFT_SHIFT)) {
			if (!myVar.isGlobalTrailsEnabled) {
				myParam.trails.segments.clear();
			}
		}

		for (size_t i = 0; i < myParam.pParticles.size(); i++) {
			if (!IsKeyDown(KEY_LEFT_SHIFT)) {
				myParam.rParticles[i].isSelected = false;
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
						myParam.trails.segments.clear();
					}
				}
			}
		}
	}
}

void ParticleSelection::particleSelection(UpdateVariables& myVar, UpdateParameters& myParam, bool externalTrigger) {
	static bool isMouseMoving = false;
	static glm::vec2 dragStartPos = { 0.0f, 0.0f };

	if ((IsMouseButtonPressed(0) && IsKeyDown(KEY_LEFT_ALT) && myVar.isMouseNotHoveringUI) || externalTrigger) {
		dragStartPos = glm::vec2(GetMousePosition().x, GetMousePosition().y);
		isMouseMoving = false;
	}

	if ((IsMouseButtonDown(0) && IsKeyDown(KEY_LEFT_ALT) && myVar.isMouseNotHoveringUI) || externalTrigger) {
		glm::vec2 currentPos = glm::vec2(GetMousePosition().x, GetMousePosition().y);
		float dragThreshold = 5.0f;
		float dx = currentPos.x - dragStartPos.x;
		float dy = currentPos.y - dragStartPos.y;

		if (dx * dx + dy * dy > dragThreshold * dragThreshold) {
			isMouseMoving = true;
		}
	}

	if ((IsMouseButtonReleased(0) && IsKeyDown(KEY_LEFT_ALT) && !isMouseMoving && myVar.isMouseNotHoveringUI) || externalTrigger) {
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
				myParam.trails.segments.clear();
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
			myParam.trails.segments.clear();
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

void ParticleSelection::boxSelection(UpdateParameters& myParam, bool& is3DMode) {

	if ((IsKeyDown(KEY_LEFT_CONTROL) && IsMouseButtonDown(2)) || (IsKeyDown(KEY_LEFT_ALT) && IsMouseButtonDown(2))) {
		if (IO::shortcutPress(KEY_LEFT_CONTROL) || IsMouseButtonPressed(2)) {
			boxInitialPos = { myParam.myCamera.mouseWorldPos.x, myParam.myCamera.mouseWorldPos.y };
			isBoxSelecting = true;
		}

		glm::vec2 currentMousePos = { myParam.myCamera.mouseWorldPos.x, myParam.myCamera.mouseWorldPos.y };
		float boxX = fmin(boxInitialPos.x, currentMousePos.x);
		float boxY = fmin(boxInitialPos.y, currentMousePos.y);
		float boxWidth = fabs(currentMousePos.x - boxInitialPos.x);
		float boxHeight = fabs(currentMousePos.y - boxInitialPos.y);
		DrawRectangleV({ boxX, boxY }, { boxWidth, boxHeight }, { 40,40,40,80 });
		if (!is3DMode) {
			DrawRectangleLinesEx({ boxX, boxY , boxWidth, boxHeight }, 0.6f, WHITE);
		}

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
				else if (myParam.rParticles[i].isSelected && isBoxDeselecting) {
					myParam.rParticles[i].isSelected = false;
				}

			}
		}

		isBoxSelecting = false;
		isBoxDeselecting = false;
	}
}

void ParticleSelection::invertSelection(std::vector<ParticleRendering>& rParticles) {
	if (IO::shortcutPress(KEY_I)) {
		invertParticleSelection = true;
	}

	if (invertParticleSelection) {
		for (auto& rParticle : rParticles) {

			rParticle.isSelected = !rParticle.isSelected;
		}

		invertParticleSelection = false;
	}
}

void ParticleSelection::deselection(std::vector<ParticleRendering>& rParticles) {

	if (deselectParticles || IO::shortcutPress(KEY_D)) {
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

// ---- 3D IMPLEMENTATION ---- //

ParticleSelection3D::ParticleSelection3D() {
}

void ParticleSelection3D::clusterSelection(UpdateVariables& myVar, UpdateParameters& myParam, bool externalTrigger) {
	static bool isMouseMoving = false;
	static glm::vec2 dragStartPos = { 0.0f, 0.0f };

	if ((IsMouseButtonPressed(0) && IsKeyDown(KEY_LEFT_CONTROL) && myVar.isMouseNotHoveringUI) || externalTrigger) {
		dragStartPos = glm::vec2(GetMousePosition().x, GetMousePosition().y);
		isMouseMoving = false;
	}
	
	if ((IsMouseButtonDown(0) && IsKeyDown(KEY_LEFT_CONTROL) && myVar.isMouseNotHoveringUI) || externalTrigger) {
		glm::vec2 currentPos = glm::vec2(GetMousePosition().x, GetMousePosition().y);
		float dragThreshold = 5.0f;
		float dx = currentPos.x - dragStartPos.x;
		float dy = currentPos.y - dragStartPos.y;

		if (dx * dx + dy * dy > dragThreshold * dragThreshold) {
			isMouseMoving = true;
		}
	}

	if ((IsMouseButtonReleased(0) && IsKeyDown(KEY_LEFT_CONTROL) && !isMouseMoving && myVar.isMouseNotHoveringUI) || externalTrigger) {
		float distanceThreshold = 100.0f;

		Ray ray = GetScreenToWorldRay(GetMousePosition(), myParam.myCamera3D.cam3D);
		glm::vec3 rayPos{ ray.position.x, ray.position.y, ray.position.z };
		glm::vec3 rayDir{ ray.direction.x, ray.direction.y, ray.direction.z };

		std::vector<int> neighborCountsSelect(myParam.pParticles3D.size(), 0);

		for (size_t i = 0; i < myParam.pParticles3D.size(); i++) {
			const auto& pParticle = myParam.pParticles3D[i];

			if (myParam.rParticles3D[i].isDarkMatter) {
				continue;
			}

			for (size_t j = i + 1; j < myParam.pParticles3D.size(); j++) {

				if (myParam.rParticles3D[j].isDarkMatter) {
					continue;
				}

				if (std::abs(myParam.pParticles3D[j].pos.x - pParticle.pos.x) > 3.4f) break;

				glm::vec3 diff = myParam.pParticles3D[j].pos - pParticle.pos;
				float dSq = glm::dot(diff, diff);

				if (dSq < distanceThreshold * distanceThreshold) {
					neighborCountsSelect[i]++;
					neighborCountsSelect[j]++;
				}
			}
		}

		if (!IsKeyDown(KEY_LEFT_SHIFT)) {
			if (!myVar.isGlobalTrailsEnabled) {
				myParam.trails.segments.clear();
			}
		}

		for (size_t i = 0; i < myParam.pParticles3D.size(); i++) {
			if (!IsKeyDown(KEY_LEFT_SHIFT)) {
				myParam.rParticles3D[i].isSelected = false;
			}

			glm::vec3 particleCursorRayDir = glm::normalize(rayPos - myParam.pParticles3D[i].pos);

			float alignment = glm::dot(rayDir, particleCursorRayDir);

			if (alignment < selectionThresholdAngle && neighborCountsSelect[i] > 4 && !myParam.rParticles3D[i].isDarkMatter) {
				myParam.rParticles3D[i].isSelected = true;
			}
			else {
				if (!IsKeyDown(KEY_LEFT_SHIFT) && !myParam.pParticles3D.empty()) {
					myParam.rParticles3D[i].isSelected = false;
					if (!myVar.isGlobalTrailsEnabled) {
						myParam.trails.segments.clear();
					}
				}
			}
		}
	}
}

void ParticleSelection3D::particleSelection(UpdateVariables& myVar, UpdateParameters& myParam, bool externalTrigger) {
	static bool isMouseMoving = false;
	static glm::vec2 dragStartPos = { 0.0f, 0.0f };

	if ((IsMouseButtonPressed(0) && IsKeyDown(KEY_LEFT_ALT) && myVar.isMouseNotHoveringUI) || externalTrigger) {
		dragStartPos = glm::vec2(GetMousePosition().x, GetMousePosition().y);
		isMouseMoving = false;
	}

	if ((IsMouseButtonDown(0) && IsKeyDown(KEY_LEFT_ALT) && myVar.isMouseNotHoveringUI) || externalTrigger) {
		glm::vec2 currentPos = glm::vec2(GetMousePosition().x, GetMousePosition().y);
		float dragThreshold = 5.0f;
		float dx = currentPos.x - dragStartPos.x;
		float dy = currentPos.y - dragStartPos.y;

		if (dx * dx + dy * dy > dragThreshold * dragThreshold) {
			isMouseMoving = true;
		}
	}

	if ((IsMouseButtonReleased(0) && IsKeyDown(KEY_LEFT_ALT) && !isMouseMoving && myVar.isMouseNotHoveringUI) || externalTrigger) {
		size_t closestIndex = 0;
		float minAngle = std::numeric_limits<float>::max();

		Ray ray = GetScreenToWorldRay(GetMousePosition(), myParam.myCamera3D.cam3D);
		glm::vec3 rayPos{ ray.position.x, ray.position.y, ray.position.z };
		glm::vec3 rayDir{ ray.direction.x, ray.direction.y, ray.direction.z };

		for (size_t i = 0; i < myParam.pParticles3D.size(); i++) {

			if (myParam.rParticles3D[i].isDarkMatter) {
				continue;
			}

			glm::vec3 particleCursorRayDir = glm::normalize(rayPos - myParam.pParticles3D[i].pos);

			float alignment = glm::dot(rayDir, particleCursorRayDir);

			if (alignment < minAngle) {
				minAngle = alignment;
				closestIndex = i;
			}
		}

		if (!IsKeyDown(KEY_LEFT_SHIFT)) {
			bool wasClosestSelected = (minAngle < selectionThresholdAngle && !myParam.pParticles3D.empty()) ?
				myParam.rParticles3D[closestIndex].isSelected : false;

			for (size_t i = 0; i < myParam.pParticles3D.size(); i++) {
				myParam.rParticles3D[i].isSelected = false;
			}

			if (!myVar.isGlobalTrailsEnabled) {
				myParam.trails.segments.clear();
			}

			if (minAngle < selectionThresholdAngle && !myParam.pParticles3D.empty() && !wasClosestSelected && !myParam.rParticles3D[closestIndex].isDarkMatter) {
				myParam.rParticles3D[closestIndex].isSelected = true;
			}
		}
		else {
			if (minAngle < selectionThresholdAngle && !myParam.pParticles3D.empty()) {
				myParam.rParticles3D[closestIndex].isSelected = !myParam.rParticles3D[closestIndex].isSelected;
			}
		}
	}
}


void ParticleSelection3D::manyClustersSelection(UpdateVariables& myVar, UpdateParameters& myParam) {
	if (selectManyClusters3D) {
		if (!myVar.isGlobalTrailsEnabled) {
			myParam.trails.segments.clear();
		}
		float distanceThreshold = 100.0f;
		std::vector<int> neighborCountsSelect(myParam.pParticles3D.size(), 0);
		for (size_t i = 0; i < myParam.pParticles3D.size(); i++) {
			myParam.rParticles3D[i].isSelected = false;

			if (myParam.rParticles3D[i].isDarkMatter) {
				continue;
			}

			const auto& pParticle = myParam.pParticles3D[i];
			for (size_t j = i + 1; j < myParam.pParticles3D.size(); j++) {

				if (myParam.rParticles3D[j].isDarkMatter) {
					continue;
				}

				if (std::abs(myParam.pParticles3D[j].pos.x - pParticle.pos.x) > 3.4f) break;

				glm::vec3 diff = myParam.pParticles3D[j].pos - pParticle.pos;
				float dSq = glm::dot(diff, diff);

				if (dSq < distanceThreshold * distanceThreshold) {
					neighborCountsSelect[i]++;
					neighborCountsSelect[j]++;
				}
			}

			if (neighborCountsSelect[i] > 4 && !myParam.rParticles3D[i].isDarkMatter) {
				myParam.rParticles3D[i].isSelected = true;
			}
		}
		selectManyClusters3D = false;
	}
}

void ParticleSelection3D::boxSelection(UpdateParameters& myParam) {

	if ((IsKeyDown(KEY_LEFT_CONTROL) && IsMouseButtonDown(2)) || (IsKeyDown(KEY_LEFT_ALT) && IsMouseButtonDown(2))) {
		if (IO::shortcutPress(KEY_LEFT_CONTROL) || IsMouseButtonPressed(2)) {
			boxInitialPos = { GetMousePosition().x, GetMousePosition().y };
			isBoxSelecting = true;
		}

		glm::vec2 currentMousePos = {GetMousePosition().x, GetMousePosition().y};
		float boxX = fmin(boxInitialPos.x, currentMousePos.x);
		float boxY = fmin(boxInitialPos.y, currentMousePos.y);
		float boxWidth = fabs(currentMousePos.x - boxInitialPos.x);
		float boxHeight = fabs(currentMousePos.y - boxInitialPos.y);

	}

	if (IsKeyDown(KEY_LEFT_ALT) && isBoxSelecting) {
		isBoxDeselecting = true;
	}

	if ((IsKeyReleased(KEY_LEFT_CONTROL) || IsMouseButtonReleased(2)) && isBoxSelecting) {
		glm::vec2 mousePos = { GetMousePosition().x, GetMousePosition().y };
		float boxX1 = fmin(boxInitialPos.x, mousePos.x);
		float boxX2 = fmax(boxInitialPos.x, mousePos.x);
		float boxY1 = fmin(boxInitialPos.y, mousePos.y);
		float boxY2 = fmax(boxInitialPos.y, mousePos.y);

		if (!IsKeyDown(KEY_LEFT_SHIFT) && !isBoxDeselecting) {
			for (size_t i = 0; i < myParam.pParticles3D.size(); i++) {
				myParam.rParticles3D[i].isSelected = false;
			}
		}

		for (size_t i = 0; i < myParam.pParticles3D.size(); i++) {

			ParticlePhysics3D& p = myParam.pParticles3D[i];

			Vector2 p3dTo2d = GetWorldToScreen({ p.pos.x, p.pos.y, p.pos.z }, myParam.myCamera3D.cam3D);

			if (p3dTo2d.x >= boxX1 && p3dTo2d.x <= boxX2 &&
				p3dTo2d.y >= boxY1 && p3dTo2d.y <= boxY2) {
				if (!isBoxDeselecting) {
					myParam.rParticles3D[i].isSelected = true;

				}
				else if (myParam.rParticles3D[i].isSelected && isBoxDeselecting) {
					myParam.rParticles3D[i].isSelected = false;
				}

			}
		}

		isBoxSelecting = false;
		isBoxDeselecting = false;
	}
}

void ParticleSelection3D::invertSelection(std::vector<ParticleRendering3D>& rParticles) {
	if (IO::shortcutPress(KEY_I)) {
		invertParticleSelection = true;
	}

	if (invertParticleSelection) {
		for (auto& rParticle : rParticles) {

			rParticle.isSelected = !rParticle.isSelected;
		}

		invertParticleSelection = false;
	}
}

void ParticleSelection3D::deselection(std::vector<ParticleRendering3D>& rParticles) {

	if (deselectParticles || IO::shortcutPress(KEY_D)) {
		for (auto& rParticle : rParticles) {
			rParticle.isSelected = false;
		}
		deselectParticles = false;
	}
}

void ParticleSelection3D::selectedParticlesStoring(UpdateParameters& myParam) {
	myParam.rParticlesSelected3D.clear();
	myParam.pParticlesSelected3D.clear();
	for (size_t i = 0; i < myParam.pParticles3D.size(); i++) {
		if (myParam.rParticles3D[i].isSelected) {
			myParam.rParticlesSelected3D.push_back(myParam.rParticles3D[i]);
			myParam.pParticlesSelected3D.push_back(myParam.pParticles3D[i]);
		}
	}
}
