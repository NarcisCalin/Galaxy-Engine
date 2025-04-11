#include "../../include/UX/camera.h"
#include <iostream>
#include <algorithm>
#include <limits>
#include "../../include/parameters.h"


SceneCamera::SceneCamera() {
	camera.offset = { 0.0f, 0.0f };
	camera.target = { 0.0f, 0.0f };
	camera.rotation = 0.0f;
	camera.zoom = 1.0f;
	mouseWorldPos = { 0.0f, 0.0f };
	panFollowingOffset = { 0.0f, 0.0f };
	isFollowing = false;
	centerCamera = false;
	previousColor = { 128,128,128,255 };
	followPosition = { 0.0f, 0.0f };
	delta = { 0.0f, 0.0f };
}

Camera2D SceneCamera::cameraLogic() {

	if (IsMouseButtonDown(1))
	{
		delta = GetMouseDelta();
		delta = Vector2Scale(delta, -1.0f / camera.zoom);
		camera.target = Vector2Add(camera.target, delta);
		panFollowingOffset = Vector2Add(panFollowingOffset, delta);

	}

	float wheel = GetMouseWheelMove();
	if (wheel != 0 && !IsKeyDown(KEY_LEFT_CONTROL)) {
		mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);

		if (isFollowing) {
			Vector2 screenCenter = { GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f };
			mouseWorldPos = GetScreenToWorld2D(screenCenter, camera);
			camera.offset = screenCenter;
			camera.target = mouseWorldPos;
		}
		else {

			mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);
			camera.offset = GetMousePosition();
			camera.target = mouseWorldPos;
		}

		float scale = 0.2f * wheel;
		camera.zoom = Clamp(expf(logf(camera.zoom) + scale), 0.95f, 64.0f);
	}

	// RESET CAMERA
	if (IsKeyPressed(KEY_F)) {
		camera.zoom = 1.0f;
		camera.target = { 0.0f };
		camera.offset = { 0.0f };
		panFollowingOffset = { 0.0f };
	}
	return camera;
}

void SceneCamera::cameraFollowObject(UpdateVariables& myVar, UpdateParameters& myParam) {

	mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);

	static bool isDragging = false;
	static Vector2 dragStartPos = { 0 };

	if ((IsMouseButtonPressed(1) && IsKeyDown(KEY_LEFT_CONTROL) && myVar.isMouseNotHoveringUI) || 
		(IsMouseButtonPressed(1) && IsKeyDown(KEY_LEFT_ALT) && myVar.isMouseNotHoveringUI)) {
		dragStartPos = GetMousePosition();
		isDragging = false;
	}

	if ((IsMouseButtonDown(1) && IsKeyDown(KEY_LEFT_CONTROL) && myVar.isMouseNotHoveringUI) ||
		(IsMouseButtonDown(1) && IsKeyDown(KEY_LEFT_ALT) && myVar.isMouseNotHoveringUI)) {
		Vector2 currentPos = GetMousePosition();
		float dragThreshold = 5.0f;
		float dx = currentPos.x - dragStartPos.x;
		float dy = currentPos.y - dragStartPos.y;

		if (dx * dx + dy * dy > dragThreshold * dragThreshold) {
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
				float dx = pParticle.pos.x - myParam.pParticles[j].pos.x;
				float dy = pParticle.pos.y - myParam.pParticles[j].pos.y;
				if (dx * dx + dy * dy < distanceThreshold * distanceThreshold) {
					neighborCountsSelect[i]++;
					neighborCountsSelect[j]++;
				}
			}
		}

		isFollowing = true;
		panFollowingOffset = { 0 };

		for (size_t i = 0; i < myParam.pParticles.size(); i++) {
			myParam.rParticles[i].isSelected = false;
			float dx = myParam.pParticles[i].pos.x - mouseWorldPos.x;
			float dy = myParam.pParticles[i].pos.y - mouseWorldPos.y;
			float distanceSq = dx * dx + dy * dy;
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
			float dx = myParam.pParticles[i].pos.x - mouseWorldPos.x;
			float dy = myParam.pParticles[i].pos.y - mouseWorldPos.y;
			float currentDistanceSq = dx * dx + dy * dy;
			if (currentDistanceSq < minDistanceSq) {
				minDistanceSq = currentDistanceSq;
				closestIndex = i;
			}
		}

		if (minDistanceSq < selectionThresholdSq && !myParam.pParticles.empty()) {
			myParam.rParticles[closestIndex].isSelected = true;
		}

		isFollowing = true;
		panFollowingOffset = { 0 };
		if (myVar.isSelectedTrailsEnabled) {
			myParam.trails.trailDots.clear();
		}
	}

	if (IsKeyPressed(KEY_Z) || centerCamera) {
		panFollowingOffset = { 0 };
		isFollowing = true;
		centerCamera = false;
	}

	if (isFollowing) {
		float sumX = 0.0f;
		float sumY = 0.0f;
		int count = 0;
		for (size_t i = 0; i < myParam.pParticles.size(); i++) {
			if (myParam.rParticles[i].isSelected) {
				sumX += myParam.pParticles[i].pos.x;
				sumY += myParam.pParticles[i].pos.y;
				count++;
			}
		}

		followPosition.x = sumX / count;
		followPosition.y = sumY / count;

		followPosition = Vector2Add(followPosition, panFollowingOffset);

		camera.target = followPosition;
		camera.offset = { GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f };

		if (IsKeyPressed(KEY_F) || count == 0 || myParam.pParticles.size() == 0) {
			isFollowing = false;
			camera.zoom = 1.0f;
			camera.target = { 0.0f };
			camera.offset = { 0.0f };
			panFollowingOffset = { 0 };
		}
	}

}


