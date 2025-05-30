#pragma once

#include "../../external/glm/glm/glm.hpp"
#include "../raylib/raylib.h"
#include <vector>
#include <iostream>
#include <memory>
#include "../Particles/particle.h"
#include "omp.h"

struct Quadtree {
	glm::vec2 pos;
	float size;
	size_t startIndex;
	size_t endIndex;
	float gridMass;
    glm::vec2 centerOfMass;
	Quadtree* parent;
	std::vector<std::unique_ptr<Quadtree>> subGrids;

    size_t nodeIdx;

    size_t maxLeafParticles = 1;
    float minLeafSize = 1.0f;

	Quadtree(glm::vec2 pos, float size,
        size_t startIndex, size_t endIndex,
		const std::vector<ParticlePhysics>& pParticles, const std::vector<ParticleRendering>& rParticles,
		Quadtree* parent);

	void subGridMaker(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles);



	static glm::vec2 boundingBoxPos;
	static float boundingBoxSize;

	static Quadtree* boundingBox(const std::vector<ParticlePhysics>& pParticles,
		const std::vector<ParticleRendering>& rParticles);

	void drawQuadtree();

private:
    inline void computeLeafMass(const std::vector<ParticlePhysics>& pParticles) {
        gridMass = 0.0f;
        centerOfMass = { 0.0f, 0.0f };

        for (size_t i = startIndex; i < endIndex; ++i) {
            gridMass += pParticles[i].mass;
            centerOfMass += pParticles[i].pos * pParticles[i].mass;
        }

        if (gridMass > 0) {
            centerOfMass /= gridMass;
        }
    }

    inline void computeInternalMass() {
        gridMass = 0.0f;
        centerOfMass = { 0.0f, 0.0f };

        for (std::unique_ptr<Quadtree>& child : subGrids) {
            gridMass += child->gridMass;
            centerOfMass += child->centerOfMass * child->gridMass;
        }

        if (gridMass > 0) {
            centerOfMass /= gridMass;
        }
    }
};