#pragma once
#include "raylib.h"
#include "particle.h"
#include "camera.h"
#include "slingshot.h"
#include "brush.h"
#include <vector>

struct Physics;
struct Quadtree;

struct UpdateVariables;

class ParticlesSpawning {
public:

	float heavyParticleWeightMultiplier = 1.0f;
	int predictPathLength = 500;

	bool enablePathPrediction = false;

	void particlesInitialConditions(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, 
		bool& isDragging, bool& isMouseNotHoveringUI, SceneCamera& myCamera, int& screenHeight, int& screenWidth, Brush& brush, 
		Physics physics, Quadtree& quadtree, UpdateVariables& myVar);

	void predictTrajectory(const std::vector<ParticlePhysics>& actualParticles, SceneCamera& myCamera, Physics physics,
		Quadtree& quadtree, UpdateVariables& myVar, Slingshot& slingshot);

private:
	bool isSpawningAllowed = true;
};