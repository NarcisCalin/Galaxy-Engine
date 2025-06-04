#include "IO/io.h"

#include "Physics/slingshot.h"

glm::vec2 slingshotPos = { 0, 0 };

Slingshot::Slingshot(glm::vec2 norm, float length) {
	this->norm = norm;
	this->length = length;
}


Slingshot Slingshot::particleSlingshot(bool &isDragging, SceneCamera myCamera) {

	if (IsMouseButtonPressed(1)) {
		isDragging = false;
	}

	glm::vec2 mouseWorldPos = glm::vec2(GetScreenToWorld2D(GetMousePosition(), myCamera.camera).x, GetScreenToWorld2D(GetMousePosition(), myCamera.camera).y);

	if ((IsMouseButtonPressed(0) && !IsKeyDown(KEY_LEFT_CONTROL) && 
		!IsKeyDown(KEY_LEFT_ALT)) || 
		IO::shortcutPress(KEY_ONE) ||
		IO::shortcutPress(KEY_TWO) || 
		IO::shortcutPress(KEY_THREE) ||
		IO::shortcutPress(KEY_J)
	)
	{   
		isDragging = true;
		slingshotPos = mouseWorldPos;
	}
	if (isDragging) {

		glm::vec2 slingshotDist = slingshotPos - mouseWorldPos;

		float slingshotLengthSquared = slingshotDist.x * slingshotDist.x + slingshotDist.y * slingshotDist.y;
		float slingshotLength = sqrt(slingshotLengthSquared);

		if (slingshotLength != 0) {

			glm::vec2 norm = slingshotDist / slingshotLength;
			DrawCircleV({ slingshotPos.x, slingshotPos.y }, 5, BLUE);
			DrawLineV({ mouseWorldPos.x, mouseWorldPos.y }, { slingshotPos.x, slingshotPos.y }, RED);

			Slingshot slingshot(norm, slingshotLength);
			return slingshot;
		}
	}

	return Slingshot({ 0.0f, 0.0f }, 0.0f);
}