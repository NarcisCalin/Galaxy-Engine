#include "brush.h"

Brush::Brush(SceneCamera myCamera, float brushRadius) {
	this->myCamera = myCamera;
	this->brushRadius = brushRadius;
	mouseWorldPos = { 0 };
}

void Brush::brushLogic(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, Vector2 mouseWorldPos) {
	for (int i = 0; i < 140; i++) {
		float angle = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * 3.14159f;
		float distance = sqrt(static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * brushRadius;

		Vector2 randomOffset = {
			cos(angle) * distance,
			sin(angle) * distance
		};

		Vector2 particlePos = Vector2Add(mouseWorldPos, randomOffset);

		pParticles.emplace_back(particlePos, Vector2{ 0, 0 }, 30000000000.0f);
		rParticles.emplace_back(Color{ 128, 128, 128, 100 }, 0.125f, false, true, false, false, true, true);
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

void Brush::particlesAttractor(std::vector<ParticlePhysics>& pParticles, Vector2 mouseWorldPos, double& G, float& softening, float& timeFactor) {

	if (IsKeyDown(KEY_B)) {

		for (size_t i = 0; i < pParticles.size(); i++) {
			attractorPos = mouseWorldPos;
			float dx = pParticles[i].pos.x - attractorPos.x;
			float dy = pParticles[i].pos.y - attractorPos.y;
			float radius = sqrt(dx * dx + dy * dy);
			if (radius < 1.0f) radius = 1.0f;


			float acceleration = static_cast<float>(G * 10000.0f * brushRadius) / (radius * radius);

			attractorForce.x = static_cast<float>(-(dx / radius) * acceleration * pParticles[i].mass);
			attractorForce.y = static_cast<float>(-(dy / radius) * acceleration * pParticles[i].mass);

			if (IsKeyDown(KEY_LEFT_CONTROL)) {
				attractorForce = { -attractorForce.x, -attractorForce.y };
			}

			pParticles[i].velocity.x += attractorForce.x * timeFactor;
			pParticles[i].velocity.y += attractorForce.y * timeFactor;
		}
	}
}

void Brush::particlesSpinner(std::vector<ParticlePhysics>& pParticles, Vector2 mouseWorldPos, float& softening,
	float& timeFactor) {

	if (IsKeyDown(KEY_N)) {
		for (auto& pParticle : pParticles) {

			Vector2 distanceFromBrush = { pParticle.pos.x - mouseWorldPos.x, pParticle.pos.y - mouseWorldPos.y };

			float distance = sqrt(distanceFromBrush.x * distanceFromBrush.x + distanceFromBrush.y * distanceFromBrush.y);

			if (distance < brushRadius) {

				float falloff = distance / brushRadius;

				float inverseDistance = 1.0f / (distance + softening);
				Vector2 radialDirection = { distanceFromBrush.x * inverseDistance, distanceFromBrush.y * inverseDistance };

				Vector2 spinDirection = { -radialDirection.y, radialDirection.x };

				if (IsKeyDown(KEY_LEFT_CONTROL)) {
					spinDirection = { -spinDirection.x, -spinDirection.y };
				}

				pParticle.velocity.x += spinDirection.x * spinForce * falloff * timeFactor;
				pParticle.velocity.y += spinDirection.y * spinForce * falloff * timeFactor;
			}
		}
	}

}

void Brush::particlesGrabber(std::vector<ParticlePhysics>& pParticles, Vector2 mouseWorldPos, float& zoom) {

	Vector2 mouseDelta = GetMouseDelta();
	mouseDelta = Vector2Scale(mouseDelta, -1.0f / zoom);

	if (IsKeyDown(KEY_M)) {
		dragging = true;
		lastMouseVelocity = mouseDelta;

		for (size_t i = 0; i < pParticles.size(); i++) {
			Vector2 distanceFromBrush = {
				pParticles[i].pos.x - mouseWorldPos.x,
				pParticles[i].pos.y - mouseWorldPos.y
			};

			float distance = sqrt(distanceFromBrush.x * distanceFromBrush.x +
				distanceFromBrush.y * distanceFromBrush.y);

			if (distance < brushRadius) {
				pParticles[i].pos.x -= mouseDelta.x;
				pParticles[i].pos.y -= mouseDelta.y;
			}
		}
	}
	else if (dragging) {
		float impulseFactor = 10.0f;

		for (size_t i = 0; i < pParticles.size(); i++) {
			Vector2 distanceFromBrush = {
				pParticles[i].pos.x - mouseWorldPos.x,
				pParticles[i].pos.y - mouseWorldPos.y
			};

			float distance = sqrt(distanceFromBrush.x * distanceFromBrush.x +
				distanceFromBrush.y * distanceFromBrush.y);

			if (distance < brushRadius) {
				pParticles[i].velocity.x -= lastMouseVelocity.x * impulseFactor;
				pParticles[i].velocity.y -= lastMouseVelocity.y * impulseFactor;
			}
		}
		dragging = false;
	}
}
