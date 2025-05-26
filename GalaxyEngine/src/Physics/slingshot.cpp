#include "../../include/Physics/slingshot.h"
#include "../../include/raylib/raylib.h"
#include "../../include/IO/io.h"
#include <cmath>

Vector2 slingshotPos = { 0, 0 };

Slingshot::Slingshot(float normalizedX, float normalizedY, float length) {
	this->normalizedX = normalizedX;
	this->normalizedY = normalizedY;
	this->length = length;
}



Slingshot Slingshot::particleSlingshot(bool &isDragging, SceneCamera myCamera) {

	if (IsMouseButtonPressed(1)) {
		isDragging = false;
	}

	Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), myCamera.camera);

	if (IsMouseButtonPressed(0) && !IsKeyDown(KEY_LEFT_CONTROL) && 
		!IsKeyDown(KEY_LEFT_ALT) || 
		IO::handleShortcut(KEY_ONE) ||
		IO::handleShortcut(KEY_TWO) || 
		IO::handleShortcut(KEY_THREE) ||
		IO::handleShortcut(KEY_J)
	)
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