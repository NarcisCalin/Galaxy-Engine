#pragma once
#include <vector>
#include "raylib.h"
#include "planet.h"

class ParticleTrails;

struct GroupParams {
	Vector2 pos;
	Color color;

	GroupParams(Vector2 pos, Color color) {
		this->pos = pos;
		this->color = color;
	}
};

class ClusterGrouping {
public:
	ClusterGrouping();
	~ClusterGrouping();


	void clusterManualSelection(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles);

	// THIS FEATURE IS AN UNFINISHED FEATURE
	void allClustersAutomaticSelection(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, Texture2D& particleBlur);

private:
	float selectionThreshold = 8.0f;
	std::vector<ParticlePhysics> groupOfParticles;
	std::vector<std::vector<ParticlePhysics>> groupOfGroups;
	std::vector<GroupParams> groupsParamsVector;
	ParticleTrails* groupTrails;
	std::vector<Vector2> previousSmoothedAverages;
	int clustersMaxTrailLength = 90;
};