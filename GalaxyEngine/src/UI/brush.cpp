#include "../../include/UI/brush.h"
#include "../../include/parameters.h"

Brush::Brush(SceneCamera myCamera, float brushRadius) {
	this->myCamera = myCamera;
	this->brushRadius = brushRadius;
	mouseWorldPos = { 0 };
}

void Brush::brushLogic(UpdateParameters& myParam) {
	for (int i = 0; i < 140; i++) {
		float angle = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * 3.14159f;
		float distance = sqrt(static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * brushRadius;

		Vector2 randomOffset = {
			cos(angle) * distance,
			sin(angle) * distance
		};

		Vector2 particlePos = Vector2Add(myParam.myCamera.mouseWorldPos, randomOffset);

		myParam.pParticles.emplace_back(particlePos, Vector2{ 0, 0 }, 30000000000.0f);
		myParam.rParticles.emplace_back(Color{ 128, 128, 128, 100 }, 0.125f, false, true, false, false, true, true);
	}
}

void Brush::brushSize(Vector2 mouseWorldPos) {
	float wheel = GetMouseWheelMove();
	if (IsKeyDown(KEY_LEFT_CONTROL) && wheel != 0) {
		float scale = 0.2f * wheel;
		brushRadius = Clamp(expf(logf(brushRadius) + scale), 7.0f, 512.0f);
	}
}

void Brush::drawBrush(Vector2 mouseWorldPos) {
	DrawCircleLinesV({ mouseWorldPos.x, mouseWorldPos.y }, brushRadius, WHITE);
}

void Brush::eraseBrush(UpdateVariables& myVar, UpdateParameters& myParam) {

	if (IsKeyDown(KEY_X) && IsMouseButtonDown(2)) {
		for (size_t i = 0; i < myParam.pParticles.size();) {
			Vector2 distanceFromBrush = {
				myParam.pParticles[i].pos.x - myParam.myCamera.mouseWorldPos.x,
				myParam.pParticles[i].pos.y - myParam.myCamera.mouseWorldPos.y
			};

			float distance = sqrt(distanceFromBrush.x * distanceFromBrush.x +
				distanceFromBrush.y * distanceFromBrush.y);

			if (distance < brushRadius) {
				std::swap(myParam.pParticles[i], myParam.pParticles.back());
				std::swap(myParam.rParticles[i], myParam.rParticles.back());

				myParam.pParticles.pop_back();
				myParam.rParticles.pop_back();
			}
			else {
				i++;
			}
		}
	}
}

void Brush::particlesAttractor(UpdateVariables& myVar, UpdateParameters& myParam) {

	if (IsKeyDown(KEY_B)) {

		for (size_t i = 0; i < myParam.pParticles.size(); i++) {
			float dx = myParam.pParticles[i].pos.x - myParam.myCamera.mouseWorldPos.x;
			float dy = myParam.pParticles[i].pos.y - myParam.myCamera.mouseWorldPos.y;
			float radius = sqrt(dx * dx + dy * dy);
			if (radius < 1.0f) radius = 1.0f;


			float acceleration = static_cast<float>(myVar.G * 10000.0f * brushRadius) / (radius * radius);

			attractorForce.x = static_cast<float>(-(dx / radius) * acceleration * myParam.pParticles[i].mass);
			attractorForce.y = static_cast<float>(-(dy / radius) * acceleration * myParam.pParticles[i].mass);

			if (IsKeyDown(KEY_LEFT_CONTROL)) {
				attractorForce = { -attractorForce.x, -attractorForce.y };
			}

			myParam.pParticles[i].velocity.x += attractorForce.x * myVar.timeFactor;
			myParam.pParticles[i].velocity.y += attractorForce.y * myVar.timeFactor;
		}
	}
}

void Brush::particlesSpinner(UpdateVariables& myVar, UpdateParameters& myParam) {

	if (IsKeyDown(KEY_N)) {
		for (auto& pParticle : myParam.pParticles) {

			Vector2 distanceFromBrush = { pParticle.pos.x - myParam.myCamera.mouseWorldPos.x, pParticle.pos.y - myParam.myCamera.mouseWorldPos.y };

			float distance = sqrt(distanceFromBrush.x * distanceFromBrush.x + distanceFromBrush.y * distanceFromBrush.y);

			if (distance < brushRadius) {

				float falloff = distance / brushRadius;

				float inverseDistance = 1.0f / (distance + myVar.softening);
				Vector2 radialDirection = { distanceFromBrush.x * inverseDistance, distanceFromBrush.y * inverseDistance };

				Vector2 spinDirection = { -radialDirection.y, radialDirection.x };

				if (IsKeyDown(KEY_LEFT_CONTROL)) {
					spinDirection = { -spinDirection.x, -spinDirection.y };
				}

				pParticle.velocity.x += spinDirection.x * spinForce * falloff * myVar.timeFactor;
				pParticle.velocity.y += spinDirection.y * spinForce * falloff * myVar.timeFactor;
			}
		}
	}

}

void Brush::particlesGrabber(UpdateVariables& myVar, UpdateParameters& myParam) {

	Vector2 mouseDelta = GetMouseDelta();
	mouseDelta = Vector2Scale(mouseDelta, -1.0f / myParam.myCamera.camera.zoom);

	if (IsKeyDown(KEY_M)) {
		dragging = true;
		lastMouseVelocity = mouseDelta;

		for (size_t i = 0; i < myParam.pParticles.size(); i++) {
			Vector2 distanceFromBrush = {
				myParam.pParticles[i].pos.x - myParam.myCamera.mouseWorldPos.x,
				myParam.pParticles[i].pos.y - myParam.myCamera.mouseWorldPos.y
			};

			float distance = sqrt(distanceFromBrush.x * distanceFromBrush.x +
				distanceFromBrush.y * distanceFromBrush.y);

			if (distance < brushRadius) {
				myParam.pParticles[i].pos.x -= mouseDelta.x;
				myParam.pParticles[i].pos.y -= mouseDelta.y;
			}
		}
	}
	else if (dragging) {
		float impulseFactor = 5.0f;

		for (size_t i = 0; i < myParam.pParticles.size(); i++) {
			Vector2 distanceFromBrush = {
				myParam.pParticles[i].pos.x - myParam.myCamera.mouseWorldPos.x,
				myParam.pParticles[i].pos.y - myParam.myCamera.mouseWorldPos.y
			};

			float distance = sqrt(distanceFromBrush.x * distanceFromBrush.x +
				distanceFromBrush.y * distanceFromBrush.y);

			if (distance < brushRadius) {
				myParam.pParticles[i].velocity.x -= lastMouseVelocity.x * impulseFactor;
				myParam.pParticles[i].velocity.y -= lastMouseVelocity.y * impulseFactor;
			}
		}
		dragging = false;
	}
}
