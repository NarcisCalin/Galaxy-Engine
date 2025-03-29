#pragma once
#include <vector>
#include "raylib.h"
#include "planet.h"
#include "camera.h"
#include "particleTrails.h"

class ParticleSelection {
public:
	
	ParticleSelection();

	void clusterSelection(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, SceneCamera& myCamera,
		bool& isMouseNotHoveringUI, ParticleTrails& trails);

	void particleSelection(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, SceneCamera& myCamera,
		bool& isMouseNotHoveringUI, ParticleTrails& trails);

private:
	float selectionThresholdSq = 100.0f;
};