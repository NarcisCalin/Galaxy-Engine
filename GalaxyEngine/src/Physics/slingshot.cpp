#include "IO/io.h"

#include "Physics/slingshot.h"
#include "parameters.h"

glm::vec2 slingshotPos = { 0.0f, 0.0f };

Slingshot::Slingshot(glm::vec2 norm, float length) {
	this->norm = norm;
	this->length = length;
}


Slingshot Slingshot::particleSlingshot(UpdateVariables& myVar, SceneCamera myCamera) {

	if (IsMouseButtonPressed(1)) {
		myVar.isDragging = false;
	}

	glm::vec2 mouseWorldPos = glm::vec2(GetScreenToWorld2D(GetMousePosition(), myCamera.camera).x, GetScreenToWorld2D(GetMousePosition(), myCamera.camera).y);

	if ((IsMouseButtonPressed(0) && !IO::shortcutDown(KEY_LEFT_CONTROL) && !IO::shortcutDown(KEY_LEFT_ALT) &&
		(myVar.toolSpawnHeavyParticle || myVar.toolSpawnGalaxy || myVar.toolSpawnStar)) || 
		IO::shortcutPress(KEY_ONE) ||
		IO::shortcutPress(KEY_TWO) || 
		IO::shortcutPress(KEY_J)
	) {   
		myVar.isDragging = true;
		slingshotPos = mouseWorldPos;
	}

	if (myVar.isDragging) {

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

glm::vec3 slingshotPos3D = { 0.0f, 0.0f, 0.0f };

Slingshot3D::Slingshot3D(glm::vec3 norm, float length) {
	this->norm = norm;
	this->length = length;
}


Slingshot3D Slingshot3D::particleSlingshot(UpdateVariables& myVar, glm::vec3& brushPos) {

	if (IsMouseButtonPressed(1)) {
		myVar.isDragging = false;
	}

	if ((IsMouseButtonPressed(0) && !IO::shortcutDown(KEY_LEFT_CONTROL) && !IO::shortcutDown(KEY_LEFT_ALT) &&
		(myVar.toolSpawnHeavyParticle || myVar.toolSpawnGalaxy || myVar.toolSpawnStar)) ||
		IO::shortcutPress(KEY_ONE) ||
		IO::shortcutPress(KEY_TWO) ||
		IO::shortcutPress(KEY_J)
		) {
		myVar.isDragging = true;
		slingshotPos3D = brushPos;
	}

	if (myVar.isDragging) {
		glm::vec3 slingshotDist = slingshotPos3D - brushPos;

		float slingshotLengthSquared = slingshotDist.x * slingshotDist.x + slingshotDist.y * slingshotDist.y + slingshotDist.z * slingshotDist.z;
		float slingshotLength = sqrt(slingshotLengthSquared);

		if (slingshotLength != 0) {

			glm::vec3 norm = slingshotDist / slingshotLength;

			DrawSphere({ slingshotPos3D.x, slingshotPos3D.y, slingshotPos3D.z }, 5, BLUE);
			DrawLine3D({ brushPos.x, brushPos.y, brushPos.z }, { slingshotPos3D.x, slingshotPos3D.y, slingshotPos3D.z }, { 255, 20, 20, 200 });

			Slingshot3D slingshot(norm, slingshotLength);

			return slingshot;
		}
	}

	return Slingshot3D({ 0.0f, 0.0f, 0.0f }, 0.0f);
}