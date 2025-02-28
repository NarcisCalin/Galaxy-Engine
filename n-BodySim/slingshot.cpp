#include "slingshot.h"
#include "raylib.h"
#include <cmath>

Vector2 slingshotPos = { 0, 0 };

Slingshot::Slingshot(float normalizedX, float normalizedY, float length) {
	this->normalizedX = normalizedX;
	this->normalizedY = normalizedY;
	this->length = length;
}



Slingshot Slingshot::planetSlingshot(bool &isDragging, bool &isMouse0Pressed, bool &isMouse2SpacePressed) {

	//Is left click pressed?
	if (IsMouseButtonPressed(0) && !IsKeyDown(KEY_SPACE)) {
		isMouse0Pressed = true;
	}
	else {
		isMouse0Pressed = false;
	}
	//Is middle mouse button and spacebar pressed?
	if (IsMouseButtonPressed(0) && IsKeyDown(KEY_SPACE)) {
		isMouse2SpacePressed = true;
	}
	else {
		isMouse2SpacePressed = false;
	}


	if (isMouse0Pressed || isMouse2SpacePressed) {
		isDragging = true;
		slingshotPos = GetMousePosition();
	}
	if (isDragging) {
		float slingshotDistX = slingshotPos.x - GetMouseX();
		float slingshotDistY = slingshotPos.y - GetMouseY();
		float slingshotLengthSquared = slingshotDistX * slingshotDistX + slingshotDistY * slingshotDistY;
		float slingshotLength = sqrt(slingshotLengthSquared);

		if (slingshotLength != 0) {
			float normalX = slingshotDistX / slingshotLength;
			float normalY = slingshotDistY / slingshotLength;
			DrawCircle(slingshotPos.x, slingshotPos.y, 5, BLUE);
			DrawLine(GetMouseX(), GetMouseY(), slingshotPos.x, slingshotPos.y, RED);

			Slingshot slingshot(normalX, normalY, slingshotLength);
			return slingshot;
		}
	}

	return Slingshot(0, 0, 0);
}