#pragma once

#include "Particles/particle.h"

#include "Physics/quadtree.h"

#include "parameters.h"

struct Physics3D {

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

	void flattenParticles3D(std::vector<ParticlePhysics3D>& pParticles3D);

	glm::vec3 calculateForceFromGrid3DOld(std::vector<ParticlePhysics3D>& pParticles,
		UpdateVariables& myVar,
		ParticlePhysics3D& pParticle);

	void calculateForceFromGrid3D(UpdateVariables& myVar);

	void naiveGravity3D(std::vector<ParticlePhysics3D>& pParticles3D, UpdateVariables& myVar);

	void readFlattenBack3D(std::vector<ParticlePhysics3D>& pParticles3D);

	void integrateStart3D(std::vector<ParticlePhysics3D>& pParticles3D, std::vector<ParticleRendering3D>& rParticles3D, UpdateVariables& myVar);

	void integrateEnd3D(std::vector<ParticlePhysics3D>& pParticles3D, std::vector<ParticleRendering3D>& rParticles3D, UpdateVariables& myVar);
};