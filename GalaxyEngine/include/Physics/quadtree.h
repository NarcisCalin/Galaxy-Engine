#pragma once

#include "Particles/particle.h"

struct Quadtree {
	glm::vec2 pos;
	float size;
	size_t startIndex;
	size_t endIndex;
	float gridMass;
	glm::vec2 centerOfMass;

	size_t myIdx;

	size_t parent;
	std::vector<size_t> subGrids;

	static std::vector<Quadtree> globalNodes;

	size_t nodeIdx;

	size_t maxLeafParticles = 1;
	float minLeafSize = 1.0f;

	float gridTemp;

	Quadtree(glm::vec2 pos, float size,
		size_t startIndex, size_t endIndex,
		const std::vector<ParticlePhysics>& pParticles, const std::vector<ParticleRendering>& rParticles,
		size_t parent, size_t myIdx);

	Quadtree() = default;

	void subGridMaker(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles);



	static glm::vec2 boundingBoxPos;
	static float boundingBoxSize;

	static size_t boundingBox(const std::vector<ParticlePhysics>& pParticles,
		const std::vector<ParticleRendering>& rParticles);

	void drawQuadtree();

private:
	inline void computeLeafMass(const std::vector<ParticlePhysics>& pParticles) {
		gridMass = 0.0f;
		gridTemp = 0.0f;
		centerOfMass = { 0.0f, 0.0f };

		for (size_t i = startIndex; i < endIndex; ++i) {
			gridMass += pParticles[i].mass;
			gridTemp += pParticles[i].temp;
			centerOfMass += pParticles[i].pos * pParticles[i].mass;
		}

		if (gridMass > 0) {
			centerOfMass /= gridMass;
		}
	}

	inline void computeInternalMass() {
		gridMass = 0.0f;
		gridTemp = 0.0f;
		centerOfMass = { 0.0f, 0.0f };

		for (size_t& idx : subGrids) {

			Quadtree& child = globalNodes[idx];

			gridMass += child.gridMass;
			gridTemp += child.gridTemp;
			centerOfMass += child.centerOfMass * child.gridMass;
		}

		if (gridMass > 0) {
			centerOfMass /= gridMass;
		}
	}
};