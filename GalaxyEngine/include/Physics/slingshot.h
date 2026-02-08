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

class Slingshot3D {
public:
	glm::vec3 norm;
	float length;
	Slingshot3D(glm::vec3 norm, float length);


	static Slingshot3D particleSlingshot(UpdateVariables& myVar, glm::vec3& brushPos);
};

