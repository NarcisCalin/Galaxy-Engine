#pragma once
#include "../raylib/raylib.h"
#include "../Particles/particle.h"
#include "../parameters.h"
#include "raymath.h"
#include <vector>
#include <cmath>
#include <unordered_map>
#include <algorithm>

struct UpdateVariables;

struct GridCell {
	std::vector<size_t> particleIndices;
};

class SPH {
public:
	float restDensity = 1.24f;
	float stiffness = 100.0f;
	float radiusMultiplier = 3.0f;
	float radiusSq = radiusMultiplier * radiusMultiplier;
	float mass = 0.1f;
	float viscosity = 15.0f;
	float cohesionCoefficient = 1.0f;
	const float boundDamping = -0.5f;

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

	size_t getGridIndex(const Vector2& pos) const {
		size_t cellX = static_cast<size_t>(floor(pos.x / cellSize));
		size_t cellY = static_cast<size_t>(floor(pos.y / cellSize));
		return cellX * 10000 + cellY;
	}

	std::vector<size_t> getNeighborCells(size_t cellIndex) const {
		std::vector<size_t> neighbors;
		size_t cellX = cellIndex / 10000;
		size_t cellY = cellIndex % 10000;

		for (int i = -1; i <= 1; i++) {
			for (int j = -1; j <= 1; j++) {
				neighbors.push_back((cellX + i) * 10000 + (cellY + j));
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

	void computeDensityPressure(std::vector<ParticlePhysics>& particles, std::vector<ParticleRendering>& rParticles);
	void computeForces(std::vector<ParticlePhysics>& particles, std::vector<ParticleRendering>& rParticles);
	void Integrate(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, float& dt, Vector2& domainSize, bool& sphGround);

	void Solver(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, float& dt, Vector2& domainSize, bool& sphGround) {

		updateGrid(pParticles, rParticles);
		computeDensityPressure(pParticles, rParticles);
		computeForces(pParticles, rParticles);

		if (sphGround) {
			Integrate(pParticles, rParticles, dt, domainSize, sphGround);
		}
	}

};