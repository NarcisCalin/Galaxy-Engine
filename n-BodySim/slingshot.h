#pragma once


class Slingshot {
public:
	float normalizedX;
	float normalizedY;
	float length;
	Slingshot(float normalizedX, float normalizedY, float length);

	
	static Slingshot planetSlingshot(bool &isDragging, bool &isMouse0Pressed, bool &isMouse2SpacePressed);
};

