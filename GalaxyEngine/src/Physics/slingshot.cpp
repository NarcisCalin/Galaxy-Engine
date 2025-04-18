#include "../../include/Physics/slingshot.h"
#include "../../include/raylib/raylib.h"
#include <cmath>

Vector2 slingshotPos = { 0, 0 };

Slingshot::Slingshot(float normalizedX, float normalizedY, float length) {
	this->normalizedX = normalizedX;
	this->normalizedY = normalizedY;
	this->length = length;
}



Slingshot Slingshot::planetSlingshot(bool &isDragging, SceneCamera myCamera) {

	if (IsMouseButtonPressed(1)) {
		isDragging = false;
	}

	Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), myCamera.camera);

	if (IsMouseButtonPressed(0) && !IsKeyDown(KEY_LEFT_CONTROL) && 
		!IsKeyDown(KEY_LEFT_ALT) || 
		IsKeyPressed(KEY_ONE) ||
		IsKeyPressed(KEY_TWO) || 
		IsKeyPressed(KEY_THREE))
	{   
		isDragging = true;
		slingshotPos = mouseWorldPos;
	}
	if (isDragging) {
		float slingshotDistX = slingshotPos.x - mouseWorldPos.x;
		float slingshotDistY = slingshotPos.y - mouseWorldPos.y;
		float slingshotLengthSquared = slingshotDistX * slingshotDistX + slingshotDistY * slingshotDistY;
		float slingshotLength = sqrt(slingshotLengthSquared);

		if (slingshotLength != 0) {
			float normalX = slingshotDistX / slingshotLength;
			float normalY = slingshotDistY / slingshotLength;
			DrawCircleV({ slingshotPos.x, slingshotPos.y }, 5, BLUE);
			DrawLineV({ mouseWorldPos.x, mouseWorldPos.y }, { slingshotPos.x, slingshotPos.y }, RED);

			Slingshot slingshot(normalX, normalY, slingshotLength);
			return slingshot;
		}
	}

	return Slingshot(0, 0, 0);
}