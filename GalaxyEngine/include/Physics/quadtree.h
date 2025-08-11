#pragma once

#include "Particles/particle.h"

struct Quadtree {
	glm::vec2 pos;
	float size;
	uint32_t startIndex;
	uint32_t endIndex;
	float gridMass;
	glm::vec2 centerOfMass;

	uint32_t subGrids[2][2] = { { UINT32_MAX, UINT32_MAX }, { UINT32_MAX, UINT32_MAX } };

	static std::vector<Quadtree> globalNodes;

	uint32_t maxLeafParticles = 1;
	float minLeafSize = 1.0f;

	float gridTemp;

	Quadtree(glm::vec2 pos, float size,
		uint32_t startIndex, uint32_t endIndex,
		const std::vector<ParticlePhysics>& pParticles, const std::vector<ParticleRendering>& rParticles);

	Quadtree() = default;

	void subGridMaker(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles);

	static glm::vec2 boundingBoxPos;
	static float boundingBoxSize;

	static void boundingBox(const std::vector<ParticlePhysics>& pParticles,
		const std::vector<ParticleRendering>& rParticles);

	void drawQuadtree();

private:
	inline void computeLeafMass(const std::vector<ParticlePhysics>& pParticles) {
		gridMass = 0.0f;
		gridTemp = 0.0f;
		centerOfMass = { 0.0f, 0.0f };

		for (uint32_t i = startIndex; i < endIndex; ++i) {
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

		for (int i = 0; i < 2; ++i) {
			for (int j = 0; j < 2; ++j) {
				uint32_t idx = subGrids[i][j];

				if (idx == UINT32_MAX) continue;

				Quadtree& child = globalNodes[idx];

				gridMass += child.gridMass;
				gridTemp += child.gridTemp;
				centerOfMass += child.centerOfMass * child.gridMass;

			}
		}

		if (gridMass > 0) {
			centerOfMass /= gridMass;
		}
	}
};