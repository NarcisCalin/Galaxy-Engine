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

	if (IsMouseButtonDown(1)) {
		delta = glm::vec2(GetMouseDelta().x, GetMouseDelta().y);
		delta = delta * (-1.0f / camera.zoom);
		camera.target.x = camera.target.x + delta.x;
		camera.target.y = camera.target.y + delta.y;
		panFollowingOffset = panFollowingOffset + delta;

	}

	float wheel = GetMouseWheelMove();

	mouseWorldPos = glm::vec2(GetScreenToWorld2D(GetMousePosition(), camera).x,
		GetScreenToWorld2D(GetMousePosition(), camera).y);

	if (wheel != 0 && !IsKeyDown(KEY_LEFT_CONTROL) && !loadFlag && isMouseNotHoveringUI) {


		if (isFollowing) {
			glm::vec2 screenCenter = { GetScreenWidth() * 0.5f, GetScreenHeight() * 0.5f };

			mouseWorldPos = glm::vec2(GetScreenToWorld2D({ screenCenter.x,   screenCenter.y }, camera).x,
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

		myParam.particleSelection.clusterSelection(myVar, myParam, true);

		isFollowing = true;
		panFollowingOffset = { 0.0f, 0.0f };

		if (myVar.isSelectedTrailsEnabled) {
			myParam.trails.segments.clear();
		}
	}

	if (IsMouseButtonReleased(1) && IsKeyDown(KEY_LEFT_ALT) && !isDragging && myVar.isMouseNotHoveringUI) {

		myParam.particleSelection.particleSelection(myVar, myParam, true);

		isFollowing = true;
		panFollowingOffset = { 0.0f, 0.0f };
		if (myVar.isSelectedTrailsEnabled) {
			myParam.trails.segments.clear();
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

// ---- 3D IMPLEMENTATION ---- //

Camera3D SceneCamera3D::cameraLogic(bool& isLoading, bool& isMouseNotHoveringUI, bool& firstPerson, bool& isShipEnabled) {

	if (isMouseNotHoveringUI && IO::mouseDown(1) && IO::shortcutDown(KEY_LEFT_ALT) && !firstPerson) {
		Vector2 mouseDelta = GetMouseDelta();
		float panSpeed = distance * 0.002f;
		float radNormX = angleX * DEG2RAD;
		float radNormY = angleY * DEG2RAD;
		camNormal.x = cosf(radNormY) * sinf(radNormX);
		camNormal.y = sinf(radNormY);
		camNormal.z = cosf(radNormY) * cosf(radNormX);
		camNormal = glm::normalize(camNormal);
		camRight = glm::cross(worldUp, camNormal);
		camRight = glm::normalize(camRight);
		camUp = glm::cross(camNormal, camRight);
		camUp = glm::normalize(camUp);
		Vector3 moveRight = Vector3Scale({ camRight.x,camRight.y, camRight.z }, -mouseDelta.x * panSpeed);
		Vector3 moveUp = Vector3Scale({ camUp.x, camUp.y, camUp.z }, mouseDelta.y * panSpeed);
		Vector3 totalPanMove = Vector3Add(moveRight, moveUp);
		if (isFollowing) {
			panFollowingOffset = Vector3Add(panFollowingOffset, totalPanMove);
		}
		else {
			target = Vector3Add(target, totalPanMove);
		}
	}

	if (isMouseNotHoveringUI && IO::mouseDown(1) && !IO::shortcutDown(KEY_LEFT_ALT)) {
		Vector2 mouseDelta = GetMouseDelta();
		angleX -= mouseDelta.x * 0.3f;
		angleY += mouseDelta.y * 0.3f;
		if (angleY > 89.0f) angleY = 89.0f;
		if (angleY < -89.0f) angleY = -89.0f;
	}

	if (isMouseNotHoveringUI && !isShipEnabled) {
		Vector3 arrowMove = { 0.0f, 0.0f, 0.0f };

		float radNormX = angleX * DEG2RAD;
		float radNormY = angleY * DEG2RAD;
		camNormal.x = cosf(radNormY) * sinf(radNormX);
		camNormal.y = sinf(radNormY);
		camNormal.z = cosf(radNormY) * cosf(radNormX);
		camNormal = glm::normalize(camNormal);
		camRight = glm::cross(worldUp, camNormal);
		camRight = glm::normalize(camRight);


		if (IsKeyDown(KEY_RIGHT)) {
			Vector3 moveRight = Vector3Scale({ camRight.x, camRight.y, camRight.z }, arrowMoveSpeed);
			arrowMove += moveRight * GetFrameTime();
		}

		if (IsKeyDown(KEY_LEFT)) {
			Vector3 moveLeft = Vector3Scale({ camRight.x, camRight.y, camRight.z }, -arrowMoveSpeed);
			arrowMove += moveLeft * GetFrameTime();
		}

		if (IsKeyDown(KEY_UP)) {
			Vector3 moveForward = Vector3Scale({ camNormal.x, camNormal.y, camNormal.z }, -arrowMoveSpeed);
			arrowMove += moveForward * GetFrameTime();
		}

		if (IsKeyDown(KEY_DOWN)) {
			Vector3 moveBackward = Vector3Scale({ camNormal.x, camNormal.y, camNormal.z }, arrowMoveSpeed);
			arrowMove += moveBackward * GetFrameTime();
		}

		if (arrowMove.x != 0.0f || arrowMove.y != 0.0f || arrowMove.z != 0.0f) {
			if (firstPerson) {
				firstPersonPosition = Vector3Add(firstPersonPosition, arrowMove);
			}
			else {
				if (isFollowing) {
					panFollowingOffset = Vector3Add(panFollowingOffset, arrowMove);
				}
				else {
					target = Vector3Add(target, arrowMove);
				}
			}
		}
	}

	if (isMouseNotHoveringUI &&
		!IO::shortcutDown(KEY_LEFT_SHIFT) &&
		!IO::shortcutDown(KEY_LEFT_CONTROL) &&
		!firstPerson)
	{
		float wheel = GetMouseWheelMove();
		float zoomSpeed = 0.1f;
		distance *= (1.0f - wheel * zoomSpeed);
		if (distance < 1.0f) distance = 1.0f;
		if (distance > 30000.0f) distance = 30000.0f;
	}

	if (!firstPerson) {

		float smoothSpeed = 0.1f;
		currentSmoothedTarget.x += (target.x - currentSmoothedTarget.x) * smoothSpeed;
		currentSmoothedTarget.y += (target.y - currentSmoothedTarget.y) * smoothSpeed;
		currentSmoothedTarget.z += (target.z - currentSmoothedTarget.z) * smoothSpeed;

		float radX = angleX * DEG2RAD;
		float radY = angleY * DEG2RAD;
		Vector3 orbitOffset;
		orbitOffset.x = distance * cosf(radY) * sinf(radX);
		orbitOffset.y = distance * sinf(radY);
		orbitOffset.z = distance * cosf(radY) * cosf(radX);

		cam3D.target = { currentSmoothedTarget.x, currentSmoothedTarget.y, currentSmoothedTarget.z };
		cam3D.position = Vector3Add(cam3D.target, orbitOffset);
	}
	else {

		float radX = angleX * DEG2RAD;
		float radY = angleY * DEG2RAD;

		cam3D.position = firstPersonPosition;

		Vector3 lookDirection;
		lookDirection.x = cosf(radY) * sinf(radX);
		lookDirection.y = sinf(radY);
		lookDirection.z = cosf(radY) * cosf(radX);

		cam3D.target = Vector3Add(cam3D.position, Vector3Scale(lookDirection, -1.0f));
	}

	float radX = angleX * DEG2RAD;
	float radY = angleY * DEG2RAD;
	camNormal.x = cosf(radY) * sinf(radX);
	camNormal.y = sinf(radY);
	camNormal.z = cosf(radY) * cosf(radX);
	camNormal = glm::normalize(camNormal);
	camRight = glm::cross(worldUp, camNormal);
	camRight = glm::normalize(camRight);
	camUp = glm::cross(camNormal, camRight);
	camUp = glm::normalize(camUp);

	cam3D.up = { camUp.x, camUp.y, camUp.z };
	cam3D.fovy = 45.0f;
	cam3D.projection = CAMERA_PERSPECTIVE;
	return cam3D;
}

void SceneCamera3D::cameraFollowObject(UpdateVariables& myVar, UpdateParameters& myParam) {

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

		myParam.particleSelection3D.clusterSelection(myVar, myParam, true);

		isFollowing = true;
		panFollowingOffset = { 0.0f, 0.0f, 0.0f };

		if (myVar.isSelectedTrailsEnabled) {
			myParam.trails.segments3D.clear();
		}
	}

	if (IsMouseButtonReleased(1) && IsKeyDown(KEY_LEFT_ALT) && !isDragging && myVar.isMouseNotHoveringUI) {

		myParam.particleSelection3D.particleSelection(myVar, myParam, true);

		isFollowing = true;
		panFollowingOffset = { 0.0f, 0.0f, 0.0f };
		if (myVar.isSelectedTrailsEnabled) {
			myParam.trails.segments3D.clear();
		}
	}

	if (IO::shortcutPress(KEY_Z) || centerCamera) {
		panFollowingOffset = { 0.0f, 0.0f, 0.0f };
		isFollowing = true;
		centerCamera = false;
	}

	if (IO::shortcutPress(KEY_F)) {
		target = { 0.0f, 0.0f, 0.0f };
		currentSmoothedTarget = { 0.0f, 0.0f, 0.0f };

		distance = defaultCamDist;
		angleX = 0.0f;
		angleY = 0.0f;

		isFollowing = false;
	}

	if (isFollowing) {
		Vector3 sum = { 0.0f, 0.0f, 0.0f };
		float count = 0.0f;

		for (size_t i = 0; i < myParam.pParticles3D.size(); i++) {
			if (myParam.rParticles3D[i].isSelected) {

				sum.x += myParam.pParticles3D[i].pos.x;
				sum.y += myParam.pParticles3D[i].pos.y;
				sum.z += myParam.pParticles3D[i].pos.z;
				count++;
			}
		}

		if (count > 0) {
			target = Vector3Scale(sum, 1.0f / count);

			target = Vector3Add(target, { panFollowingOffset.x, panFollowingOffset.y, panFollowingOffset.z });
		}
		else {
			isFollowing = false;
		}

		if (IO::shortcutPress(KEY_F) || myParam.pParticles3D.empty()) {
			target = { 0.0f, 0.0f, 0.0f };
			currentSmoothedTarget = { 0.0f, 0.0f, 0.0f };

			distance = defaultCamDist;
			angleX = 0.0f;
			angleY = 0.0f;

			isFollowing = false;
		}
	}
}
