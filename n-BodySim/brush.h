#pragma once
#include <vector>
#include "raylib.h"
#include "particle.h"
#include "camera.h"

class Brush {
public:
	Vector2 mouseWorldPos;

	Brush(SceneCamera myCamera, float brushRadius);

	void brushLogic(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, Vector2 mouseWorldPos);

	void brushSize(Vector2 mouseWorldPos);

	void drawBrush(Vector2 mouseWorldPos);

	void particlesAttractor(std::vector<ParticlePhysics>& pParticles, Vector2 mouseWorldPos, double& G, float& softening, float& timeFactor);

	void particlesSpinner(std::vector<ParticlePhysics>& pParticles, Vector2 mouseWorldPos, float& softening,
		float& timeFactor);

	void particlesGrabber(std::vector<ParticlePhysics>& pParticles, Vector2 mouseWorldPos, float& zoom);

private:
	float brushRadius;
	SceneCamera myCamera;

	Vector2 attractorPos = { 0.0f };

	float spinForce = 140.0f;

	Vector2 attractorForce = { 0.0f };

	bool dragging = false;
	Vector2 lastMouseVelocity = { 0.0f, 0.0f };
};