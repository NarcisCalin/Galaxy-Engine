#pragma once
#include "../UX/camera.h"

class Slingshot {
public:
	float normalizedX;
	float normalizedY;
	float length;
	Slingshot(float normalizedX, float normalizedY, float length);

	
	static Slingshot particleSlingshot(bool &isDragging, SceneCamera myCamera);
};

