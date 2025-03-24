#include "brush.h"

Brush::Brush(SceneCamera myCamera, float brushRadius) {
	this->myCamera = myCamera;
	this->brushRadius = brushRadius;
	mouseWorldPos = { 0 };
}

void Brush::brushLogic(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, Vector2 mouseWorldPos){
	for (int i = 0; i < 70; i++) {
	float angle = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * 3.14159f;
	float distance = sqrt(static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * brushRadius;

	Vector2 randomOffset = {
		cos(angle) * distance,
		sin(angle) * distance
	};

	Vector2 particlePos = Vector2Add(mouseWorldPos, randomOffset);

	pParticles.emplace_back(particlePos, Vector2{ 0, 0 }, 200000000000.0f);
	rParticles.emplace_back(Color{ 128, 128, 128, 100 }, 0.125f, true, false, true);
	}
}

void Brush::brushSize(Vector2 mouseWorldPos){
	float wheel = GetMouseWheelMove();
	if (IsKeyDown(KEY_LEFT_CONTROL) && wheel != 0) {
		float scale = 0.2f * wheel;
		brushRadius = Clamp(expf(logf(brushRadius) + scale), 14.0f, 512.0f);
	}
	DrawCircleLinesV({ mouseWorldPos.x, mouseWorldPos.y }, brushRadius, WHITE);
}
