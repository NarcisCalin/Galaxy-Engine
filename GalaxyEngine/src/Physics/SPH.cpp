#include "../../include/Physics/SPH.h"

void SPH::computeDensityPressure(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, const float& pBaseMass) {

	updateGrid(pParticles);

#pragma omp parallel for schedule(dynamic)
	for (size_t i = 0; i < pParticles.size(); i++) {

		if (!rParticles[i].isSPH) {
			continue;
		}

		auto& pi = pParticles[i];
		pi.dens = 0.0f;

		int cellIndex = getGridIndex(pi.predPos);

		std::vector<int> neighborCells = getNeighborCells(cellIndex);

		for (int neighborCell : neighborCells) {
			if (grid.find(neighborCell) == grid.end()) continue;

			for (int j : grid[neighborCell].particleIndices) {

				if (!rParticles[j].isSPH) {
					continue;
				}

				auto& pj = pParticles[j];

				Vector2 d = { pj.predPos.x - pi.predPos.x, pj.predPos.y - pi.predPos.y };
				float r2 = d.x * d.x + d.y * d.y;

				if (r2 >= radiusSq) continue;

				float r = sqrt(r2);

				float sphMass = (pj.mass / pBaseMass) * mass;
				pi.dens += sphMass * smoothingKernel(r, radiusMultiplier);
			}
		}

		pi.dens = fmax(pi.dens, 0.1f * restDensity * pi.restPress);

		pi.pressF = stiffness * (pow(pi.dens / restDensity * pi.restPress, 2) - 1.0f);
	}
}

void SPH::computeForces(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, const float& pBaseMass) {
#pragma omp parallel for schedule(dynamic)
	for (size_t i = 0; i < pParticles.size(); i++) {

		if (!rParticles[i].isSPH) {
			continue;
		}

		auto& pi = pParticles[i];
		Vector2 fpress = { 0.0f, 0.0f };
		Vector2 fvisc = { 0.0f, 0.0f };
		Vector2 fcoh = { 0.0f, 0.0f };

		int cellIndex = getGridIndex(pi.predPos);
		std::vector<int> neighborCells = getNeighborCells(cellIndex);

		for (int neighborCell : neighborCells) {

			if (grid.find(neighborCell) == grid.end()) continue;

			for (int j : grid[neighborCell].particleIndices) {

				if (!rParticles[j].isSPH) {
					continue;
				}

				auto& pj = pParticles[j];

				if (i == j) continue;

				Vector2 d = { pj.predPos.x - pi.predPos.x, pj.predPos.y - pi.predPos.y };
				float r2 = d.x * d.x + d.y * d.y;

				if (r2 >= radiusSq) continue;

				float r = sqrt(r2);

				if (r < 0.001f) {
					r = 0.001f;
					r2 = r * r;
				}

				Vector2 nr = Vector2Normalize(d);

				float repulsionStrength = 0.0f;

				float minDist = 0.9f * radiusMultiplier;

				if (r < minDist) {
					repulsionStrength = 1.0f * stiffness * (1.0f - r / minDist) * (1.0f - r / minDist);
					fpress.x += repulsionStrength * nr.x;
					fpress.y += repulsionStrength * nr.y;
				}

				float dW = smoothingKernelDerivative(r, radiusMultiplier);

				float sphMass = (pj.mass / pBaseMass) * mass;

				float commonP = -sphMass * (pi.pressF / (pi.dens * pi.dens) + pj.pressF / (pj.dens * pj.dens)) * dW;
				fpress.x += commonP * nr.x;
				fpress.y += commonP * nr.y;

				float lapW = smoothingKernelLaplacian(r, radiusMultiplier);
				float commonV = viscosity * sphMass / pj.dens * lapW;
				fvisc.x += commonV * (pj.vel.x - pi.vel.x);
				fvisc.y += commonV * (pj.vel.y - pi.vel.y);

				float cohesionStrength = -cohesionCoefficient * sphMass;
				float cohesionFactor = smoothingKernelCohesion(r, radiusMultiplier);

				fcoh.x -= cohesionStrength * cohesionFactor * nr.x;
				fcoh.y -= cohesionStrength * cohesionFactor * nr.y;

			}
		}

		Vector2 totalForce = (fpress * pi.stiff) + (fvisc * pi.visc) + (fcoh * pi.cohesion);

		float invMass = pBaseMass / pi.mass;
		pi.acc += totalForce * invMass;
	}
}

void SPH::Integrate(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, float& dt, Vector2& domainSize) {
	for (size_t i = 0; i < pParticles.size(); i++) {

		if (!rParticles[i].isSPH) {
			continue;
		}

		pParticles[i].vel += pParticles[i].acc * dt;
		pParticles[i].pos += pParticles[i].vel * dt;

		//if (pParticles[i].pos.x - EPS < 0.0f)
		//{
		//	pParticles[i].vel.x *= BOUND_DAMPING;
		//	pParticles[i].pos.x = EPS;
		//}
		//// right wall
		//if (pParticles[i].pos.x + EPS > domainSize.x - 3000.0f)
		//{
		//	pParticles[i].vel.x *= BOUND_DAMPING;
		//	pParticles[i].pos.x = (domainSize.x - 3000.0f) - EPS;
		//}
		//// bottom wall
		//if (pParticles[i].pos.y - EPS < 0.0f)
		//{
		//	pParticles[i].vel.y *= BOUND_DAMPING;
		//	pParticles[i].pos.y = EPS;
		//}
		//// top wall
		//if (pParticles[i].pos.y + EPS > domainSize.y)
		//{
		//	pParticles[i].vel.y *= BOUND_DAMPING;
		//	pParticles[i].pos.y = domainSize.y - EPS;
		//}
	}
}
