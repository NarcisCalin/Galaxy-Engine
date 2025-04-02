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
		bool& isMouseNotHoveringUI, ParticleTrails& trails, bool& isGlobalTrailsEnabled);

	void particleSelection(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, SceneCamera& myCamera,
		bool& isMouseNotHoveringUI, ParticleTrails& trails, bool& isGlobalTrailsEnabled);

	void manyClustersSelection(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, ParticleTrails& trails,
		bool& isGlobalTrailsEnabled);

	void selectedParticlesStoring(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, std::vector<ParticleRendering>& rParticlesSelected,
		std::vector<ParticlePhysics>& pParticlesSelected);

private:
	float selectionThresholdSq = 100.0f;
};