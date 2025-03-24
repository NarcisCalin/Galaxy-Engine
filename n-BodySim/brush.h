#pragma once
#include <vector>
#include "raylib.h"
#include "planet.h"
#include "camera.h"

class Brush {
public:
	Vector2 mouseWorldPos;

	Brush(SceneCamera myCamera, float brushRadius);

	void brushLogic(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, Vector2 mouseWorldPos);

	void brushSize(Vector2 mouseWorldPos);

private:
	float brushRadius;
	SceneCamera myCamera;
};