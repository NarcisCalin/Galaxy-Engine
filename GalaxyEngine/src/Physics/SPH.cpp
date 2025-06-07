#include "Physics/SPH.h"

void SPH::computeViscCohesionForces(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles,
	std::vector<glm::vec2>& sphForce, size_t& N) {

	const float h = radiusMultiplier;
	const float h2 = h * h;

#pragma omp parallel for
	for (size_t i = 0; i < N; ++i) {

		if (!rParticles[i].isSPH) continue;

		auto& pi = pParticles[i];

		size_t cellIndex = getGridIndex(pi.pos);
		auto neighborCells = getNeighborCells(cellIndex);

		for (auto neighCellIdx : neighborCells) {
			auto it = grid.find(neighCellIdx);
			if (it == grid.end()) continue;
			auto& cell = it->second;

			for (auto pjIdx : cell.particleIndices) {

				if (!rParticles[pjIdx].isSPH) continue;

				if (pjIdx == i) continue;
				auto& pj = pParticles[pjIdx];

				glm::vec2 d = { pj.pos.x - pi.pos.x, pj.pos.y - pi.pos.y };
				float   rSq = d.x * d.x + d.y * d.y;
				if (rSq >= h2) continue;

				float r = sqrtf(std::max(rSq, 1e-6f));
				glm::vec2 nr = { d.x / r, d.y / r };

				float mJ = pj.sphMass * mass;

				float lapW = smoothingKernelLaplacian(r, h);
				glm::vec2 viscF = {
					viscosity * pj.visc * mJ / std::max(pj.dens, 0.001f) * lapW * (pj.vel.x - pi.vel.x),
					viscosity * pj.visc * mJ / std::max(pj.dens, 0.001f) * lapW * (pj.vel.y - pi.vel.y)
				};

				float cohCoef = cohesionCoefficient * pi.cohesion;
				float cohFactor = smoothingKernelCohesion(r, h);
				glm::vec2 cohF = { cohCoef * mJ * cohFactor * nr.x,
									cohCoef * mJ * cohFactor * nr.y };

#pragma omp atomic
				sphForce[i].x += viscF.x + cohF.x;
#pragma omp atomic
				sphForce[i].y += viscF.y + cohF.y;
#pragma omp atomic
				sphForce[pjIdx].x -= viscF.x + cohF.x;
#pragma omp atomic
				sphForce[pjIdx].y -= viscF.y + cohF.y;	
			}
		}
	}
}

void SPH::PCISPH(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, float& dt) {

	size_t N = pParticles.size();

	std::vector<glm::vec2> sphForce(N, { 0.0f, 0.0f });

	for (size_t i = 0; i < N; ++i) {
		pParticles[i].press = 0.0f;
		pParticles[i].pressF = { 0.0f, 0.0f };
	}

	computeViscCohesionForces(pParticles, rParticles, sphForce, N);

	float rhoError = 0.0f;
	iter = 0;

	do {

		float maxRhoErr = 0.0f;
		//std::fill(sphForce.begin(), sphForce.end(), glm::vec2{ 0.0f, 0.0f });
		//computeViscCohesionForces(pParticles, rParticles, sphForce, N);
		//sphForce = baseSphForce;

#pragma omp parallel for
		for (size_t i = 0; i < N; ++i) {

			if (!rParticles[i].isSPH) continue;

			auto& p = pParticles[i];
			p.predVel.x = p.vel.x + dt * 1.5f * (sphForce[i].x / p.sphMass);
			p.predVel.y = p.vel.y + dt * 1.5f * (sphForce[i].y / p.sphMass);

			p.predPos = { p.pos.x + p.predVel.x * dt, p.pos.y + p.predVel.y * dt };
		}

		grid.clear();
		for (size_t i = 0; i < N; ++i) {
			size_t idx = getGridIndex(pParticles[i].predPos);
			grid[idx].particleIndices.push_back(i);
		}

#pragma omp parallel for reduction(max:maxRhoErr)
		for (size_t i = 0; i < N; ++i) {

			if (!rParticles[i].isSPH) continue;

			auto& pi = pParticles[i];
			pi.predDens = 0.0f;

			size_t cellIndex = getGridIndex(pi.predPos);
			auto neighborCells = getNeighborCells(cellIndex);

			for (auto neighIdx : neighborCells) {
				auto it = grid.find(neighIdx);
				if (it == grid.end()) continue;
				for (auto pjIdx : it->second.particleIndices) {

					if (!rParticles[pjIdx].isSPH) continue;

					auto& pj = pParticles[pjIdx];
					glm::vec2 dr = { pi.predPos.x - pj.predPos.x,
								   pi.predPos.y - pj.predPos.y };
					float   rr = sqrtf(dr.x * dr.x + dr.y * dr.y);
					if (rr >= radiusMultiplier) continue;
					float mJ = pj.sphMass * mass;
					float rho0 = 0.5f * (pi.restDens + pj.restDens);

					pi.predDens += mJ * smoothingKernel(rr, radiusMultiplier) / rho0;
				}
			}

			float err = pi.predDens - pi.restDens;
			pi.pressTmp = delta * err;

			if (pi.pressTmp < 0.0f)
				pi.pressTmp = 0.0f;

			maxRhoErr = std::max(maxRhoErr, std::abs(err));

			pi.press += pi.pressTmp * pi.stiff * stiffMultiplier;
		}

#pragma omp parallel for
		for (size_t i = 0; i < N; ++i) {

			if (!rParticles[i].isSPH) continue;

			auto& pi = pParticles[i];
			size_t cellIndex = getGridIndex(pi.predPos);
			auto neighborCells = getNeighborCells(cellIndex);

			for (auto neighIdx : neighborCells) {
				auto it = grid.find(neighIdx);
				if (it == grid.end()) continue;
				for (auto pjIdx : it->second.particleIndices) {
					if (pjIdx == i) continue;

					if (!rParticles[pjIdx].isSPH) continue;
					auto& pj = pParticles[pjIdx];
					glm::vec2 dr = { pi.predPos.x - pj.predPos.x,
								   pi.predPos.y - pj.predPos.y };
					float   rr = sqrtf(dr.x * dr.x + dr.y * dr.y);
					if (rr < 1e-5f || rr >= radiusMultiplier) continue;

					float gradW = spikyKernelDerivative(rr, radiusMultiplier);
					glm::vec2 nrm = { dr.x / rr, dr.y / rr };
					float   avgP = 0.5f * (pi.press + pj.press);
					float   avgD = 0.5f * (pi.predDens + pj.predDens);

					float   mag = -(pi.sphMass * mass + pj.sphMass * mass) * avgP / std::max(avgD, 0.01f);

					// Mass ratio mag limiter
					float massRatio = std::max(pi.sphMass, pj.sphMass) / std::min(pi.sphMass, pj.sphMass);
					float scale = std::min(1.0f, 8.0f / massRatio);

					mag *= scale;

					glm::vec2 pF = { mag * gradW * nrm.x,
								   mag * gradW * nrm.y };

#pragma omp atomic
					sphForce[i].x += pF.x;
#pragma omp atomic
					sphForce[i].y += pF.y;
#pragma omp atomic
					sphForce[pjIdx].x -= pF.x;
#pragma omp atomic
					sphForce[pjIdx].y -= pF.y;
				}
			}
		}

		rhoError = maxRhoErr;
		++iter;

	} while (iter < maxIter/* && rhoError > densTolerance*/); // I'm keeping that condition commented because I might need it int the future

#pragma omp parallel for
	for (size_t i = 0; i < N; ++i) {
		auto& p = pParticles[i];

		p.pressF = sphForce[i] / p.sphMass;

		p.acc += p.pressF;
	}
}

void SPH::groundModeBoundary(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, glm::vec2 domainSize) {

#pragma omp parallel for
	for (size_t i = 0; i < pParticles.size(); ++i) {
		if (!rParticles[i].isSPH) continue;
		auto& p = pParticles[i];
		p.acc.y += verticalGravity;

		// Left wall
		if (p.pos.x - radiusMultiplier < 0.0f) {
			p.vel.x *= boundDamping;
			p.pos.x = radiusMultiplier;
		}
		// Right wall
		if (p.pos.x + radiusMultiplier > domainSize.x) {
			p.vel.x *= boundDamping;
			p.pos.x = domainSize.x - radiusMultiplier;
		}
		// Bottom wall
		if (p.pos.y - radiusMultiplier < 0.0f) {
			p.vel.y *= boundDamping;
			p.pos.y = radiusMultiplier;
		}
		// Top wall
		if (p.pos.y + radiusMultiplier > domainSize.y) {
			p.vel.y *= boundDamping;
			p.pos.y = domainSize.y - radiusMultiplier;
		}
	}
}

