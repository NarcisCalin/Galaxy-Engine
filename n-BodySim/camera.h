#pragma once
#include <iostream>

#include <vector>
#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"
#include "particle.h"
#include "particleTrails.h"


class SceneCamera {
public:
	Camera2D camera;
	Vector2 mouseWorldPos;
	Vector2 followPosition;
	bool isFollowing;

	bool centerCamera;

	SceneCamera();

	Camera2D cameraLogic();

	void cameraFollowObject(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, bool& isMouseNotHoveringUI,
		bool& isSelectedTrailsEnabled, ParticleTrails& trails);

private:
	Color previousColor;
	Vector2 panFollowingOffset;
	float selectionThresholdSq = 100.0f;
};
