#include "../../include/raylib/raylib.h"
#include <vector>
#include <iostream>
#include <algorithm>
#include <limits>
#include "../../include/Physics/quadtree.h"
#include "../../include/Particles/particle.h"
#include "omp.h"


Quadtree::Quadtree(float posX, float posY, float size,
	size_t startIndex, size_t endIndex,
	const std::vector<ParticlePhysics>& pParticles, const std::vector<ParticleRendering>& rParticles,
	Quadtree* parent = nullptr, size_t nodeIdx = 0) {

	this->pos.x = posX;
	this->pos.y = posY;
	this->size = size;
	this->startIndex = startIndex;
	this->endIndex = endIndex;
	this->gridMass = 0;
	this->centerOfMass = { 0,0 };
	this->parent = parent;

	if ((endIndex - startIndex) <= maxLeafParticles || size <= minLeafSize) {
		computeLeafMass(pParticles);
	}
	else {
		subGridMaker(const_cast<std::vector<ParticlePhysics>&>(pParticles), const_cast<std::vector<ParticleRendering>&>(rParticles));
		computeInternalMass();
	}
}

template<typename T, typename U, typename Predicate>
size_t dualPartition(std::vector<T>& pParticlesVector, std::vector<U>& rParticlesVector, size_t begin, size_t end, Predicate predicate) {
	size_t i = begin;
	for (size_t j = begin; j < end; ++j) {
		if (predicate(pParticlesVector[j])) {
			if (i != j) {
				std::swap(pParticlesVector[i], pParticlesVector[j]);
				std::swap(rParticlesVector[i], rParticlesVector[j]);
			}
			++i;
		}
	}
	return i;
}

void Quadtree::subGridMaker(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles) {
	float midX = pos.x + size / 2.0f;
	float midY = pos.y + size / 2.0f;

	size_t originalStart = startIndex;
	size_t originalEnd = endIndex;
	size_t boundaries[5];
	boundaries[0] = originalStart;

	auto isQuad0 = [midX, midY](const ParticlePhysics& pParticle) {
		return (pParticle.pos.x < midX && pParticle.pos.y < midY);
		};
	boundaries[1] = dualPartition(pParticles, rParticles, originalStart, originalEnd, isQuad0);

	auto isQuad1 = [midX, midY](const ParticlePhysics& pParticle) {
		return (pParticle.pos.x >= midX && pParticle.pos.y < midY);
		};
	boundaries[2] = dualPartition(pParticles, rParticles, boundaries[1], originalEnd, isQuad1);

	auto isQuad2 = [midX, midY](const ParticlePhysics& pParticle) {
		return (pParticle.pos.x < midX && pParticle.pos.y >= midY);
		};
	boundaries[3] = dualPartition(pParticles, rParticles, boundaries[2], originalEnd, isQuad2);

	boundaries[4] = originalEnd;

	subGrids.reserve(4);

	for (int q = 0; q < 4; ++q) {
		if (boundaries[q + 1] > boundaries[q]) {
			float newPosX = pos.x + ((q & 1) ? size / 2.0f : 0.0f);
			float newPosY = pos.y + ((q & 2) ? size / 2.0f : 0.0f);
			subGrids.emplace_back(std::make_unique<Quadtree>(
				newPosX, newPosY, size / 2.0f,
				boundaries[q], boundaries[q + 1],
				pParticles, rParticles, this, 0
			));
		}
	}
}

Vector2 Quadtree::boundingBoxPos = { 0.0f, 0.0f };
float Quadtree::boundingBoxSize = 0.0f;
Quadtree* Quadtree::boundingBox(const std::vector<ParticlePhysics>& pParticles,
	const std::vector<ParticleRendering>& rParticles) {

	float min_x = std::numeric_limits<float>::max();
	float min_y = std::numeric_limits<float>::max();
	float max_x = std::numeric_limits<float>::lowest();
	float max_y = std::numeric_limits<float>::lowest();

	for (const auto& particle : pParticles) {
		min_x = std::min(min_x, particle.pos.x);
		min_y = std::min(min_y, particle.pos.y);
		max_x = std::max(max_x, particle.pos.x);
		max_y = std::max(max_y, particle.pos.y);
	}

	boundingBoxSize = std::max(max_x - min_x, max_y - min_y);

	float centerX = (min_x + max_x) * 0.5f;
	float centerY = (min_y + max_y) * 0.5f;

	boundingBoxPos.x = centerX - boundingBoxSize / 2.0f;
	boundingBoxPos.y = centerY - boundingBoxSize / 2.0f;

	return new Quadtree(boundingBoxPos.x, boundingBoxPos.y, boundingBoxSize, 0, pParticles.size(),
		pParticles, rParticles, nullptr);
}

void Quadtree::drawQuadtree() {
	DrawRectangleLinesEx({ pos.x, pos.y, size, size }, 1.0f, WHITE);

	if (gridMass > 0) {
		DrawCircleV({ centerOfMass.x, centerOfMass.y }, 2.0f, {180,50,50,128});
	}

	for (auto& child : subGrids) {
		child->drawQuadtree();
	}
}