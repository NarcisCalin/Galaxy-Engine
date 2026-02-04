#pragma once

#include "Particles/particle.h"

// --- UNUSED QUADTREE FOR MORTON KEYS VERSION --- //
//struct Node {
//	glm::vec2 pos;
//	float size;
//	float mass;
//	glm::vec2 CoM;
//	uint32_t next;
//	int level;
//
//	Node(glm::vec2 pos, float size, float mass, glm::vec2 CoM, uint32_t next, int level) {
//		this->pos = pos;
//		this->size = size;
//		this->mass = mass;
//		this->CoM = CoM;
//		this->next = next;
//		this->level = level;
//	}
//};
//
//struct TestTree {
//	static std::vector<Node> nodesTest;
//};

struct Node;
extern std::vector<Node> globalNodes;

struct Node {
	glm::vec2 pos;
	glm::vec2 centerOfMass;

	float size;
	float gridMass;
	float gridTemp;

	uint32_t startIndex;
	uint32_t endIndex;
	uint32_t next = 0;

	uint32_t subGrids[2][2] = { { UINT32_MAX, UINT32_MAX }, { UINT32_MAX, UINT32_MAX } };

	Node(glm::vec2 pos, float size,
		uint32_t startIndex, uint32_t endIndex,
		std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles);

	Node() = default;

	void subGridMaker(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles);

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

				Node& child = globalNodes[idx];

				gridMass += child.gridMass;
				gridTemp += child.gridTemp;
				centerOfMass += child.centerOfMass * child.gridMass;

			}
		}

		if (gridMass > 0) {
			centerOfMass /= gridMass;
		}
	}

	inline void calculateNextNeighbor() {

		next = 0;

		for (int i = 0; i < 2; ++i) {
			for (int j = 0; j < 2; ++j) {
				uint32_t idx = subGrids[i][j];

				if (idx == UINT32_MAX) continue;

				next++;

				Node& child = globalNodes[idx];

				next += child.next;
			}
		}
	}
};

struct Quadtree {

	glm::vec3 boundingBox;

	Quadtree(std::vector<ParticlePhysics>& pParticles,
		std::vector<ParticleRendering>& rParticles,
		glm::vec3& boundingBox) {

		this->boundingBox = boundingBox;

		root(pParticles, rParticles);
	}

	void root(std::vector<ParticlePhysics>& pParticles,
		std::vector<ParticleRendering>& rParticles);
};

struct Node3D;
extern std::vector<Node3D> globalNodes3D;

struct Node3D {
	glm::vec3 pos;
	glm::vec3 centerOfMass;

	float size;
	float gridMass;
	float gridTemp;

	uint32_t startIndex;
	uint32_t endIndex;
	uint32_t next = 0;

	uint32_t subGrids[2][2][2] = {
		{ { UINT32_MAX, UINT32_MAX }, { UINT32_MAX, UINT32_MAX } },
		{ { UINT32_MAX, UINT32_MAX }, { UINT32_MAX, UINT32_MAX } }
	};

	Node3D(glm::vec3 pos, float size,
		uint32_t startIndex, uint32_t endIndex,
		std::vector<ParticlePhysics3D>& pParticles, std::vector<ParticleRendering3D>& rParticles);

	Node3D() = default;

	void subGridMaker3D(std::vector<ParticlePhysics3D>& pParticles, std::vector<ParticleRendering3D>& rParticles);

	inline void computeLeafMass3D(const std::vector<ParticlePhysics3D>& pParticles) {
		gridMass = 0.0f;
		gridTemp = 0.0f;
		centerOfMass = { 0.0f, 0.0f, 0.0f };

		for (uint32_t i = startIndex; i < endIndex; ++i) {
			gridMass += pParticles[i].mass;
			gridTemp += pParticles[i].temp;

			centerOfMass += pParticles[i].pos * pParticles[i].mass;
		}

		if (gridMass > 0) {
			centerOfMass /= gridMass;
		}
	}

	inline void computeInternalMass3D();

	inline void calculateNextNeighbor3D();
};

struct Octree {

	glm::vec3 rootPos;
	float rootSize;

	Octree(std::vector<ParticlePhysics3D>& pParticles,
		std::vector<ParticleRendering3D>& rParticles,
		glm::vec3 rootPos, float rootSize) {

		this->rootPos = rootPos;
		this->rootSize = rootSize;

		root(pParticles, rParticles);
	}

	void root(std::vector<ParticlePhysics3D>& pParticles,
		std::vector<ParticleRendering3D>& rParticles);
};