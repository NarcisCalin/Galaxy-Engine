#pragma once

#include <vector>
#include "../raylib/raylib.h"
#include "particle.h"
#include "../UX/camera.h"
#include "particleTrails.h"

struct UpdateVariables;
struct UpdateParameters;


class ParticleSelection {
public:

	bool invertParticleSelection = false;
	bool deselectParticles = false;

	bool selectManyClusters = false;
	
	ParticleSelection();

	void clusterSelection(UpdateVariables& myVar, UpdateParameters& myParam);

	void particleSelection(UpdateVariables& myVar, UpdateParameters& myParam);

	void manyClustersSelection(UpdateVariables& myVar, UpdateParameters& myParam);

	void boxSelection(UpdateParameters& myParam);

	void invertSelection(std::vector<ParticleRendering>& rParticles);

	void deselection(std::vector<ParticleRendering>& rParticles);

	void selectedParticlesStoring(UpdateParameters& myParam);

private:
	float selectionThresholdSq = 100.0f;

	Vector2 boxInitialPos = { 0.0f, 0.0f };

	bool isBoxSelecting = false;
	bool isBoxDeselecting = false;
};