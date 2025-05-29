#pragma once

#include <iostream>
#include <vector>
#include "../../external/glm/glm/glm.hpp"
#include "../raylib/raylib.h"
#include "../raylib/rlgl.h"
#include "../raylib/raymath.h"
#include "../Particles/particle.h"
#include "../Particles/particleTrails.h"


class SceneCamera {
public:
	Camera2D camera;
	glm::vec2 mouseWorldPos;
	glm::vec2 followPosition;
	bool isFollowing;

	glm::vec2 delta;

	bool centerCamera;

	SceneCamera();

	Camera2D cameraLogic(bool& isLoading, bool& isMouseNotHoveringUI);

	void cameraFollowObject(UpdateVariables& myVar, UpdateParameters& myParam);

private:
	Color previousColor;
	glm::vec2 panFollowingOffset;
	float selectionThresholdSq = 100.0f;
	float defaultCamZoom = 0.5f;
};
