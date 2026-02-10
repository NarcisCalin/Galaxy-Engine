#pragma once

#include "Particles/particle.h"
#include "Particles/particleTrails.h"

class SceneCamera {
public:
	Camera2D camera;
	glm::vec2 mouseWorldPos;
	glm::vec2 followPosition;
	bool isFollowing;

	glm::vec2 delta;

	bool centerCamera;

	bool cameraChangedThisFrame;

	SceneCamera();

	Camera2D cameraLogic(bool& isLoading, bool& isMouseNotHoveringUI);

	void cameraFollowObject(UpdateVariables& myVar, UpdateParameters& myParam);

	void hasCamMoved();

private:
	Color previousColor;
	glm::vec2 panFollowingOffset;
	float selectionThresholdSq = 100.0f;
	float defaultCamZoom = 0.5f;

	Vector2 lastTarget = camera.target;
	Vector2 lastOffset = camera.offset;
	float lastZoom = camera.zoom;
	float lastRotation = camera.rotation;
};

class SceneCamera3D {
public:
	Camera3D cam3D = {
		{200.0f, 200.0f, 0.0f},
		{0.0f, 0.0f, 0.0f},
		{0.0f, 1.0f, 0.0f},
		45.0f,
		CAMERA_PERSPECTIVE
	};

	glm::vec3 offset = { 0.0f, 0.0f, 0.0f };
	glm::vec3 currentSmoothedTarget = { 0.0f, 0.0f, 0.0f };

	glm::vec3 followPosition;
	Vector3 panFollowingOffset;
	float defaultCamDist = 500.0f;

	Vector3 target = { 0.0f, 0.0f, 0.0f };

	Vector3 panOffsetRight = { 0.0f, 0.0f, 0.0f };
	Vector3 panOffsetUp = { 0.0f, 0.0f, 0.0f };

	float distance = defaultCamDist;
	float angleX = 45.0f;
	float angleY = 30.0f;

	bool centerCamera;
	bool isFollowing;

	Camera3D cameraLogic(bool& isLoading, bool& isMouseNotHoveringUI);

	void cameraFollowObject(UpdateVariables& myVar, UpdateParameters& myParam);
};
