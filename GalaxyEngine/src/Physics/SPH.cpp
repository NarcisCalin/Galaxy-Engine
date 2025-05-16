#include "../../include/Physics/SPH.h"

void SPH::computeDensityPressure(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles) {
	updateGrid(pParticles, rParticles);

#pragma omp parallel for schedule(dynamic)
	for (size_t i = 0; i < pParticles.size(); i++) {
		if (!rParticles[i].isSPH) { continue; }

		ParticlePhysics& pi = pParticles[i];
		pi.dens = 0.0f;

		size_t cellIndex = getGridIndex(pi.pos);
		std::vector<size_t> neighborCells = getNeighborCells(cellIndex);

		for (size_t neighborCell : neighborCells) {
			if (grid.find(neighborCell) == grid.end()) { continue; }
			for (size_t j : grid[neighborCell].particleIndices) {
				if (!rParticles[j].isSPH) { continue; }
				ParticlePhysics& pj = pParticles[j];

				Vector2 d = { pj.pos.x - pi.pos.x, pj.pos.y - pi.pos.y };
				float rSq = d.x * d.x + d.y * d.y;
				if (rSq >= radiusSq) { continue; }

				float r = sqrt(rSq);

				float sphMass = pj.sphMass * mass;

				pi.dens += sphMass * smoothingKernel(r, radiusMultiplier);
			}
		}

		float minDensity = 0.1f * restDensity * pi.restPress;
		pi.dens = std::max(pi.dens, minDensity);

		float effectiveRestPress = pi.restPress;
		pi.pressF = stiffness * pi.stiff * (pow(pi.dens / (restDensity * pi.restPress * effectiveRestPress), 2) - 1.0f);
	}
}

void SPH::computeForces(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles) {

	int N = pParticles.size();
#pragma omp parallel for schedule(dynamic)
	for (int i = 0; i < N; i++) {
		if (!rParticles[i].isSPH) { continue; }
		ParticlePhysics& pi = pParticles[i];

		size_t cellIndex = getGridIndex(pi.pos);
		std::vector<size_t> neighborCells = getNeighborCells(cellIndex);

		for (size_t neighborCell : neighborCells) {
			if (grid.find(neighborCell) == grid.end()) { continue; }
			for (size_t j : grid[neighborCell].particleIndices) {
				if (!rParticles[j].isSPH) { continue; }

				ParticlePhysics& pj = pParticles[j];

				Vector2 d = { pj.pos.x - pi.pos.x, pj.pos.y - pi.pos.y };
				float rSq = d.x * d.x + d.y * d.y;
				if (rSq >= radiusSq) { continue; }

				float r = sqrt(std::max(rSq, 1e-6f));
				Vector2 nr = { d.x / r, d.y / r };

				float mI = pi.sphMass * mass;
				float mJ = pj.sphMass * mass;

				float minDist = 0.3f * radiusMultiplier;
				Vector2 repF = { 0.0f, 0.0f };
				if (r < minDist) {
					float safeRestI = restDensity * pi.restPress;
					float safeRestJ = restDensity * pj.restPress;
					float dfI = pi.dens / safeRestI;
					float dfJ = pj.dens / safeRestJ;
					float densityScaling = 0.5f * (pow(dfI, 2.0f) + pow(dfJ, 2.0f));
					float repStr = 80.0f * stiffness * (1.0f - r / minDist) * densityScaling;
					repF.x = repStr * nr.x * 0.5f * (mI + mJ);
					repF.y = repStr * nr.y * 0.5f * (mI + mJ);
				}

				float dW = spikyKernelDerivative(r, radiusMultiplier);
				float pressI = pi.pressF, Pj = pj.pressF;
				float densSqI = std::max(pi.dens * pi.dens, 1e-6f);
				float densSqJ = std::max(pj.dens * pj.dens, 1e-6f);
				Vector2 pressF;
				pressF.x = -mJ * (pressI / densSqI + Pj / densSqJ) * dW * nr.x;
				pressF.y = -mJ * (pressI / densSqI + Pj / densSqJ) * dW * nr.y;

				float lapW = smoothingKernelLaplacian(r, radiusMultiplier);
				Vector2 viscF;
				viscF.x = viscosity * pj.visc * mJ / std::max(pj.dens, 0.001f) * lapW * (pj.vel.x - pi.vel.x);
				viscF.y = viscosity * pj.visc * mJ / std::max(pj.dens, 0.001f) * lapW * (pj.vel.y - pi.vel.y);

				float cohCoef = cohesionCoefficient * pi.cohesion;
				float cohFactor = smoothingKernelCohesion(r, radiusMultiplier);
				Vector2 cohF;
				cohF.x = cohCoef * mJ * cohFactor * nr.x;
				cohF.y = cohCoef * mJ * cohFactor * nr.y;

				Vector2 Fij;

				Fij.x = repF.x + pressF.x + viscF.x + cohF.x;
				Fij.y = repF.y + pressF.y + viscF.y + cohF.y;

#pragma omp atomic
				pi.acc.x += Fij.x / pi.sphMass;
#pragma omp atomic
				pi.acc.y += Fij.y / pi.sphMass;
#pragma omp atomic
				pj.acc.x -= Fij.x / pj.sphMass;
#pragma omp atomic
				pj.acc.y -= Fij.y / pj.sphMass;
			}
		}
	}
}

void SPH::Integrate(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, float& dt, Vector2& domainSize, bool& sphGround) {
	for (size_t i = 0; i < pParticles.size(); i++) {

		if (!rParticles[i].isSPH) {
			continue;
		}

		pParticles[i].acc.y += 5.0f;

		// Left wall
		if (pParticles[i].pos.x - radiusMultiplier < 0.0f)
		{
			pParticles[i].vel.x *= boundDamping;
			pParticles[i].pos.x = radiusMultiplier;
		}
		// Right wall
		if (pParticles[i].pos.x + radiusMultiplier > domainSize.x)
		{
			pParticles[i].vel.x *= boundDamping;
			pParticles[i].pos.x = (domainSize.x) - radiusMultiplier;
		}
		// Bottom wall
		if (pParticles[i].pos.y - radiusMultiplier < 0.0f)
		{
			pParticles[i].vel.y *= boundDamping;
			pParticles[i].pos.y = radiusMultiplier;
		}
		// Top wall
		if (pParticles[i].pos.y + radiusMultiplier > domainSize.y)
		{
			pParticles[i].vel.y *= boundDamping;
			pParticles[i].pos.y = domainSize.y - radiusMultiplier;
		}
	}
}
