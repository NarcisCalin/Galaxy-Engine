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
