#pragma once
#include <vector>
#include "raylib.h"
#include "particle.h"
#include "camera.h"
#include "particleTrails.h"

class ParticleSelection {
public:

	bool invertParticleSelection = false;
	bool deselectParticles = false;

	bool selectManyClusters = false;
	
	ParticleSelection();

	void clusterSelection(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, SceneCamera& myCamera,
		bool& isMouseNotHoveringUI, ParticleTrails& trails, bool& isGlobalTrailsEnabled);

	void particleSelection(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, SceneCamera& myCamera,
		bool& isMouseNotHoveringUI, ParticleTrails& trails, bool& isGlobalTrailsEnabled);

	void manyClustersSelection(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, ParticleTrails& trails,
		bool& isGlobalTrailsEnabled);

	void boxSelection(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, SceneCamera& myCamera);

	void invertSelection(std::vector<ParticleRendering>& rParticles);

	void deselection(std::vector<ParticleRendering>& rParticles);

	void selectedParticlesStoring(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, std::vector<ParticleRendering>& rParticlesSelected,
		std::vector<ParticlePhysics>& pParticlesSelected);

private:
	float selectionThresholdSq = 100.0f;

	Vector2 boxInitialPos = { 0.0f };

	bool isBoxSelecting = false;
	bool isBoxDeselecting = false;
};