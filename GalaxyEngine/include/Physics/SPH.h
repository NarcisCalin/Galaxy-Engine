#pragma once

#include "Particles/particle.h"

#include "parameters.h"

struct UpdateVariables;
struct UpdateVariables;

struct GridCell {
	std::vector<size_t> particleIndices;
};

class SPH {
public:
	float radiusMultiplier = 3.0f;
	float mass = 0.03f;
	float stiffMultiplier = 1.0f;
	float viscosity = 0.3f;
	float cohesionCoefficient = 1.0f;
	const float boundDamping = -0.1f;
	float delta = 19000.0f;
	float verticalGravity = 3.0f;
	float boundaryFriction = 0.8f;

	float densTolerance = 0.08f;

	int maxIter = 1; // I keep only 1 iteration when I don't use the density error condition
	int iter = 0;

	float rhoError = 0.0f;

	float cellSize;
	std::unordered_map<size_t, GridCell> grid;

	SPH() : cellSize(radiusMultiplier) {}

	float smoothingKernel(float dst, float radiusMultiplier) {
		if (dst >= radiusMultiplier) return 0.0f;

		float volume = (PI * pow(radiusMultiplier, 4.0f)) / 6.0f;
		return (radiusMultiplier - dst) * (radiusMultiplier - dst) / volume;
	}

	float spikyKernelDerivative(float dst, float radiusMultiplier) {
		if (dst >= radiusMultiplier) return 0.0f;

		float scale = -45.0f / (PI * pow(radiusMultiplier, 6.0f));
		return scale * pow(radiusMultiplier - dst, 2.0f);
	}

	float smoothingKernelLaplacian(float dst, float radiusMultiplier) {
		if (dst >= radiusMultiplier) return 0.0f;

		float scale = 45.0f / (PI * pow(radiusMultiplier, 6.0f));
		return scale;
	}

	float smoothingKernelCohesion(float r, float h) {
		if (r >= h) return 0.0f;

		float q = r / h;
		return (1.0f - q) * (0.5f - q) * (0.5f - q) * 30.0f / (PI * h * h);
	}

	int cellAmountX = 3840;

	size_t getGridIndex(const glm::vec2& pos) const {
		size_t cellX = static_cast<size_t>(floor(pos.x / cellSize));
		size_t cellY = static_cast<size_t>(floor(pos.y / cellSize));
		return cellX * cellAmountX + cellY;
	}

	std::vector<size_t> getNeighborCells(size_t cellIndex) const {
		std::vector<size_t> neighbors;
		size_t cellX = cellIndex / cellAmountX;
		size_t cellY = cellIndex % cellAmountX;

		for (int i = -1; i <= 1; i++) {
			for (int j = -1; j <= 1; j++) {
				neighbors.push_back((cellX + i) * cellAmountX + (cellY + j));
			}
		}

		return neighbors;
	}

	void updateGrid(const std::vector<ParticlePhysics>& pParticles, const std::vector<ParticleRendering>& rParticles) {
		grid.clear();

		for (size_t i = 0; i < pParticles.size(); i++) {

			if (rParticles[i].isDarkMatter) { continue; }

			size_t cellIndex = getGridIndex(pParticles[i].pos);
			grid[cellIndex].particleIndices.push_back(i);
		}
	}

	// Currently unused
	float computeDelta(const std::vector<glm::vec2>& kernelGradients, float dt, float mass, float restDensity) {
		float beta = (dt * dt * mass * mass) / (restDensity * restDensity);

		glm::vec2 sumGradW = { 0.0f, 0.0f };
		float sumGradW_dot = 0.0f;

		for (const glm::vec2& gradW : kernelGradients) {
			sumGradW.x += gradW.x;
			sumGradW.y += gradW.y;

			sumGradW_dot += gradW.x * gradW.x + gradW.y * gradW.y;
		}

		float sumDot = sumGradW.x * sumGradW.x + sumGradW.y * sumGradW.y;

		float delta = -1.0f / (beta * (-sumDot - sumGradW_dot));

		return delta;
	}

	void computeViscCohesionForces(std::vector<ParticlePhysics>& pParticles,
		std::vector<ParticleRendering>& rParticles, std::vector<glm::vec2>& sphForce, size_t& N);

	void groundModeBoundary(std::vector<ParticlePhysics>& pParticles,
		std::vector<ParticleRendering>& rParticles, glm::vec2 domainSize);

	void PCISPH(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, float& dt);

	void pcisphSolver(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, float& dt, glm::vec2& domainSize, bool& sphGround) {

		updateGrid(pParticles, rParticles);
		PCISPH(pParticles, rParticles, dt);

		if (sphGround) {
			groundModeBoundary(pParticles, rParticles, domainSize);
		}
	}
};