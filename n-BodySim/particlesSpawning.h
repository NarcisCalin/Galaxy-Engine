#pragma once
#include "raylib.h"
#include "particle.h"
#include "camera.h"
#include "slingshot.h"
#include "brush.h"
#include <vector>

class ParticlesSpawning {
public:
	void particlesInitialConditions(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, 
		bool& isDragging, bool& isMouseNotHoveringUI, SceneCamera& myCamera, int& screenHeight, int& screenWidth, Brush& brush);

private:
	bool isSpawningAllowed = true;
};