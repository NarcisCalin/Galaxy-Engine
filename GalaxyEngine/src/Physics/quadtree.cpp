#include "Particles/particle.h"

#include "Physics/quadtree.h"

std::vector<Quadtree> Quadtree::globalNodes;

std::vector<Quadtree> Quadtree::globalNodes1;
std::vector<Quadtree> Quadtree::globalNodes2;
std::vector<Quadtree> Quadtree::globalNodes3;
std::vector<Quadtree> Quadtree::globalNodes4;

Quadtree::Quadtree(glm::vec2 pos, float size,
	uint32_t startIndex, uint32_t endIndex,
	std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles) {

	this->pos = pos;
	this->size = size;
	this->startIndex = startIndex;
	this->endIndex = endIndex;
	this->gridMass = 0.0f;
	this->centerOfMass = { 0.0f, 0.0f };

	if (((endIndex - startIndex) <= 16 /*Max Leaf Particles*/ && size <= 2.0f /*Max Non-Dense Size*/) || size <= 0.01f /*Min Leaf Size*/) {
		computeLeafMass(pParticles);
	}
	else {
		subGridMaker(pParticles, rParticles);
		computeInternalMass();

		calculateNextNeighbor();
	}
}

template<typename Predicate>
uint32_t dualPartition(std::vector<ParticlePhysics>& pParticlesVector, std::vector<ParticleRendering>& rParticlesVector, uint32_t begin, uint32_t end, Predicate predicate) {
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

void Quadtree::root(std::vector<ParticlePhysics>& pParticles,
	std::vector<ParticleRendering>& rParticles, glm::vec3& boundingBox) {

	globalNodes.clear();

	if (!pParticles.empty()) {
		globalNodes.reserve(pParticles.size() * 4);
	}
	else {
		globalNodes.reserve(4);
	}

	globalNodes.emplace_back();

	globalNodes[0] = Quadtree(
		{ boundingBox.x, boundingBox.y }, boundingBox.z,
		0, pParticles.size(),
		pParticles, rParticles
	);
}