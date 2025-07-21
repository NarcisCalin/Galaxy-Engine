#include "UX/camera.h"

#include "parameters.h"

SceneCamera::SceneCamera() {
	camera.offset = { 0.0f, 0.0f };
	camera.target = { 0.0f, 0.0f };
	camera.rotation = 0.0f;
	camera.zoom = 0.5f;
	mouseWorldPos = { 0.0f, 0.0f };
	panFollowingOffset = { 0.0f, 0.0f };
	isFollowing = false;
	centerCamera = false;
	cameraChangedThisFrame = false;
	previousColor = { 128,128,128,255 };
	followPosition = { 0.0f, 0.0f };
	delta = { 0.0f, 0.0f };
}

Camera2D SceneCamera::cameraLogic(bool& loadFlag, bool& isMouseNotHoveringUI) {

	if (IsMouseButtonDown(1))
	{
		delta = glm::vec2(GetMouseDelta().x, GetMouseDelta().y);
		delta = delta * (-1.0f / camera.zoom);
		camera.target.x = camera.target.x + delta.x;
		camera.target.y = camera.target.y + delta.y;
		panFollowingOffset = panFollowingOffset + delta;

	}

	float wheel = GetMouseWheelMove();
	if (wheel != 0 && !IsKeyDown(KEY_LEFT_CONTROL) && !loadFlag && isMouseNotHoveringUI) {
		mouseWorldPos = glm::vec2(GetScreenToWorld2D(GetMousePosition(), camera).x,
			GetScreenToWorld2D(GetMousePosition(), camera).y);

		if (isFollowing) {
			glm::vec2 screenCenter = { GetScreenWidth() * 0.5f, GetScreenHeight() * 0.5f };

			mouseWorldPos = glm::vec2(GetScreenToWorld2D({ screenCenter.x,   screenCenter.y}, camera).x, 
				GetScreenToWorld2D({ screenCenter.x,   screenCenter.y }, camera).y);

			camera.offset = { screenCenter.x, screenCenter.y };
			camera.target = { mouseWorldPos.x, mouseWorldPos.y };
		}
		else {

			mouseWorldPos = glm::vec2(GetScreenToWorld2D(GetMousePosition(), camera).x, GetScreenToWorld2D(GetMousePosition(), camera).y);
			camera.offset = GetMousePosition();
			camera.target = { mouseWorldPos.x, mouseWorldPos.y };
		}

		float scale = 0.2f * wheel;
		camera.zoom = Clamp(expf(logf(camera.zoom) + scale), 0.475f, 64.0f);
	}

	// RESET CAMERA
	if (IO::shortcutPress(KEY_F)) {
		camera.zoom = defaultCamZoom;
		camera.target = { 0.0f, 0.0f };
		camera.offset = { 0.0f, 0.0f };
		panFollowingOffset = { 0.0f, 0.0f };
	}

	return camera;
}

void SceneCamera::cameraFollowObject(UpdateVariables& myVar, UpdateParameters& myParam) {

	mouseWorldPos = glm::vec2(GetScreenToWorld2D(GetMousePosition(), camera).x, GetScreenToWorld2D(GetMousePosition(), camera).y);

	static bool isDragging = false;
	static glm::vec2 dragStartPos = { 0.0f, 0.0f };

	if ((IsMouseButtonPressed(1) && IsKeyDown(KEY_LEFT_CONTROL) && myVar.isMouseNotHoveringUI) || 
		(IsMouseButtonPressed(1) && IsKeyDown(KEY_LEFT_ALT) && myVar.isMouseNotHoveringUI)) {
		dragStartPos = glm::vec2(GetMousePosition().x, GetMousePosition().y);
		isDragging = false;
	}

	if ((IsMouseButtonDown(1) && IsKeyDown(KEY_LEFT_CONTROL) && myVar.isMouseNotHoveringUI) ||
		(IsMouseButtonDown(1) && IsKeyDown(KEY_LEFT_ALT) && myVar.isMouseNotHoveringUI)) {
		glm::vec2 currentPos = glm::vec2(GetMousePosition().x, GetMousePosition().y);
		float dragThreshold = 5.0f;

		glm::vec2 d = currentPos - dragStartPos;

		if (d.x * d.x + d.y * d.y > dragThreshold * dragThreshold) {
			isDragging = true;
		}
	}

	if (IsMouseButtonReleased(1) && IsKeyDown(KEY_LEFT_CONTROL) && !isDragging && myVar.isMouseNotHoveringUI) {
		float distanceThreshold = 10.0f;
		std::vector<int> neighborCountsSelect(myParam.pParticles.size(), 0);
		for (size_t i = 0; i < myParam.pParticles.size(); i++) {
			const auto& pParticle = myParam.pParticles[i];
			for (size_t j = i + 1; j < myParam.pParticles.size(); j++) {
				if (std::abs(myParam.pParticles[j].pos.x - pParticle.pos.x) > 2.4f) break;

				glm::vec2 d = pParticle.pos - myParam.pParticles[j].pos;

				if (d.x * d.x + d.y * d.y < distanceThreshold * distanceThreshold) {
					neighborCountsSelect[i]++;
					neighborCountsSelect[j]++;
				}
			}
		}

		isFollowing = true;
		panFollowingOffset = { 0.0f, 0.0f };

		for (size_t i = 0; i < myParam.pParticles.size(); i++) {
			myParam.rParticles[i].isSelected = false;

			glm::vec2 d = myParam.pParticles[i].pos - mouseWorldPos;

			float distanceSq = d.x * d.x + d.y * d.y;
			if (distanceSq < selectionThresholdSq && neighborCountsSelect[i] > 3) {
				myParam.rParticles[i].isSelected = true;
			}
		}

		if (myVar.isSelectedTrailsEnabled) {
			myParam.trails.trailDots.clear();
		}
	}

	if (IsMouseButtonReleased(1) && IsKeyDown(KEY_LEFT_ALT) && !isDragging && myVar.isMouseNotHoveringUI) {

		size_t closestIndex = 0;
		float minDistanceSq = std::numeric_limits<float>::max();

		for (size_t i = 0; i < myParam.pParticles.size(); i++) {
			myParam.rParticles[i].isSelected = false;

			glm::vec2 d = myParam.pParticles[i].pos - mouseWorldPos;

			float currentDistanceSq = d.x * d.x + d.y * d.y;
			if (currentDistanceSq < minDistanceSq) {
				minDistanceSq = currentDistanceSq;
				closestIndex = i;
			}
		}

		if (minDistanceSq < selectionThresholdSq && !myParam.pParticles.empty()) {
			myParam.rParticles[closestIndex].isSelected = true;
		}

		isFollowing = true;
		panFollowingOffset = { 0.0f, 0.0f };
		if (myVar.isSelectedTrailsEnabled) {
			myParam.trails.trailDots.clear();
		}
	}

	if (IO::shortcutPress(KEY_Z) || centerCamera) {
		panFollowingOffset = { 0.0f, 0.0f };
		isFollowing = true;
		centerCamera = false;
	}

	if (isFollowing) {

		glm::vec2 sum = glm::vec2(0.0f, 0.0f);

		float count = 0.0f;
		for (size_t i = 0; i < myParam.pParticles.size(); i++) {
			if (myParam.rParticles[i].isSelected) {
				sum += myParam.pParticles[i].pos;
				count++;
			}
		}

		followPosition = sum / count;

		followPosition = followPosition + panFollowingOffset;

		camera.target = { followPosition.x, followPosition.y };

		camera.offset = { GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f };

		if (IO::shortcutPress(KEY_F) || count == 0 || myParam.pParticles.size() == 0) {
			isFollowing = false;
			camera.zoom = defaultCamZoom;
			camera.target = { 0.0f, 0.0f };
			camera.offset = { 0.0f, 0.0f };
			panFollowingOffset = { 0.0f, 0.0f };
		}
	}
}

void SceneCamera::hasCamMoved() {

	cameraChangedThisFrame = false;

	if (lastTarget.x != camera.target.x || lastTarget.y != camera.target.y ||
		lastOffset.x != camera.offset.x || lastOffset.y != camera.offset.y ||
		lastZoom != camera.zoom || lastRotation != camera.rotation) {

		cameraChangedThisFrame = true;
	}
	else {
		cameraChangedThisFrame = false;
	}

	lastTarget = camera.target;
	lastOffset = camera.offset;
	lastZoom = camera.zoom;
	lastRotation = camera.rotation;
}


