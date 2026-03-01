#pragma once

#include "Particles/particle.h"

#include "Particles/QueryNeighbors.h"

#include "Physics/quadtree.h"

#include "Physics/constraint.h"

#include "parameters.h"

struct Physics {

	std::vector<ParticleConstraint> particleConstraints;
	std::unordered_map<uint64_t, ParticleConstraint*> constraintMap;

	std::vector<int64_t> idToIndexTable;

	uint64_t makeKey(uint32_t id1, uint32_t id2) {
		return id1 < id2 ? ((uint64_t)id1 << 32) | id2
			: ((uint64_t)id2 << 32) | id1;
	}

	const float globalConstraintDamping = 0.001f;

	const float stiffCorrectionRatio = 0.013333f; // Heuristic. This used to modify the stiffness of a constraint in a more intuitive way. DO NOT CHANGE

	void calculateForceFromGrid(UpdateVariables& myVar);
	void calculateForceFromGridAVX2(UpdateVariables& myVar);

	glm::vec2 calculateForceFromGridOld(std::vector<ParticlePhysics>& pParticles, UpdateVariables& myVar, ParticlePhysics& pParticle);


	std::vector<float> posX;
	std::vector<float> posY;
	std::vector<float> accX;
	std::vector<float> accY;
	std::vector<float> velX;
	std::vector<float> velY;
	std::vector<float> prevVelX;
	std::vector<float> prevVelY;
	std::vector<float> mass;
	std::vector<float> temp;

	void flattenParticles(std::vector<ParticlePhysics>& pParticles);

	void naiveGravity(std::vector<ParticlePhysics>& pParticles, UpdateVariables& myVar);
	void naiveGravityAVX2(std::vector<ParticlePhysics>& pParticles, UpdateVariables& myVar);

	void readFlattenBack(std::vector<ParticlePhysics>& pParticles);

	void temperatureCalculation(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, UpdateVariables& myVar);

	void createConstraints(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, bool& constraintCreateSpecialFlag,
		UpdateVariables& myVar, UpdateParameters& myParam);

	void constraints(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, UpdateVariables& myVar);

	void pausedConstraints(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, UpdateVariables& myVar);

	void mergerSolver(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, UpdateVariables& myVar, UpdateParameters& myParam);

	void spawnCorrection(UpdateParameters& myParam, bool& hasVAX2, const int& iterations);

	void integrateStart(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, UpdateVariables& myVar);

	void integrateEnd(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, UpdateVariables& myVar);

	void pruneParticles(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, UpdateVariables& myVar);

	// ----- Unused. Test code ----- //

	// This was made for learning purposes and it is not going to replace the existing gravity algorithm.
	// To use it, call first initGrid() and then gravityGrid(). Also be sure to process the bounding box before these
	struct GravityCell {
		glm::vec2 pos;
		float size;
		float mass = 0.0f;
		glm::vec2 force = { 0.0f, 0.0f };
		int depth;

		GravityCell() = default;

		GravityCell(glm::vec2 pos, float size, int depth)
			: pos(pos), size(size), depth(depth)
		{
		}

		std::vector<ParticlePhysics*> particles;
	};

	std::vector<GravityCell> cells;

	int maxDepth = 9; // This controls quality. Higher means more accurate. Gravity forces scale with this too, which is not intended
	int gridRes = 0;

	void initGrid(std::vector<ParticlePhysics>& pParticles, glm::vec3& bb) {

		int totalCells = 0;
		for (int depth = 0; depth < maxDepth; depth++) {
			int res = std::pow(2, depth + 1);
			totalCells += res * res;
		}

		cells.clear();
		cells.resize(totalCells);

		int offset = 0;

		for (int depth = 0; depth < maxDepth; depth++) {
			gridRes = std::pow(2, depth + 1);
			float cellSize = bb.z / static_cast<float>(gridRes);

#pragma omp parallel for collapse(2)
			for (int y = 0; y < gridRes; y++) {
				for (int x = 0; x < gridRes; x++) {
					int index = offset + y * gridRes + x;
					cells[index] = GravityCell(
						glm::vec2{ bb.x + (x * cellSize), bb.y + (y * cellSize) },
						cellSize,
						depth
					);
				}
			}

			offset += gridRes * gridRes;
		}
	}

	void gravityGrid(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, UpdateVariables& myVar, glm::vec3& bb);
	// ----- Unused. Test code ----- //
};