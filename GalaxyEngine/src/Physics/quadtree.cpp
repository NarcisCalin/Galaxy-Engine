#include "../../include/raylib/raylib.h"
#include <vector>
#include <iostream>
#include <algorithm>
#include <limits>
#include "../../include/Physics/quadtree.h"
#include "../../include/Particles/particle.h"
#include "omp.h"


Quadtree::Quadtree(glm::vec2 pos, float size,
	size_t startIndex, size_t endIndex,
	const std::vector<ParticlePhysics>& pParticles, const std::vector<ParticleRendering>& rParticles,
	Quadtree* parent = nullptr, size_t nodeIdx = 0) {

	this->pos = pos;
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

	glm::vec2 mid = pos + size * 0.5f;

	size_t originalStart = startIndex;
	size_t originalEnd = endIndex;
	size_t boundaries[5];
	boundaries[0] = originalStart;

	auto isQuad0 = [mid](const ParticlePhysics& pParticle) {
		return (pParticle.pos.x < mid.x && pParticle.pos.y < mid.y);
		};
	boundaries[1] = dualPartition(pParticles, rParticles, originalStart, originalEnd, isQuad0);

	auto isQuad1 = [mid](const ParticlePhysics& pParticle) {
		return (pParticle.pos.x >= mid.x && pParticle.pos.y < mid.y);
		};
	boundaries[2] = dualPartition(pParticles, rParticles, boundaries[1], originalEnd, isQuad1);

	auto isQuad2 = [mid](const ParticlePhysics& pParticle) {
		return (pParticle.pos.x < mid.x && pParticle.pos.y >= mid.y);
		};
	boundaries[3] = dualPartition(pParticles, rParticles, boundaries[2], originalEnd, isQuad2);

	boundaries[4] = originalEnd;

	subGrids.reserve(4);

	for (int q = 0; q < 4; ++q) {
		if (boundaries[q + 1] > boundaries[q]) {

			glm::vec2 newPos = { pos.x + ((q & 1) ? size * 0.5f : 0.0f), pos.y + ((q & 2) ? size * 0.5f : 0.0f) };
			subGrids.emplace_back(std::make_unique<Quadtree>(
				newPos, size * 0.5f,
				boundaries[q], boundaries[q + 1],
				pParticles, rParticles, this, 0
			));
		}
	}
}

glm::vec2 Quadtree::boundingBoxPos = { 0.0f, 0.0f };
float Quadtree::boundingBoxSize = 0.0f;
Quadtree* Quadtree::boundingBox(const std::vector<ParticlePhysics>& pParticles,
	const std::vector<ParticleRendering>& rParticles) {

	glm::vec2 min = glm::vec2(std::numeric_limits<float>::max());
	glm::vec2 max = glm::vec2(std::numeric_limits<float>::lowest());

	for (const ParticlePhysics& particle : pParticles) {
		min = glm::min(min, particle.pos);
		max = glm::max(max, particle.pos);
	}

	boundingBoxSize = glm::max(max.x - min.x, max.y - min.y);

	glm::vec2 center = (min + max) * 0.5f;

	boundingBoxPos = center - boundingBoxSize * 0.5f;

	return new Quadtree(boundingBoxPos, boundingBoxSize, 0, pParticles.size(),
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