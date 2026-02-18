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

	glm::vec3 camNormal = { 0.0f, 0.0f, 0.0f };
	glm::vec3 camRight = { 0.0f, 0.0f, 0.0f };
	glm::vec3 camUp = { 0.0f, 0.0f, 0.0f };

	glm::vec3 worldUp = { 0.0f, 1.0f, 0.0f };
	Vector3 firstPersonPosition = { 0.0f, 0.0f, 0.0f };

	float arrowMoveSpeed = 250.0f;

	float distance = defaultCamDist;
	float angleX = 45.0f;
	float angleY = 30.0f;

	bool centerCamera;
	bool isFollowing;

	Camera3D cameraLogic(bool& isLoading, bool& isMouseNotHoveringUI, bool& firstPerson, bool& isShipEnabled);

	void cameraFollowObject(UpdateVariables& myVar, UpdateParameters& myParam);

    glm::vec3 prevOffset = { 0.0f, 0.0f, 0.0f };
    glm::vec3 prevCurrentSmoothedTarget = { 0.0f, 0.0f, 0.0f };
    glm::vec3 prevFollowPosition = { 0.0f, 0.0f, 0.0f };
    Vector3 prevPanFollowingOffset = { 0.0f, 0.0f, 0.0f };
    float prevDefaultCamDist = 500.0f;
    Vector3 prevTarget = { 0.0f, 0.0f, 0.0f };
    Vector3 prevPanOffsetRight = { 0.0f, 0.0f, 0.0f };
    Vector3 prevPanOffsetUp = { 0.0f, 0.0f, 0.0f };
    glm::vec3 prevCamNormal = { 0.0f, 0.0f, 0.0f };
    glm::vec3 prevCamRight = { 0.0f, 0.0f, 0.0f };
    glm::vec3 prevCamUp = { 0.0f, 0.0f, 0.0f };
    Vector3 prevFirstPersonPosition = { 0.0f, 0.0f, 0.0f };

    bool HasCameraChanged() {
        const float epsilon = 0.0001f;

        bool changed = false;

        if (glm::length(offset - prevOffset) > epsilon) changed = true;
        if (glm::length(currentSmoothedTarget - prevCurrentSmoothedTarget) > epsilon) changed = true;
        if (glm::length(followPosition - prevFollowPosition) > epsilon) changed = true;
        if (glm::length(camNormal - prevCamNormal) > epsilon) changed = true;
        if (glm::length(camRight - prevCamRight) > epsilon) changed = true;
        if (glm::length(camUp - prevCamUp) > epsilon) changed = true;

        if (fabsf(panFollowingOffset.x - prevPanFollowingOffset.x) > epsilon ||
            fabsf(panFollowingOffset.y - prevPanFollowingOffset.y) > epsilon ||
            fabsf(panFollowingOffset.z - prevPanFollowingOffset.z) > epsilon) changed = true;

        if (fabsf(target.x - prevTarget.x) > epsilon ||
            fabsf(target.y - prevTarget.y) > epsilon ||
            fabsf(target.z - prevTarget.z) > epsilon) changed = true;

        if (fabsf(panOffsetRight.x - prevPanOffsetRight.x) > epsilon ||
            fabsf(panOffsetRight.y - prevPanOffsetRight.y) > epsilon ||
            fabsf(panOffsetRight.z - prevPanOffsetRight.z) > epsilon) changed = true;

        if (fabsf(panOffsetUp.x - prevPanOffsetUp.x) > epsilon ||
            fabsf(panOffsetUp.y - prevPanOffsetUp.y) > epsilon ||
            fabsf(panOffsetUp.z - prevPanOffsetUp.z) > epsilon) changed = true;

        if (fabsf(firstPersonPosition.x - prevFirstPersonPosition.x) > epsilon ||
            fabsf(firstPersonPosition.y - prevFirstPersonPosition.y) > epsilon ||
            fabsf(firstPersonPosition.z - prevFirstPersonPosition.z) > epsilon) changed = true;

        if (fabsf(defaultCamDist - prevDefaultCamDist) > epsilon) changed = true;

        prevOffset = offset;
        prevCurrentSmoothedTarget = currentSmoothedTarget;
        prevFollowPosition = followPosition;
        prevPanFollowingOffset = panFollowingOffset;
        prevDefaultCamDist = defaultCamDist;
        prevTarget = target;
        prevPanOffsetRight = panOffsetRight;
        prevPanOffsetUp = panOffsetUp;
        prevCamNormal = camNormal;
        prevCamRight = camRight;
        prevCamUp = camUp;
        prevFirstPersonPosition = firstPersonPosition;

        return changed;
    }
};
