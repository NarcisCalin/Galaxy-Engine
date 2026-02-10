#pragma once

#include "Particles/particle.h"

#include "Physics/quadtree.h"

#include "parameters.h"

#include "Particles/QueryNeighbors.h"

#include "Physics/constraint.h"

struct Physics3D {

	std::vector<ParticleConstraint> particleConstraints;
	std::unordered_map<uint64_t, ParticleConstraint*> constraintMap;

	std::vector<int64_t> idToIndexTable;

	uint64_t makeKey(uint32_t id1, uint32_t id2) {
		return id1 < id2 ? ((uint64_t)id1 << 32) | id2
			: ((uint64_t)id2 << 32) | id1;
	}

	const float globalConstraintDamping = 0.001f;

	const float stiffCorrectionRatio = 0.013333f; // Heuristic. This used to modify the stiffness of a constraint in a more intuitive way. DO NOT CHANGE

	std::vector<float> posX;
	std::vector<float> posY;
	std::vector<float> posZ;
	std::vector<float> accX;
	std::vector<float> accY;
	std::vector<float> accZ;
	std::vector<float> velX;
	std::vector<float> velY;
	std::vector<float> velZ;
	std::vector<float> prevVelX;
	std::vector<float> prevVelY;
	std::vector<float> prevVelZ;
	std::vector<float> mass;
	std::vector<float> temp;

	void flattenParticles3D(std::vector<ParticlePhysics3D>& pParticles3D);

	glm::vec3 calculateForceFromGrid3DOld(std::vector<ParticlePhysics3D>& pParticles,
		UpdateVariables& myVar,
		ParticlePhysics3D& pParticle);

	void calculateForceFromGrid3D(UpdateVariables& myVar);

	void naiveGravity3D(std::vector<ParticlePhysics3D>& pParticles3D, UpdateVariables& myVar);

	void readFlattenBack3D(std::vector<ParticlePhysics3D>& pParticles3D);

	void temperatureCalculation(std::vector<ParticlePhysics3D>& pParticles, std::vector<ParticleRendering3D>& rParticles, UpdateVariables& myVar);

	void integrateStart3D(std::vector<ParticlePhysics3D>& pParticles3D, std::vector<ParticleRendering3D>& rParticles3D, UpdateVariables& myVar);

	void pruneParticles(std::vector<ParticlePhysics3D>& pParticles, std::vector<ParticleRendering3D>& rParticles, UpdateVariables& myVar);

	void integrateEnd3D(std::vector<ParticlePhysics3D>& pParticles3D, std::vector<ParticleRendering3D>& rParticles3D, UpdateVariables& myVar);

	void createConstraints(std::vector<ParticlePhysics3D>& pParticles, std::vector<ParticleRendering3D>& rParticles, bool& constraintCreateSpecialFlag,
		UpdateVariables& myVar, UpdateParameters& myParam);

	void constraints(std::vector<ParticlePhysics3D>& pParticles, std::vector<ParticleRendering3D>& rParticles, UpdateVariables& myVar);

	void pausedConstraints(std::vector<ParticlePhysics3D>& pParticles, std::vector<ParticleRendering3D>& rParticles, UpdateVariables& myVar);

	void spawnCorrection(UpdateParameters& myParam, bool& hasAVX2, const int& iterations);
};