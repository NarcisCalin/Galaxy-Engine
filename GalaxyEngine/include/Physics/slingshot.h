#pragma once

#include "UX/camera.h"

struct UpdateVariables;

class Slingshot {
public:
	glm::vec2 norm;
	float length;
	Slingshot(glm::vec2 norm, float length);

	
	static Slingshot particleSlingshot(UpdateVariables& myVar, SceneCamera myCamera);
};

