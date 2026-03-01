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

	if ((((endIndex - startIndex) <= 4 /*Max Leaf Particles*/ && size <= 2.0f) /*Max Non-Dense Size*/ || (endIndex - startIndex) == 1) ||
		size <= 0.000001f /*Min Leaf Size*/) {
		computeLeafMass(pParticles);
	}
	else {
		subGridMaker(pParticles, rParticles);
		computeInternalMass();

		calculateNextNeighbor();
	}
}

void Node::subGridMaker(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles) {

    glm::vec2 mid = pos + size * 0.5f;

    uint32_t boundaries[5];
    boundaries[0] = startIndex;
    boundaries[4] = endIndex;

    auto beginIt = pParticles.begin();

    auto isBottomHalf = [mid](const ParticlePhysics& pParticle) {
        return pParticle.pos.y < mid.y;
        };

    auto midY_Iterator = std::partition_point(
        beginIt + boundaries[0],
        beginIt + boundaries[4],
        isBottomHalf
    );
    boundaries[2] = std::distance(beginIt, midY_Iterator);

    auto isLeftHalf = [mid](const ParticlePhysics& pParticle) {
        return pParticle.pos.x < mid.x;
        };

    auto midX_BottomIterator = std::partition_point(
        beginIt + boundaries[0],
        beginIt + boundaries[2],
        isLeftHalf
    );
    boundaries[1] = std::distance(beginIt, midX_BottomIterator);

    auto midX_TopIterator = std::partition_point(
        beginIt + boundaries[2],
        beginIt + boundaries[4],
        isLeftHalf
    );
    boundaries[3] = std::distance(beginIt, midX_TopIterator);

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

    if ((((endIndex - startIndex) <= 4 /*Max Leaf Particles*/ && size <= 2.0f) /*Max Non-Dense Size*/ || (endIndex - startIndex) == 1) ||
        size <= 0.000001f /*Min Leaf Size*/) {
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

    uint32_t boundaries[9];
    boundaries[0] = startIndex;
    boundaries[8] = endIndex;

    auto beginIt = pParticles.begin();

    auto isBottomHalfZ = [mid](const ParticlePhysics3D& p) {
        return p.pos.z < mid.z;
        };
    auto midZ_It = std::partition_point(
        beginIt + boundaries[0],
        beginIt + boundaries[8],
        isBottomHalfZ
    );
    boundaries[4] = std::distance(beginIt, midZ_It);

    auto isBackHalfY = [mid](const ParticlePhysics3D& p) {
        return p.pos.y < mid.y;
        };

    auto midY_BottomIt = std::partition_point(
        beginIt + boundaries[0],
        beginIt + boundaries[4],
        isBackHalfY
    );
    boundaries[2] = std::distance(beginIt, midY_BottomIt);

    auto midY_TopIt = std::partition_point(
        beginIt + boundaries[4],
        beginIt + boundaries[8],
        isBackHalfY
    );
    boundaries[6] = std::distance(beginIt, midY_TopIt);

    auto isLeftHalfX = [mid](const ParticlePhysics3D& p) {
        return p.pos.x < mid.x;
        };

    auto midX_01_It = std::partition_point(beginIt + boundaries[0], beginIt + boundaries[2], isLeftHalfX);
    boundaries[1] = std::distance(beginIt, midX_01_It);

    auto midX_23_It = std::partition_point(beginIt + boundaries[2], beginIt + boundaries[4], isLeftHalfX);
    boundaries[3] = std::distance(beginIt, midX_23_It);

    auto midX_45_It = std::partition_point(beginIt + boundaries[4], beginIt + boundaries[6], isLeftHalfX);
    boundaries[5] = std::distance(beginIt, midX_45_It);

    auto midX_67_It = std::partition_point(beginIt + boundaries[6], beginIt + boundaries[8], isLeftHalfX);
    boundaries[7] = std::distance(beginIt, midX_67_It);

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