#pragma once
#include "camera.h"

class Slingshot {
public:
	float normalizedX;
	float normalizedY;
	float length;
	Slingshot(float normalizedX, float normalizedY, float length);

	
	static Slingshot planetSlingshot(bool &isDragging, SceneCamera myCamera);
};

