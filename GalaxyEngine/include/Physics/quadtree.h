#pragma once
#include "../raylib/raylib.h"
#include <vector>
#include <iostream>
#include <memory>
#include "../Particles/particle.h"
#include "omp.h"

struct Quadtree {
	Vector2 pos;
	float size;
	size_t startIndex;
	size_t endIndex;
	float gridMass;
	Vector2 centerOfMass;
	Quadtree* parent;
	std::vector<std::unique_ptr<Quadtree>> subGrids;

    size_t nodeIdx;

    int maxLeafParticles = 1;
    float minLeafSize = 1.0f;

	Quadtree(float posX, float posY, float size,
        size_t startIndex, size_t endIndex,
		const std::vector<ParticlePhysics>& pParticles, const std::vector<ParticleRendering>& rParticles,
		Quadtree* parent, size_t nodeIdx);

	void subGridMaker(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles);



	static Vector2 boundingBoxPos;
	static float boundingBoxSize;

	static Quadtree* boundingBox(const std::vector<ParticlePhysics>& pParticles,
		const std::vector<ParticleRendering>& rParticles);

	void drawQuadtree();

private:
    inline void computeLeafMass(const std::vector<ParticlePhysics>& pParticles) {
        gridMass = 0.0f;
        centerOfMass = { 0.0f, 0.0f };

        for (int i = startIndex; i < endIndex; ++i) {
            gridMass += pParticles[i].mass;
            centerOfMass.x += pParticles[i].pos.x * pParticles[i].mass;
            centerOfMass.y += pParticles[i].pos.y * pParticles[i].mass;
        }

        if (gridMass > 0) {
            centerOfMass.x /= gridMass;
            centerOfMass.y /= gridMass;
        }
    }

    inline void computeInternalMass() {
        gridMass = 0.0f;
        centerOfMass = { 0.0f, 0.0f };

        for (auto& child : subGrids) {
            gridMass += child->gridMass;
            centerOfMass.x += child->centerOfMass.x * child->gridMass;
            centerOfMass.y += child->centerOfMass.y * child->gridMass;
        }

        if (gridMass > 0) {
            centerOfMass.x /= gridMass;
            centerOfMass.y /= gridMass;
        }
    }
};