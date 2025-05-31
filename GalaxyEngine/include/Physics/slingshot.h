#pragma once

#include "UX/camera.h"

class Slingshot {
public:
	glm::vec2 norm;
	float length;
	Slingshot(glm::vec2 norm, float length);

	
	static Slingshot particleSlingshot(bool &isDragging, SceneCamera myCamera);
};

