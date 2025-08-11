#include "Particles/particle.h"

#include "Physics/quadtree.h"

std::vector<Quadtree> Quadtree::globalNodes;

Quadtree::Quadtree(glm::vec2 pos, float size,
	uint32_t startIndex, uint32_t endIndex,
	const std::vector<ParticlePhysics>& pParticles, const std::vector<ParticleRendering>& rParticles) {

	this->pos = pos;
	this->size = size;
	this->startIndex = startIndex;
	this->endIndex = endIndex;
	this->gridMass = 0.0f;
	this->centerOfMass = { 0.0f, 0.0f };

	if ((endIndex - startIndex) <= maxLeafParticles || size <= minLeafSize) {
		computeLeafMass(pParticles);
	}
	else {
		subGridMaker(const_cast<std::vector<ParticlePhysics>&>(pParticles), const_cast<std::vector<ParticleRendering>&>(rParticles));
		computeInternalMass();
	}
}

template<typename T, typename U, typename Predicate>
uint32_t dualPartition(std::vector<T>& pParticlesVector, std::vector<U>& rParticlesVector, uint32_t begin, uint32_t end, Predicate predicate) {
	uint32_t i = begin;
	for (uint32_t j = begin; j < end; ++j) {
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

	uint32_t originalStart = startIndex;
	uint32_t originalEnd = endIndex;
	uint32_t boundaries[5];
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

	for (int q = 0; q < 4; ++q) {
		if (boundaries[q + 1] > boundaries[q]) {

			glm::vec2 newPos = { pos.x + ((q & 1) ? size * 0.5f : 0.0f), pos.y + ((q & 2) ? size * 0.5f : 0.0f) };

			uint32_t childIndex = globalNodes.size();
			globalNodes.emplace_back();

			Quadtree& newNode = globalNodes[childIndex];
			newNode = Quadtree(
				newPos, size * 0.5f,
				boundaries[q], boundaries[q + 1],
				pParticles, rParticles
			);

			bool lr = (q & 1) ? 1 : 0;
			bool ud = (q & 2) ? 1 : 0;

			subGrids[lr][ud] = childIndex;
		}
	}
}

glm::vec2 Quadtree::boundingBoxPos = { 0.0f, 0.0f };
float Quadtree::boundingBoxSize = 0.0f;

void Quadtree::boundingBox(const std::vector<ParticlePhysics>& pParticles,
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

	globalNodes.clear();

	if (!pParticles.empty()) {
		globalNodes.reserve(pParticles.size() * 4);
	}
	else {
		globalNodes.reserve(4);
	}

	globalNodes.emplace_back();

	globalNodes[0] = Quadtree(
		boundingBoxPos, boundingBoxSize,
		0, pParticles.size(),
		pParticles, rParticles
	);
}

void Quadtree::drawQuadtree() {
	DrawRectangleLinesEx({ pos.x, pos.y, size, size }, 1.0f, WHITE);

	if (gridMass > 0) {
		DrawCircleV({ centerOfMass.x, centerOfMass.y }, 2.0f, {180,50,50,128});
	}

	for (int i = 0; i < 2; ++i) {
		for (int j = 0; j < 2; ++j) {
			uint32_t idx = subGrids[i][j];

			if (idx == UINT32_MAX) continue;

			globalNodes[idx].drawQuadtree();
		}
	}
}