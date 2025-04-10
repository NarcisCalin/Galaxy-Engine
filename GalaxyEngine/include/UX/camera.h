#pragma once
#include <iostream>

#include <vector>
#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"
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

	Camera2D cameraLogic();

	void cameraFollowObject(UpdateVariables& myVar, UpdateParameters& myParam);

private:
	Color previousColor;
	Vector2 panFollowingOffset;
	float selectionThresholdSq = 100.0f;
};
