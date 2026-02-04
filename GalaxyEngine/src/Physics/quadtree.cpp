#include "Particles/particle.h"

#include "Physics/quadtree.h"

Node::Node(glm::vec2 pos, float size,
	uint32_t startIndex, uint32_t endIndex,
	std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles) {

	this->pos = pos;
	this->size = size;
	this->startIndex = startIndex;
	this->endIndex = endIndex;
	this->gridMass = 0.0f;
	this->centerOfMass = { 0.0f, 0.0f };

	if ((((endIndex - startIndex) <= 16 /*Max Leaf Particles*/ && size <= 2.0f) /*Max Non-Dense Size*/ || (endIndex - startIndex) == 1) ||
		size <= 0.01f /*Min Leaf Size*/) {
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

void Node::subGridMaker(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles) {

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

			Node& newNode = globalNodes[childIndex];
			newNode = Node(
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
	std::vector<ParticleRendering>& rParticles) {

	if (!pParticles.empty()) {
		globalNodes.reserve(pParticles.size() * 4);
	}
	else {
		globalNodes.reserve(4);
	}

	globalNodes.emplace_back();

	globalNodes[0] = Node(
		{ boundingBox.x, boundingBox.y }, boundingBox.z,
		0, pParticles.size(),
		pParticles, rParticles
	);
}

Node3D::Node3D(glm::vec3 pos, float size,
    uint32_t startIndex, uint32_t endIndex,
    std::vector<ParticlePhysics3D>& pParticles, std::vector<ParticleRendering3D>& rParticles) {

    this->pos = pos;
    this->size = size;
    this->startIndex = startIndex;
    this->endIndex = endIndex;
    this->gridMass = 0.0f;
    this->centerOfMass = { 0.0f, 0.0f, 0.0f };

    if ((((endIndex - startIndex) <= 16 /*Max Leaf Particles*/ && size <= 2.0f) /*Max Non-Dense Size*/ || (endIndex - startIndex) == 1) ||
        size <= 0.01f /*Min Leaf Size*/) {
        computeLeafMass3D(pParticles);
    }
    else {
        subGridMaker3D(pParticles, rParticles);
        computeInternalMass3D();
        calculateNextNeighbor3D();
    }
}

template<typename Predicate>
uint32_t dualPartition3D(std::vector<ParticlePhysics3D>& pParticlesVector, std::vector<ParticleRendering3D>& rParticlesVector, uint32_t begin, uint32_t end, Predicate predicate) {
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

void Node3D::subGridMaker3D(std::vector<ParticlePhysics3D>& pParticles, std::vector<ParticleRendering3D>& rParticles) {

    glm::vec3 mid = pos + size * 0.5f;

    uint32_t currentStart = startIndex;
    uint32_t boundaries[9];
    boundaries[0] = currentStart;

    auto isOct0 = [mid](const ParticlePhysics3D& p) {
        return p.pos.x < mid.x && p.pos.y < mid.y && p.pos.z < mid.z;
        };
    boundaries[1] = dualPartition3D(pParticles, rParticles, boundaries[0], endIndex, isOct0);

    auto isOct1 = [mid](const ParticlePhysics3D& p) {
        return p.pos.x >= mid.x && p.pos.y < mid.y && p.pos.z < mid.z;
        };
    boundaries[2] = dualPartition3D(pParticles, rParticles, boundaries[1], endIndex, isOct1);

    auto isOct2 = [mid](const ParticlePhysics3D& p) {
        return p.pos.x < mid.x && p.pos.y >= mid.y && p.pos.z < mid.z;
        };
    boundaries[3] = dualPartition3D(pParticles, rParticles, boundaries[2], endIndex, isOct2);

    auto isOct3 = [mid](const ParticlePhysics3D& p) {
        return p.pos.x >= mid.x && p.pos.y >= mid.y && p.pos.z < mid.z;
        };
    boundaries[4] = dualPartition3D(pParticles, rParticles, boundaries[3], endIndex, isOct3);

    auto isOct4 = [mid](const ParticlePhysics3D& p) {
        return p.pos.x < mid.x && p.pos.y < mid.y && p.pos.z >= mid.z;
        };
    boundaries[5] = dualPartition3D(pParticles, rParticles, boundaries[4], endIndex, isOct4);

    auto isOct5 = [mid](const ParticlePhysics3D& p) {
        return p.pos.x >= mid.x && p.pos.y < mid.y && p.pos.z >= mid.z;
        };
    boundaries[6] = dualPartition3D(pParticles, rParticles, boundaries[5], endIndex, isOct5);

    auto isOct6 = [mid](const ParticlePhysics3D& p) {
        return p.pos.x < mid.x && p.pos.y >= mid.y && p.pos.z >= mid.z;
        };
    boundaries[7] = dualPartition3D(pParticles, rParticles, boundaries[6], endIndex, isOct6);

    boundaries[8] = endIndex;

    for (int q = 0; q < 8; ++q) {
        if (boundaries[q + 1] > boundaries[q]) {

            float offsetX = (q & 1) ? size * 0.5f : 0.0f;
            float offsetY = (q & 2) ? size * 0.5f : 0.0f;
            float offsetZ = (q & 4) ? size * 0.5f : 0.0f;

            glm::vec3 newPos = { pos.x + offsetX, pos.y + offsetY, pos.z + offsetZ };

            uint32_t childIndex = (uint32_t)globalNodes3D.size();
            globalNodes3D.emplace_back();

            globalNodes3D[childIndex] = Node3D(
                newPos, size * 0.5f,
                boundaries[q], boundaries[q + 1],
                pParticles, rParticles
            );

            int x = (q & 1) ? 1 : 0;
            int y = (q & 2) ? 1 : 0;
            int z = (q & 4) ? 1 : 0;

            subGrids[x][y][z] = childIndex;
        }
    }
}

void Node3D::computeInternalMass3D() {
    gridMass = 0.0f;
    gridTemp = 0.0f;
    centerOfMass = { 0.0f, 0.0f, 0.0f };

    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 2; ++j) {
            for (int k = 0; k < 2; ++k) {

                uint32_t idx = subGrids[i][j][k];

                if (idx == UINT32_MAX) continue;

                Node3D& child = globalNodes3D[idx];

                gridMass += child.gridMass;
                gridTemp += child.gridTemp;
                centerOfMass += child.centerOfMass * child.gridMass;
            }
        }
    }

    if (gridMass > 0) {
        centerOfMass /= gridMass;
    }
}

void Node3D::calculateNextNeighbor3D() {
    next = 0;

    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 2; ++j) {
            for (int k = 0; k < 2; ++k) {

                uint32_t idx = subGrids[i][j][k];

                if (idx == UINT32_MAX) continue;

                next++;
                Node3D& child = globalNodes3D[idx];
                next += child.next;
            }
        }
    }
}

void Octree::root(std::vector<ParticlePhysics3D>& pParticles,
    std::vector<ParticleRendering3D>& rParticles) {

    if (!pParticles.empty()) {
        globalNodes3D.clear();
        globalNodes3D.reserve(pParticles.size() * 8);
    }
    else {
        globalNodes3D.clear();
        globalNodes3D.reserve(8);
    }

    globalNodes3D.emplace_back();

    globalNodes3D[0] = Node3D(
        rootPos, rootSize,
        0, (uint32_t)pParticles.size(),
        pParticles, rParticles
    );
}