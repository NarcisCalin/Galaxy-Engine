#pragma once

#include <iostream>
#include <vector>
#include "../raylib/raylib.h"
#include "../raylib/rlgl.h"
#include "../raylib/raymath.h"
#include "../Particles/particle.h"
#include "../Particles/particleTrails.h"


class SceneCamera {
public:
	Camera2D camera;
	Vector2 mouseWorldPos;
	Vector2 followPosition;
	bool isFollowing;

	Vector2 delta;

	bool centerCamera;

	SceneCamera();

	Camera2D cameraLogic(bool& isLoading, bool& isMouseNotHoveringUI);

	void cameraFollowObject(UpdateVariables& myVar, UpdateParameters& myParam);

private:
	Color previousColor;
	Vector2 panFollowingOffset;
	float selectionThresholdSq = 100.0f;
	float defaultCamZoom = 0.5f;
};
