#pragma once
#include <iostream>

#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"
#include "planet.h"
#include <vector>


class SceneCamera {
public:
	Camera2D camera;

	Vector2 mouseWorldPos;

	SceneCamera();

	Camera2D cameraLogic();

	void cameraFollowObject(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles);

private:
	bool isFollowing;
	Vector2 followPosition;
	Color previousColor;
	Vector2 panFollowingOffset;
};
