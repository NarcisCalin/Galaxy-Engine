#include "Physics/SPH3D.h"

void SPH3D::flattenParticles(std::vector<ParticlePhysics>& pParticles) {
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

void SPH3D::readFlattenBack(std::vector<ParticlePhysics>& pParticles) {
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

void SPH3D::computeViscCohesionForces(UpdateVariables& myVar, UpdateParameters& myParam, std::vector<glm::vec3>& sphForce, size_t& N) {

	const float h = radiusMultiplier;
	const float h2 = h * h;

#pragma omp parallel for schedule(dynamic, 32)
	for (size_t i = 0; i < N; ++i) {

		if (!myParam.rParticles3D[i].isSPH || myParam.rParticles3D[i].isPinned || myParam.rParticles3D[i].isBeingDrawn) continue;

		auto& pi = myParam.pParticles3D[i];

		std::vector<size_t> neighborIndices = QueryNeighbors3D::queryNeighbors3D(myParam, myVar.hasAVX2, 64, pi.pos);

		for (size_t j : neighborIndices) {
			size_t pjIdx = j;

			if (!myParam.rParticles3D[pjIdx].isSPH || myParam.rParticles3D[pjIdx].isBeingDrawn) continue;

			if (pjIdx == i) continue;

			auto& pj = myParam.pParticles3D[pjIdx];

			glm::vec3 d = {
				pj.pos.x - pi.pos.x,
				pj.pos.y - pi.pos.y,
				pj.pos.z - pi.pos.z
			};

			float rSq = d.x * d.x + d.y * d.y + d.z * d.z;

			if (rSq >= h2) continue;

			float r = sqrtf(std::max(rSq, 1e-6f));

			glm::vec3 nr = { d.x / r, d.y / r, d.z / r };

			float mJ = pj.sphMass * myVar.mass;

			float lapW = smoothingKernelLaplacian(r, h);

			glm::vec3 viscF = {
				myVar.viscosity * pj.visc * mJ / std::max(pj.dens, 0.001f) * lapW * (pj.vel.x - pi.vel.x),
				myVar.viscosity * pj.visc * mJ / std::max(pj.dens, 0.001f) * lapW * (pj.vel.y - pi.vel.y),
				myVar.viscosity * pj.visc * mJ / std::max(pj.dens, 0.001f) * lapW * (pj.vel.z - pi.vel.z)
			};

			float cohCoef = myVar.cohesionCoefficient * pi.cohesion;
			float cohFactor = smoothingKernelCohesion(r, h);

			glm::vec3 cohF = {
				cohCoef * mJ * cohFactor * nr.x,
				cohCoef * mJ * cohFactor * nr.y,
				cohCoef * mJ * cohFactor * nr.z
			};


#pragma omp atomic
			sphForce[i].x += viscF.x + cohF.x;
#pragma omp atomic
			sphForce[i].y += viscF.y + cohF.y;
#pragma omp atomic
			sphForce[i].z += viscF.z + cohF.z;

#pragma omp atomic
			sphForce[pjIdx].x -= viscF.x + cohF.x;
#pragma omp atomic
			sphForce[pjIdx].y -= viscF.y + cohF.y;
#pragma omp atomic
			sphForce[pjIdx].z -= viscF.z + cohF.z;
		}
	}
}

void SPH3D::PCISPH(UpdateVariables& myVar, UpdateParameters& myParam) {

	size_t N = myParam.pParticles3D.size();

	const float predictionCoeff = 0.5f * myVar.timeFactor * myVar.timeFactor;

	std::vector<glm::vec3> sphForce(N, { 0.0f, 0.0f, 0.0f });

	computeViscCohesionForces(myVar, myParam, sphForce, N);

	for (size_t i = 0; i < N; ++i) {
		myParam.pParticles3D[i].press = 0.0f;
		myParam.pParticles3D[i].pressF = { 0.0f, 0.0f, 0.0f };
	}

	size_t iter = 0;

	do {
#pragma omp parallel for schedule(static)
		for (size_t i = 0; i < N; ++i) {

			if (!myParam.rParticles3D[i].isSPH || myParam.rParticles3D[i].isBeingDrawn) continue;

			auto& p = myParam.pParticles3D[i];

			glm::vec3 displacement = (sphForce[i] / p.sphMass) * predictionCoeff;
			p.predPos = p.pos + displacement;
		}

#pragma omp parallel for schedule(dynamic, 16)
		for (size_t i = 0; i < N; ++i) {

			if (!myParam.rParticles3D[i].isSPH || myParam.rParticles3D[i].isBeingDrawn) continue;

			auto& pi = myParam.pParticles3D[i];
			pi.predDens = 0.0f;

			std::vector<size_t> neighborIndices = QueryNeighbors3D::queryNeighbors3D(myParam, myVar.hasAVX2, 64, pi.predPos);

			for (size_t j : neighborIndices) {
				size_t pjIdx = j;

				if (!myParam.rParticles3D[pjIdx].isSPH || myParam.rParticles3D[pjIdx].isBeingDrawn) continue;

				auto& pj = myParam.pParticles3D[pjIdx];

				glm::vec3 dr = pi.predPos - pj.predPos;
				float rrSq = dr.x * dr.x + dr.y * dr.y + dr.z * dr.z;

				if (rrSq >= radiusMultiplier * radiusMultiplier) continue;

				float rr = sqrtf(rrSq);
				float mJ = pj.sphMass * (myVar.mass * 2.0f);
				float rho0 = 0.5f * (pi.restDens + pj.restDens);

				pi.predDens += mJ * smoothingKernel(rr, radiusMultiplier) / rho0;
			}

			float err = pi.predDens - pi.restDens;
			pi.pressTmp = myVar.delta * err;
			if (pi.pressTmp < 0.0f) pi.pressTmp = 0.0f;

			pi.press += pi.pressTmp * pi.stiff * myVar.stiffMultiplier;
		}

#pragma omp parallel for schedule(dynamic, 32)
		for (size_t i = 0; i < N; ++i) {
			if (!myParam.rParticles3D[i].isSPH || myParam.rParticles3D[i].isBeingDrawn) continue;

			auto& pi = myParam.pParticles3D[i];

			std::vector<size_t> neighborIndices = QueryNeighbors3D::queryNeighbors3D(myParam, myVar.hasAVX2, 64, pi.predPos);

			for (size_t j : neighborIndices) {
				size_t pjIdx = j;

				if (pjIdx == i) continue;

				if (!myParam.rParticles3D[pjIdx].isSPH || myParam.rParticles3D[pjIdx].isBeingDrawn) continue;

				auto& pj = myParam.pParticles3D[pjIdx];

				glm::vec3 dr = pi.predPos - pj.predPos;
				float rr = sqrtf(dr.x * dr.x + dr.y * dr.y + dr.z * dr.z);

				if (rr < 1e-5f || rr >= radiusMultiplier) continue;

				float gradW = spikyKernelDerivative(rr, radiusMultiplier);

				glm::vec3 nrm = { dr.x / rr, dr.y / rr, dr.z / rr };

				float avgP = 0.5f * (pi.press + pj.press);
				float avgD = 0.5f * (pi.predDens + pj.predDens);

				float mag = -(pi.sphMass * (myVar.mass * 2.0f) + pj.sphMass * (myVar.mass * 2.0f)) * avgP / std::max(avgD, 0.01f);

				float massRatio = std::max(pi.sphMass, pj.sphMass) / std::min(pi.sphMass, pj.sphMass);
				float scale = std::min(1.0f, 8.0f / massRatio);
				mag *= scale;

				glm::vec3 pF = {
					mag * gradW * nrm.x,
					mag * gradW * nrm.y,
					mag * gradW * nrm.z
				};

#pragma omp atomic
				sphForce[i].x += pF.x;
#pragma omp atomic
				sphForce[i].y += pF.y;
#pragma omp atomic
				sphForce[i].z += pF.z;

#pragma omp atomic
				sphForce[pjIdx].x -= pF.x;
#pragma omp atomic
				sphForce[pjIdx].y -= pF.y;
#pragma omp atomic
				sphForce[pjIdx].z -= pF.z;
			}
		}

		++iter;

	} while (iter < maxIter);

#pragma omp parallel for schedule(static)
	for (size_t i = 0; i < N; ++i) {
		auto& p = myParam.pParticles3D[i];

		p.pressF = sphForce[i] / p.sphMass;

		if (!myParam.rParticles3D[i].isPinned) {
			p.acc += p.pressF;
		}
	}
}

void SPH3D::groundModeBoundary(std::vector<ParticlePhysics3D>& pParticles,
	std::vector<ParticleRendering3D>& rParticles,
	glm::vec3 domainSize, UpdateVariables& myVar) {
#pragma omp parallel for
	for (size_t i = 0; i < pParticles.size(); ++i) {
		if (rParticles[i].isPinned) continue;
		auto& p = pParticles[i];
		p.acc.y -= myVar.verticalGravity;

		// Left wall
		if (p.pos.x - radiusMultiplier < -domainSize.x / 2.0f) {
			p.vel.x *= boundDamping;
			p.vel.y *= boundaryFriction;
			p.vel.z *= boundaryFriction;
			p.pos.x = -domainSize.x / 2.0f + radiusMultiplier;
		}
		// Right wall
		if (p.pos.x + radiusMultiplier > domainSize.x / 2.0f) {
			p.vel.x *= boundDamping;
			p.vel.y *= boundaryFriction;
			p.vel.z *= boundaryFriction;
			p.pos.x = domainSize.x / 2.0f - radiusMultiplier;
		}
		// Bottom wall
		if (p.pos.y - radiusMultiplier < -domainSize.y / 2.0f) {
			p.vel.y *= boundDamping;
			p.vel.x *= boundaryFriction;
			p.vel.z *= boundaryFriction;
			p.pos.y = -domainSize.y / 2.0f + radiusMultiplier;
		}
		// Top wall
		if (p.pos.y + radiusMultiplier > domainSize.y / 2.0f) {
			p.vel.y *= boundDamping;
			p.vel.x *= boundaryFriction;
			p.vel.z *= boundaryFriction;
			p.pos.y = domainSize.y / 2.0f - radiusMultiplier;
		}
		// Front wall
		if (p.pos.z - radiusMultiplier < -domainSize.z / 2.0f) {
			p.vel.z *= boundDamping;
			p.vel.x *= boundaryFriction;
			p.vel.y *= boundaryFriction;
			p.pos.z = -domainSize.z / 2.0f + radiusMultiplier;
		}
		// Back wall
		if (p.pos.z + radiusMultiplier > domainSize.z / 2.0f) {
			p.vel.z *= boundDamping;
			p.vel.x *= boundaryFriction;
			p.vel.y *= boundaryFriction;
			p.pos.z = domainSize.z / 2.0f - radiusMultiplier;
		}
	}
}

