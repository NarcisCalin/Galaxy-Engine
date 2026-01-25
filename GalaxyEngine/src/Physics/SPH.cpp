#include "Physics/SPH.h"

void SPH::flattenParticles(std::vector<ParticlePhysics>& pParticles) {
	size_t particleCount = pParticles.size();

	posX.resize(particleCount);
	posY.resize(particleCount);
	predPosX.resize(particleCount);
	predPosY.resize(particleCount);

	accX.resize(particleCount);
	accY.resize(particleCount);

	velX.resize(particleCount);
	velY.resize(particleCount);
	prevVelX.resize(particleCount);
	prevVelY.resize(particleCount);

	sphMass.resize(particleCount);
	press.resize(particleCount);
	pressFX.resize(particleCount);
	pressFY.resize(particleCount);
	stiff.resize(particleCount);
	visc.resize(particleCount);
	dens.resize(particleCount);
	predDens.resize(particleCount);
	restDens.resize(particleCount);

#pragma omp parallel for schedule(static)
	for (int i = 0; i < static_cast<int>(particleCount); i++)
	{
		const auto& particle = pParticles[i];

		posX[i] = particle.pos.x;
		posY[i] = particle.pos.y;

		predPosX[i] = particle.pos.x;
		predPosY[i] = particle.pos.y;

		accX[i] = particle.acc.x;
		accY[i] = particle.acc.y;

		velX[i] = particle.vel.x;
		velY[i] = particle.vel.y;

		prevVelX[i] = particle.prevVel.x;
		prevVelY[i] = particle.prevVel.y;

		sphMass[i] = particle.sphMass;

		press[i] = 0.0f;
		pressFX[i] = 0.0f;
		pressFY[i] = 0.0f;
		stiff[i] = 0.0f;
		visc[i] = 0.0f;
		dens[i] = 0.0f;
		predDens[i] = 0.0f;
		restDens[i] = 0.0f;
	}
}

void SPH::readFlattenBack(std::vector<ParticlePhysics>& pParticles) {
	size_t particleCount = pParticles.size();

#pragma omp parallel for schedule(static)
	for (int i = 0; i < static_cast<int>(particleCount); i++)
	{
		auto& particle = pParticles[i];

		particle.pos.x = posX[i];
		particle.pos.y = posY[i];

		particle.predPos.x = predPosX[i];
		particle.predPos.y = predPosY[i];

		particle.acc.x = accX[i];
		particle.acc.y = accY[i];

		particle.vel.x = velX[i];
		particle.vel.y = velY[i];

		particle.prevVel.x = prevVelX[i];
		particle.prevVel.y = prevVelY[i];

		particle.sphMass = sphMass[i];

		particle.press = press[i];
		particle.pressF.x = pressFX[i];
		particle.pressF.y = pressFY[i];
		particle.stiff = stiff[i];
		particle.visc = visc[i];
		particle.dens = dens[i];
		particle.predDens = predDens[i];
		particle.restDens = restDens[i];
	}
}

void SPH::computeViscCohesionForces(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles,
	std::vector<glm::vec2>& sphForce, size_t& N) {

	const float h = radiusMultiplier;
	const float h2 = h * h;

#pragma omp parallel for schedule(dynamic, 32)
	for (size_t i = 0; i < N; ++i) {

		if (!rParticles[i].isSPH || rParticles[i].isPinned || rParticles[i].isBeingDrawn) continue;

		auto& pi = pParticles[i];

		std::vector<size_t> neighborIndices = queryNeighbors(pParticles[i].pos, pParticles);

		for (size_t j : neighborIndices) {
			size_t pjIdx = j;

			if (!rParticles[pjIdx].isSPH || rParticles[pjIdx].isBeingDrawn) continue;

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

void SPH::PCISPH(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, float& dt) {

	size_t N = pParticles.size();

	const float predictionCoeff = 0.5f * dt * dt;

	std::vector<glm::vec2> sphForce(N, { 0.0f, 0.0f });

	computeViscCohesionForces(pParticles, rParticles, sphForce, N);

	for (size_t i = 0; i < N; ++i) {
		pParticles[i].press = 0.0f;
		pParticles[i].pressF = { 0.0f, 0.0f };
	}

	//float rhoError = 0.0f;
	iter = 0;

	do {
		//float maxRhoErr = 0.0f;

#pragma omp parallel for schedule(static)
		for (size_t i = 0; i < N; ++i) {
			if (!rParticles[i].isSPH || rParticles[i].isBeingDrawn) continue;

			auto& p = pParticles[i];

			glm::vec2 displacement = (sphForce[i] / p.sphMass) * predictionCoeff;
			p.predPos = p.pos + displacement;
		}

#pragma omp parallel for schedule(dynamic, 16) /*reduction(max:maxRhoErr)*/
		for (size_t i = 0; i < N; ++i) {

			if (!rParticles[i].isSPH || rParticles[i].isBeingDrawn) continue;

			auto& pi = pParticles[i];
			pi.predDens = 0.0f;

			std::vector<size_t> neighborIndices = queryNeighbors(pParticles[i].predPos, pParticles);

			for (size_t j : neighborIndices) {
				size_t pjIdx = j;

				//if (pjIdx == i) continue;
				if (!rParticles[pjIdx].isSPH || rParticles[pjIdx].isBeingDrawn) continue;

				auto& pj = pParticles[pjIdx];
				glm::vec2 dr = pi.predPos - pj.predPos;
				float rrSq = dr.x * dr.x + dr.y * dr.y;

				if (rrSq >= radiusMultiplier * radiusMultiplier) continue;

				float rr = sqrtf(rrSq);
				float mJ = pj.sphMass * mass;
				float rho0 = 0.5f * (pi.restDens + pj.restDens);
				pi.predDens += mJ * smoothingKernel(rr, radiusMultiplier) / rho0;
			}



			float err = pi.predDens - pi.restDens;
			pi.pressTmp = delta * err;
			if (pi.pressTmp < 0.0f) pi.pressTmp = 0.0f;

			//maxRhoErr = std::max(maxRhoErr, std::abs(err));
			pi.press += pi.pressTmp * pi.stiff * stiffMultiplier;
		}

#pragma omp parallel for schedule(dynamic, 32)
		for (size_t i = 0; i < N; ++i) {
			if (!rParticles[i].isSPH || rParticles[i].isBeingDrawn) continue;

			auto& pi = pParticles[i];
			std::vector<size_t> neighborIndices = queryNeighbors(pParticles[i].predPos, pParticles);

			for (size_t j : neighborIndices) {
				size_t pjIdx = j;

				if (pjIdx == i) continue;
				if (!rParticles[pjIdx].isSPH || rParticles[pjIdx].isBeingDrawn) continue;

				auto& pj = pParticles[pjIdx];
				glm::vec2 dr = pi.predPos - pj.predPos;
				float rr = sqrtf(dr.x * dr.x + dr.y * dr.y);

				if (rr < 1e-5f || rr >= radiusMultiplier) continue;

				float gradW = spikyKernelDerivative(rr, radiusMultiplier);
				glm::vec2 nrm = { dr.x / rr, dr.y / rr };
				float avgP = 0.5f * (pi.press + pj.press);
				float avgD = 0.5f * (pi.predDens + pj.predDens);
				float mag = -(pi.sphMass * mass + pj.sphMass * mass) * avgP / std::max(avgD, 0.01f);

				float massRatio = std::max(pi.sphMass, pj.sphMass) / std::min(pi.sphMass, pj.sphMass);
				float scale = std::min(1.0f, 8.0f / massRatio);
				mag *= scale;

				glm::vec2 pF = { mag * gradW * nrm.x, mag * gradW * nrm.y };

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

		//rhoError = maxRhoErr;
		++iter;

	} while (iter < maxIter /*&& rhoError > densTolerance */);

#pragma omp parallel for schedule(static)
	for (size_t i = 0; i < N; ++i) {
		auto& p = pParticles[i];

		p.pressF = sphForce[i] / p.sphMass;

		if (!rParticles[i].isPinned) {
			p.acc += p.pressF;
		}
	}
}

void SPH::groundModeBoundary(std::vector<ParticlePhysics>& pParticles,
	std::vector<ParticleRendering>& rParticles,
	glm::vec2 domainSize) {

#pragma omp parallel for
	for (size_t i = 0; i < pParticles.size(); ++i) {
		if (rParticles[i].isPinned) continue;

		auto& p = pParticles[i];
		p.acc.y += verticalGravity;

		// Left wall
		if (p.pos.x - radiusMultiplier < 0.0f) {
			p.vel.x *= boundDamping;
			p.vel.y *= boundaryFriction;
			p.pos.x = radiusMultiplier;
		}

		// Right wall
		if (p.pos.x + radiusMultiplier > domainSize.x) {
			p.vel.x *= boundDamping;
			p.vel.y *= boundaryFriction;
			p.pos.x = domainSize.x - radiusMultiplier;
		}

		// Bottom wall
		if (p.pos.y - radiusMultiplier < 0.0f) {
			p.vel.y *= boundDamping;
			p.vel.x *= boundaryFriction;
			p.pos.y = radiusMultiplier;
		}

		// Top wall
		if (p.pos.y + radiusMultiplier > domainSize.y) {
			p.vel.y *= boundDamping;
			p.vel.x *= boundaryFriction;
			p.pos.y = domainSize.y - radiusMultiplier;
		}
	}
}

