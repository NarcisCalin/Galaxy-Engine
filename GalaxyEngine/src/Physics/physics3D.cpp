#include "Physics/physics3D.h"

void Physics3D::flattenParticles3D(std::vector<ParticlePhysics3D>& pParticles3D) {
	size_t particleCount = pParticles3D.size();

	posX.resize(particleCount);
	posY.resize(particleCount);
	posZ.resize(particleCount);
	accX.resize(particleCount);
	accY.resize(particleCount);
	accZ.resize(particleCount);
	velX.resize(particleCount);
	velY.resize(particleCount);
	velZ.resize(particleCount);
	prevVelX.resize(particleCount);
	prevVelY.resize(particleCount);
	prevVelZ.resize(particleCount);
	mass.resize(particleCount);
	temp.resize(particleCount);

#pragma omp parallel for schedule(static)
	for (int i = 0; i < static_cast<int>(particleCount); i++) {

		const auto& particle = pParticles3D[i];

		posX[i] = particle.pos.x;
		posY[i] = particle.pos.y;
		posZ[i] = particle.pos.z;

		accX[i] = particle.acc.x;
		accY[i] = particle.acc.y;
		accZ[i] = particle.acc.z;

		velX[i] = particle.vel.x;
		velY[i] = particle.vel.y;
		velZ[i] = particle.vel.z;

		prevVelX[i] = particle.prevVel.x;
		prevVelY[i] = particle.prevVel.y;
		prevVelZ[i] = particle.prevVel.z;

		mass[i] = particle.mass;

		temp[i] = particle.temp;
	}
}

// This is used in predict trajectory inside particleSpawning.cpp
glm::vec3 Physics3D::calculateForceFromGrid3DOld(std::vector<ParticlePhysics3D>& pParticles,
	UpdateVariables& myVar,
	ParticlePhysics3D& pParticle) {

	glm::vec3 totalForce = { 0.0f, 0.0f, 0.0f };
	uint32_t gridIdx = 0;

	const uint32_t nodeCount = static_cast<uint32_t>(globalNodes3D.size());

	const float thetaSq = myVar.theta * myVar.theta;
	const float softeningSq = myVar.softening * myVar.softening;

	const float Gf = static_cast<float>(myVar.G);

	const float pmass = pParticle.mass;

	auto* particlesPtr = pParticles.data();

	while (gridIdx < nodeCount) {
		Node3D& grid = globalNodes3D[gridIdx];
		float gridMass = grid.gridMass;

		if (gridMass <= 0.0f) {
			gridIdx += grid.next + 1;
			continue;
		}

		const glm::vec3 gridCOM = grid.centerOfMass;
		const float gridSize = grid.size;
		glm::vec3 d = gridCOM - pParticle.pos;

		if (myVar.isPeriodicBoundaryEnabled) {
			d.x -= myVar.domainSize3D.x * ((d.x > myVar.halfDomain3DWidth) - (d.x < -myVar.halfDomain3DWidth));
			d.y -= myVar.domainSize3D.y * ((d.y > myVar.halfDomain3DHeight) - (d.y < -myVar.halfDomain3DHeight));
			d.z -= myVar.domainSize3D.z * ((d.z > myVar.halfDomain3DDepth) - (d.z < -myVar.halfDomain3DDepth));
		}

		float distanceSq = d.x * d.x + d.y * d.y + d.z * d.z + softeningSq;

		bool isSubgridsEmpty = true;
		for (int i = 0; i < 2; ++i) {
			for (int j = 0; j < 2; ++j) {
				for (int k = 0; k < 2; ++k) {
					if (grid.subGrids[i][j][k] != UINT32_MAX) {
						isSubgridsEmpty = false;
						break;
					}
				}
				if (!isSubgridsEmpty) break;
			}
			if (!isSubgridsEmpty) break;
		}

		float gridSizeSq = gridSize * gridSize;

		if ((gridSizeSq < thetaSq * distanceSq) || isSubgridsEmpty) {
			if ((grid.endIndex - grid.startIndex) == 1) {
				const ParticlePhysics3D& other = particlesPtr[grid.startIndex];
				if (std::abs(other.pos.x - pParticle.pos.x) < 0.001f &&
					std::abs(other.pos.y - pParticle.pos.y) < 0.001f &&
					std::abs(other.pos.z - pParticle.pos.z) < 0.001f) {
					gridIdx += grid.next + 1;
					continue;
				}
			}

			float invDistance = 1.0f / std::sqrtf(distanceSq);
			float invDist2 = invDistance * invDistance;
			float invDist3 = invDist2 * invDistance;
			float forceMagnitude = Gf * pmass * gridMass * invDist3;
			totalForce += d * forceMagnitude;

			gridIdx += grid.next + 1;
		}
		else {
			++gridIdx;
		}
	}

	return totalForce;
}

void Physics3D::calculateForceFromGrid3D(UpdateVariables& myVar) {
#pragma omp parallel for schedule(dynamic)
	for (size_t i = 0; i < posX.size(); i++) {

		glm::vec3 totalForce = { 0.0f, 0.0f, 0.0f };

		uint32_t gridIdx = 0;
		const uint32_t nodeCount = static_cast<uint32_t>(globalNodes3D.size());

		const float thetaSq = myVar.theta * myVar.theta;
		const float softeningSq = myVar.softening * myVar.softening;
		const float Gf = myVar.G;
		const float pmass = mass[i];

		while (gridIdx < nodeCount) {
			Node3D& grid = globalNodes3D[gridIdx];

			float gridMass = grid.gridMass;

			if (gridMass <= 0.0f) {
				gridIdx += grid.next + 1;
				continue;
			}

			const glm::vec3 gridCOM = grid.centerOfMass;
			const float gridSize = grid.size;

			glm::vec3 d = gridCOM - glm::vec3{ posX[i], posY[i], posZ[i] };

			if (myVar.isPeriodicBoundaryEnabled) {
				
				d.x -= myVar.domainSize3D.x * ((d.x > myVar.halfDomain3DWidth) - (d.x < -myVar.halfDomain3DWidth));
				d.y -= myVar.domainSize3D.y * ((d.y > myVar.halfDomain3DHeight) - (d.y < -myVar.halfDomain3DHeight));
				d.z -= myVar.domainSize3D.z * ((d.z > myVar.halfDomain3DDepth) - (d.z < -myVar.halfDomain3DDepth));
			}

			float distanceSq = d.x * d.x + d.y * d.y + d.z * d.z + softeningSq;


			bool isSubgridsEmpty = true;
			for (int x = 0; x < 2; ++x) {
				for (int y = 0; y < 2; ++y) {
					for (int z = 0; z < 2; ++z) {
						if (grid.subGrids[x][y][z] != UINT32_MAX) {
							isSubgridsEmpty = false;
							goto skip_check;
						}
					}
				}
			}
		skip_check:;

			float gridSizeSq = gridSize * gridSize;

			if ((gridSizeSq < thetaSq * distanceSq) || isSubgridsEmpty) {

				if (((grid.endIndex - grid.startIndex) <= 16 /*Max Leaf Particles*/ && grid.size <= 2.0f) /*Max Non-Dense Size*/ || (grid.endIndex - grid.startIndex) == 1) {

					if (std::abs(posX[grid.startIndex] - posX[i]) < 0.001f &&
						std::abs(posY[grid.startIndex] - posY[i]) < 0.001f &&
						std::abs(posZ[grid.startIndex] - posZ[i]) < 0.001f) {

						gridIdx += grid.next + 1;
						continue;
					}
				}

				float invDistance = 1.0f / std::sqrtf(distanceSq);
				float invDist2 = invDistance * invDistance;
				float invDist3 = invDist2 * invDistance;

				float forceMagnitude = Gf * pmass * gridMass * invDist3;

				totalForce += d * forceMagnitude;

				if (myVar.isTempEnabled) {
					uint32_t count = grid.endIndex - grid.startIndex;
					if (count > 0) {
						float gridAverageTemp = grid.gridTemp / static_cast<float>(count);
						float temperatureDifference = gridAverageTemp - temp[i];

						float distance = 0.0f;
						if (distanceSq > 1e-16f) distance = 1.0f / invDistance;

						if (distance > 1e-8f) {
							float heatTransfer = myVar.globalHeatConductivity * temperatureDifference / distance;
							temp[i] += heatTransfer * myVar.timeFactor;
						}
					}
				}

				gridIdx += grid.next + 1;
			}
			else {
				++gridIdx;
			}
		}

		accX[i] = totalForce.x / mass[i];
		accY[i] = totalForce.y / mass[i];
		accZ[i] = totalForce.z / mass[i];
	}
}

void Physics3D::naiveGravity3D(std::vector<ParticlePhysics3D>& pParticles3D, UpdateVariables& myVar) {
	int n = static_cast<int>(posX.size());

	__m256 gVec = _mm256_set1_ps(myVar.G);
	__m256 softVec = _mm256_set1_ps(myVar.softening);

	const float* posXPtr = posX.data();
	const float* posYPtr = posY.data();
	const float* posZPtr = posZ.data();
	const float* massPtr = mass.data();

	float* accXPtr = accX.data();
	float* accYPtr = accY.data();
	float* accZPtr = accZ.data();

	__m256 domainSizeX = _mm256_set1_ps(myVar.domainSize3D.x);
	__m256 domainSizeY = _mm256_set1_ps(myVar.domainSize3D.y);
	__m256 domainSizeZ = _mm256_set1_ps(myVar.domainSize3D.z);

	__m256 halfDomainWidth = _mm256_set1_ps(myVar.halfDomain3DWidth);
	__m256 halfDomainHeight = _mm256_set1_ps(myVar.halfDomain3DHeight);
	__m256 halfDomainDepth = _mm256_set1_ps(myVar.halfDomain3DDepth);

	__m256 negHalfDomainWidth = _mm256_set1_ps(-myVar.halfDomain3DWidth);
	__m256 negHalfDomainHeight = _mm256_set1_ps(-myVar.halfDomain3DHeight);
	__m256 negHalfDomainDepth = _mm256_set1_ps(-myVar.halfDomain3DDepth);

	__m256 threeHalfs = _mm256_set1_ps(1.5f);
	__m256 pointFive = _mm256_set1_ps(0.5f);

#pragma omp parallel for schedule(static)
	for (int i = 0; i < n; i++) {
		accXPtr[i] = 0.0f;
		accYPtr[i] = 0.0f;
		accZPtr[i] = 0.0f;
	}

#pragma omp parallel for schedule(dynamic, 64)
	for (int i = 0; i < n; i++) {
		float p_ix = posXPtr[i];
		float p_iy = posYPtr[i];
		float p_iz = posZPtr[i];

		__m256 pxi = _mm256_set1_ps(p_ix);
		__m256 pyi = _mm256_set1_ps(p_iy);
		__m256 pzi = _mm256_set1_ps(p_iz);

		__m256 totalAccX = _mm256_setzero_ps();
		__m256 totalAccY = _mm256_setzero_ps();
		__m256 totalAccZ = _mm256_setzero_ps();

		int j;

		for (j = 0; j <= n - 8; j += 8) {
			__m256 pxj = _mm256_loadu_ps(&posXPtr[j]);
			__m256 pyj = _mm256_loadu_ps(&posYPtr[j]);
			__m256 pzj = _mm256_loadu_ps(&posZPtr[j]);
			__m256 mj = _mm256_loadu_ps(&massPtr[j]);

			__m256 dx = _mm256_sub_ps(pxj, pxi);
			__m256 dy = _mm256_sub_ps(pyj, pyi);
			__m256 dz = _mm256_sub_ps(pzj, pzi);

			if (myVar.isPeriodicBoundaryEnabled) {
				dx = _mm256_sub_ps(dx,
					_mm256_sub_ps(
						_mm256_and_ps(_mm256_cmp_ps(dx, halfDomainWidth, _CMP_GT_OQ), domainSizeX),
						_mm256_and_ps(_mm256_cmp_ps(dx, negHalfDomainWidth, _CMP_LT_OQ), domainSizeX)
					));

				dy = _mm256_sub_ps(dy,
					_mm256_sub_ps(
						_mm256_and_ps(_mm256_cmp_ps(dy, halfDomainHeight, _CMP_GT_OQ), domainSizeY),
						_mm256_and_ps(_mm256_cmp_ps(dy, negHalfDomainHeight, _CMP_LT_OQ), domainSizeY)
					));

				dz = _mm256_sub_ps(dz,
					_mm256_sub_ps(
						_mm256_and_ps(_mm256_cmp_ps(dz, halfDomainDepth, _CMP_GT_OQ), domainSizeZ),
						_mm256_and_ps(_mm256_cmp_ps(dz, negHalfDomainDepth, _CMP_LT_OQ), domainSizeZ)
					));
			}

			__m256 distSq =
				_mm256_add_ps(
					_mm256_add_ps(
						_mm256_mul_ps(dx, dx),
						_mm256_mul_ps(dy, dy)),
					_mm256_add_ps(
						_mm256_mul_ps(dz, dz),
						softVec));

			__m256 invDist = _mm256_rsqrt_ps(distSq);

			__m256 nrTerm = _mm256_mul_ps(pointFive, distSq);
			nrTerm = _mm256_mul_ps(nrTerm, invDist);
			nrTerm = _mm256_mul_ps(nrTerm, invDist);

			__m256 nrFactor = _mm256_sub_ps(threeHalfs, nrTerm);

			invDist = _mm256_mul_ps(invDist, nrFactor);

			__m256 invDist3 = _mm256_mul_ps(invDist, _mm256_mul_ps(invDist, invDist));

			__m256 factor = _mm256_mul_ps(gVec, _mm256_mul_ps(mj, invDist3));

			totalAccX = _mm256_add_ps(totalAccX, _mm256_mul_ps(dx, factor));
			totalAccY = _mm256_add_ps(totalAccY, _mm256_mul_ps(dy, factor));
			totalAccZ = _mm256_add_ps(totalAccZ, _mm256_mul_ps(dz, factor));
		}

		float accX_array[8], accY_array[8], accZ_array[8];
		_mm256_storeu_ps(accX_array, totalAccX);
		_mm256_storeu_ps(accY_array, totalAccY);
		_mm256_storeu_ps(accZ_array, totalAccZ);

		float finalAccX = 0.0f;
		float finalAccY = 0.0f;
		float finalAccZ = 0.0f;

		for (int k = 0; k < 8; k++) {
			finalAccX += accX_array[k];
			finalAccY += accY_array[k];
			finalAccZ += accZ_array[k];
		}

		for (; j < n; j++) {
			float dx = posXPtr[j] - p_ix;
			float dy = posYPtr[j] - p_iy;
			float dz = posZPtr[j] - p_iz;

			if (myVar.isPeriodicBoundaryEnabled) {
				dx -= myVar.domainSize3D.x * ((dx > myVar.halfDomain3DWidth) - (dx < -myVar.halfDomain3DWidth));
				dy -= myVar.domainSize3D.y * ((dy > myVar.halfDomain3DHeight) - (dy < -myVar.halfDomain3DHeight));
				dz -= myVar.domainSize3D.z * ((dz > myVar.halfDomain3DDepth) - (dz < -myVar.halfDomain3DDepth));
			}

			float distSq = dx * dx + dy * dy + dz * dz + myVar.softening;

			float invDist = 1.0f / std::sqrt(distSq);
			float invDist3 = invDist * invDist * invDist;
			float factor = myVar.G * massPtr[j] * invDist3;

			finalAccX += dx * factor;
			finalAccY += dy * factor;
			finalAccZ += dz * factor;
		}

		accXPtr[i] = finalAccX;
		accYPtr[i] = finalAccY;
		accZPtr[i] = finalAccZ;
	}
}

void Physics3D::temperatureCalculation(std::vector<ParticlePhysics3D>& pParticles, std::vector<ParticleRendering3D>& rParticles, UpdateVariables& myVar) {

	for (size_t i = 0; i < pParticles.size(); i++) {
		ParticlePhysics3D& p = pParticles[i];
		auto it = SPHMaterials::idToMaterial.find(rParticles[i].sphLabel);
		if (it != SPHMaterials::idToMaterial.end()) {
			SPHMaterial* pMat = it->second;

			float pTotalVel = sqrtf(p.vel.x * p.vel.x + p.vel.y * p.vel.y);
			float pTotalPrevVel = sqrtf(p.prevVel.x * p.prevVel.x + p.prevVel.y * p.prevVel.y);

			p.ke = 0.5f * p.sphMass * pTotalVel * pTotalVel;
			p.prevKe = 0.5f * p.sphMass * pTotalPrevVel * pTotalPrevVel;

			float q = std::abs(p.ke - p.prevKe);

			float dTemp = q / (2.0f * pMat->heatConductivity * p.sphMass + 1.0f);
			p.temp += dTemp;

			float tempDifference = p.temp - myVar.ambientTemp;
			float dTempCooling = -(pMat->heatConductivity * myVar.globalAmbientHeatRate) * tempDifference * myVar.timeFactor;
			p.temp += dTempCooling;


			if (p.temp >= pMat->hotPoint) {
				p.sphMass = pMat->hotMassMult;
				p.mass = UpdateVariables::particleBaseMass * p.sphMass;
				p.restDens = pMat->hotRestDens;
				p.stiff = pMat->hotStiff;
				p.visc = pMat->hotVisc;
				p.cohesion = pMat->hotCohesion;
				if (pMat->coldPoint == 0.0f) {
					p.isHotPoint = true;
				}
			}
			else if (p.temp <= pMat->coldPoint) {
				p.sphMass = pMat->coldMassMult;
				p.mass = UpdateVariables::particleBaseMass * p.sphMass;
				p.restDens = pMat->coldRestDens;
				p.stiff = pMat->coldStiff;
				p.visc = pMat->coldVisc;
				p.cohesion = pMat->coldCohesion;
			}
			else {
				p.sphMass = pMat->massMult;
				p.mass = UpdateVariables::particleBaseMass * p.sphMass;
				p.restDens = pMat->restDens;
				p.stiff = pMat->stiff;
				p.visc = pMat->visc;
				p.cohesion = pMat->cohesion;
				if (pMat->coldPoint != 0.0f) {
					p.isHotPoint = true;
				}
			}

			if (pMat->coldPoint == 0.0f) {
				if (p.temp <= pMat->hotPoint && p.isHotPoint) {
					p.hasSolidified = true;
					p.isHotPoint = false;
				}
			}
			else {
				if (p.temp <= pMat->coldPoint && p.isHotPoint) {
					p.hasSolidified = true;
					p.isHotPoint = false;
				}
			}
		}
		else {
			float pTotalVel = sqrtf(p.vel.x * p.vel.x + p.vel.y * p.vel.y + p.vel.z * p.vel.z);
			float pTotalPrevVel = sqrtf(p.prevVel.x * p.prevVel.x + p.prevVel.y * p.prevVel.y + p.prevVel.z * p.prevVel.z);

			p.ke = 0.5f * p.sphMass * pTotalVel * pTotalVel;
			p.prevKe = 0.5f * p.sphMass * pTotalPrevVel * pTotalPrevVel;

			float q = std::abs(p.ke - p.prevKe);

			float dTemp = q / (2.0f * 0.05f * p.sphMass + 1.0f);
			p.temp += dTemp;

			float tempDifference = p.temp - myVar.ambientTemp;
			float dTempCooling = -(0.05f * myVar.globalAmbientHeatRate) * tempDifference * myVar.timeFactor;
			p.temp += dTempCooling;
		}
	}
}

void Physics3D::readFlattenBack3D(std::vector<ParticlePhysics3D>& pParticles3D) {
	size_t particleCount = pParticles3D.size();

#pragma omp parallel for schedule(static)
	for (int i = 0; i < static_cast<int>(particleCount); i++) {

		pParticles3D[i].pos.x = posX[i];
		pParticles3D[i].pos.y = posY[i];
		pParticles3D[i].pos.z = posZ[i];

		pParticles3D[i].vel.x = velX[i];
		pParticles3D[i].vel.y = velY[i];
		pParticles3D[i].vel.z = velZ[i];

		pParticles3D[i].acc.x = accX[i];
		pParticles3D[i].acc.y = accY[i];
		pParticles3D[i].acc.z = accZ[i];

		pParticles3D[i].prevVel.x = prevVelX[i];
		pParticles3D[i].prevVel.y = prevVelY[i];
		pParticles3D[i].prevVel.z = prevVelZ[i];

		pParticles3D[i].mass = mass[i];

		pParticles3D[i].temp = temp[i];
	}
}

void Physics3D::integrateStart3D(
	std::vector<ParticlePhysics3D>& pParticles3D,
	std::vector<ParticleRendering3D>& rParticles3D,
	UpdateVariables& myVar) {

	float dt = myVar.timeFactor;
	float halfDt = dt * 0.5f;
	float sphMaxVelSq = myVar.sphMaxVel * myVar.sphMaxVel;

#pragma omp parallel for schedule(dynamic)
	for (size_t i = 0; i < pParticles3D.size(); i++) {
		ParticlePhysics3D& p = pParticles3D[i];

		if (rParticles3D[i].isPinned) {
			continue;
		}

		p.prevVel = p.vel;

		p.vel += p.acc * halfDt;

		if (myVar.isSPHEnabled) {
			float vSq =
				p.vel.x * p.vel.x +
				p.vel.y * p.vel.y +
				p.vel.z * p.vel.z;

			if (vSq > sphMaxVelSq) {
				float prevVSq =
					p.prevVel.x * p.prevVel.x +
					p.prevVel.y * p.prevVel.y +
					p.prevVel.z * p.prevVel.z;

				if (prevVSq > 0.00001f) {
					float invPrevLen = myVar.sphMaxVel / sqrtf(prevVSq);
					p.prevVel *= invPrevLen;
				}

				float invLen = myVar.sphMaxVel / sqrtf(vSq);
				p.vel *= invLen;
			}
		}

		p.pos += p.vel * dt;

		if (myVar.isPeriodicBoundaryEnabled) {

			if (p.pos.x < -myVar.halfDomain3DWidth)
				p.pos.x += myVar.domainSize3D.x;

			else if (p.pos.x >= myVar.halfDomain3DWidth)
				p.pos.x -= myVar.domainSize3D.x;

			if (p.pos.y < -myVar.halfDomain3DHeight)
				p.pos.y += myVar.domainSize3D.y;
			else if (p.pos.y >= myVar.halfDomain3DHeight)
				p.pos.y -= myVar.domainSize3D.y;

			if (p.pos.z < -myVar.halfDomain3DDepth)
				p.pos.z += myVar.domainSize3D.z;
			else if (p.pos.z >= myVar.halfDomain3DDepth)
				p.pos.z -= myVar.domainSize3D.z;
		}
	}
}

void Physics3D::createConstraints(std::vector<ParticlePhysics3D>& pParticles, std::vector<ParticleRendering3D>& rParticles, bool& constraintCreateSpecialFlag,
	UpdateVariables& myVar, UpdateParameters& myParam) {

	bool shouldCreateConstraints = IO::shortcutPress(KEY_P) || myVar.constraintAllSolids || constraintCreateSpecialFlag || myVar.constraintSelected;

	for (size_t i = 0; i < pParticles.size(); i++) {
		ParticlePhysics3D& pi = pParticles[i];

		if (constraintCreateSpecialFlag) {
			if (!rParticles[i].isBeingDrawn) {
				continue;
			}
		}

		if (myVar.constraintSelected) {
			if (!rParticles[i].isSelected) {
				continue;
			}
		}

		SPHMaterial* pMatI = nullptr;
		auto matItI = SPHMaterials::idToMaterial.find(rParticles[i].sphLabel);
		if (matItI != SPHMaterials::idToMaterial.end()) {
			pMatI = matItI->second;
		}

		if (shouldCreateConstraints) {
			if (pMatI) {
				if (pMatI->coldPoint == 0.0f) {
					if (pi.temp >= pMatI->hotPoint) continue;
				}
				else {
					if (pi.temp >= pMatI->coldPoint) continue;
				}
			}
		}
		else {
			if (!pi.hasSolidified) continue;
			constraintMap.clear();
			pi.hasSolidified = false;
		}

		std::vector<size_t> neighborIndices = QueryNeighbors3D::queryNeighbors3D(myParam, myVar.hasAVX2, 64, pi.pos);

		for (size_t j : neighborIndices) {
			size_t neighborIndex = j;

			if (neighborIndex == i) continue;

			ParticlePhysics3D& pj = myParam.pParticles3D[neighborIndex];

			float distSq = glm::dot(pj.pos - pi.pos, pj.pos - pi.pos);

			if (distSq > 12.0f) {
				continue;
			}

			if (constraintCreateSpecialFlag) {
				if (!rParticles[neighborIndex].isBeingDrawn) {
					continue;
				}
			}

			if (myVar.constraintSelected) {
				if (!rParticles[neighborIndex].isSelected) {
					continue;
				}
			}

			SPHMaterial* pMatJ = nullptr;
			auto matItJ = SPHMaterials::idToMaterial.find(rParticles[neighborIndex].sphLabel);
			if (matItJ != SPHMaterials::idToMaterial.end()) {
				pMatJ = matItJ->second;
			}

			if (pMatI && pMatJ && pMatI->coldPoint == 0.0f && pMatJ->coldPoint != 0.0f) {
				continue;
			}

			uint64_t key = makeKey(pi.id, pj.id);
			if (constraintMap.find(key) != constraintMap.end()) {
				continue;
			}

			float resistance = 0.6f;
			if (pMatI && pMatJ) {
				resistance = (pMatI->constraintResistance + pMatJ->constraintResistance) * 0.5f;
			}

			float plasticityPoint = 0.6f;
			if (pMatI && pMatJ) {
				plasticityPoint = (pMatI->constraintPlasticPoint + pMatJ->constraintPlasticPoint) * 0.5f;
			}

			if (pi.id < pj.id) {

				float currentDist = glm::distance(pi.pos, pj.pos);
				bool broken = false;
				if (pMatI && pMatJ) {
					particleConstraints.push_back({ pi.id, pj.id, currentDist, currentDist, pMatI->constraintStiffness, resistance, 0.0f, plasticityPoint, broken });
				}
				else {
					float defaultStiffness = 60.0f;
					particleConstraints.push_back({ pi.id, pj.id, currentDist, currentDist, defaultStiffness, resistance, 0.0f, plasticityPoint, broken });
				}
				constraintMap[key] = &particleConstraints.back();
			}
		}
	}

	constraintCreateSpecialFlag = false;
	myVar.constraintAllSolids = false;
	myVar.constraintSelected = false;
}

void Physics3D::constraints(std::vector<ParticlePhysics3D>& pParticles, std::vector<ParticleRendering3D>& rParticles, UpdateVariables& myVar) {

	if (myVar.deleteAllConstraints) {
		particleConstraints.clear();
		constraintMap.clear();
		myVar.deleteAllConstraints = false;
		return;
	}

	uint32_t maxId = 0;
	for (const auto& p : pParticles) if (p.id > maxId) maxId = p.id;

	idToIndexTable.assign(maxId + 1, -1);
	for (size_t i = 0; i < pParticles.size(); i++) {
		idToIndexTable[pParticles[i].id] = i;
	}

	auto getIdx = [&](uint32_t id) -> int64_t {
		if (id >= idToIndexTable.size()) return -1;
		return idToIndexTable[id];
		};

	if (myVar.deleteSelectedConstraints) {
		for (auto& constraint : particleConstraints) {
			int64_t idx1 = getIdx(constraint.id1);
			int64_t idx2 = getIdx(constraint.id2);

			if (idx1 == -1 || idx2 == -1) {
				constraint.isBroken = true;
			}
			else if (rParticles[idx1].isSelected || rParticles[idx2].isSelected) {
				constraint.isBroken = true;
			}
		}
		myVar.deleteSelectedConstraints = false;
	}

	if (!particleConstraints.empty()) {
		auto new_end = std::remove_if(particleConstraints.begin(), particleConstraints.end(),
			[&](const auto& constraint) {
				int64_t idx1 = getIdx(constraint.id1);
				int64_t idx2 = getIdx(constraint.id2);
				return idx1 == -1 || idx2 == -1 || constraint.isBroken;
			});

		for (auto it = new_end; it != particleConstraints.end(); ++it) {
			constraintMap.erase(makeKey(it->id1, it->id2));
		}
		particleConstraints.erase(new_end, particleConstraints.end());

		bool enforceTriangles = true;

		myVar.frameCount++;
		if (enforceTriangles && !particleConstraints.empty() && myVar.frameCount % 5 == 0) {

			std::vector<std::vector<uint32_t>> adjacency(maxId + 1);

			for (const auto& c : particleConstraints) {
				if (c.isBroken) continue;

				if (c.id1 <= maxId && c.id2 <= maxId) {
					adjacency[c.id1].push_back(c.id2);
					adjacency[c.id2].push_back(c.id1);
				}
			}

			for (auto& constraint : particleConstraints) {
				if (constraint.isBroken) continue;

				uint32_t idA = constraint.id1;
				uint32_t idB = constraint.id2;

				bool partOfTriangle = false;

				const std::vector<uint32_t>& neighborsOfA = adjacency[idA];
				const std::vector<uint32_t>& neighborsOfB = adjacency[idB];

				for (uint32_t neighborA : neighborsOfA) {

					for (uint32_t neighborB : neighborsOfB) {
						if (neighborA == neighborB) {
							partOfTriangle = true;
							break;
						}
					}
					if (partOfTriangle) break;
				}

				if (!partOfTriangle) {
					constraint.isBroken = true;
				}
			}
		}

		const int substeps = 15;
		for (int step = 0; step < substeps; step++) {

#pragma omp parallel for schedule(dynamic)
			for (int64_t i = 0; i < (int64_t)particleConstraints.size(); i++) {
				auto& constraint = particleConstraints[i];

				if (constraint.isBroken) continue;

				int64_t idx1 = getIdx(constraint.id1);
				int64_t idx2 = getIdx(constraint.id2);

				if (idx1 == -1 || idx2 == -1) {
					constraint.isBroken = true;
					continue;
				}

				ParticlePhysics3D& pi = pParticles[idx1];
				ParticlePhysics3D& pj = pParticles[idx2];

				SPHMaterial* pMatI = SPHMaterials::idToMaterial[rParticles[idx1].sphLabel];
				SPHMaterial* pMatJ = SPHMaterials::idToMaterial[rParticles[idx2].sphLabel];

				glm::vec3 delta = pj.pos - pi.pos;

				if (myVar.isPeriodicBoundaryEnabled) {
					delta.x = fmod(delta.x + myVar.domainSize.x * 1.5f, myVar.domainSize.x) - myVar.domainSize.x * 0.5f;
					delta.y = fmod(delta.y + myVar.domainSize.y * 1.5f, myVar.domainSize.y) - myVar.domainSize.y * 0.5f;
				}

				float currentLength = glm::length(delta);
				if (currentLength < 0.0001f) continue;

				glm::vec3 dir = delta / currentLength;
				constraint.displacement = currentLength - constraint.restLength;

				if (!myVar.unbreakableConstraints) {
					if (pMatI && pMatJ) {
						if (!pMatI->isPlastic || !pMatJ->isPlastic) {
							constraint.isPlastic = false;
							float limit = (constraint.resistance * myVar.globalConstraintResistance) * constraint.restLength;
							if (std::abs(constraint.displacement) >= limit) {
								constraint.isBroken = true;
							}
						}
						else {
							constraint.isPlastic = true;
							if (std::abs(constraint.displacement) >= constraint.plasticityPoint * constraint.originalLength) {
								constraint.restLength += constraint.displacement;
							}

							float breakLimit = (constraint.originalLength + constraint.originalLength * (constraint.resistance * myVar.globalConstraintResistance)) * (pMatI->constraintPlasticPointMult + pMatJ->constraintPlasticPointMult) * 0.5f;

							if (std::abs(constraint.restLength) >= breakLimit) {
								constraint.isBroken = true;
							}
						}

						if (pi.isHotPoint || pj.isHotPoint) constraint.isBroken = true;
					}
				}

				if (myVar.timeFactor > 0.0f && myVar.gridExists && !constraint.isBroken) {

					glm::vec3 springForce = constraint.stiffness * constraint.displacement * dir * pi.mass * myVar.globalConstraintStiffnessMult;
					glm::vec3 relVel = pj.vel - pi.vel;
					glm::vec3 dampForce = -globalConstraintDamping * glm::dot(relVel, dir) * dir * pi.mass;
					glm::vec3 totalForce = springForce + dampForce;

#pragma omp atomic
					pi.acc.x += totalForce.x / pi.mass;
#pragma omp atomic
					pi.acc.y += totalForce.y / pi.mass;
#pragma omp atomic
					pi.acc.z += totalForce.z / pi.mass;
#pragma omp atomic
					pj.acc.x -= totalForce.x / pj.mass;
#pragma omp atomic
					pj.acc.y -= totalForce.y / pj.mass;
#pragma omp atomic
					pj.acc.z -= totalForce.z / pj.mass;

					float correctionFactor = constraint.stiffness * stiffCorrectionRatio * myVar.globalConstraintStiffnessMult;
					glm::vec3 correction = dir * constraint.displacement * correctionFactor;
					float massSum = pi.mass + pj.mass;
					glm::vec3 correctionI = correction * (pj.mass / massSum);
					glm::vec3 correctionJ = correction * (pi.mass / massSum);

#pragma omp atomic
					pi.pos.x += correctionI.x;
#pragma omp atomic
					pi.pos.y += correctionI.y;
#pragma omp atomic
					pi.pos.z += correctionI.z;
#pragma omp atomic
					pj.pos.x -= correctionJ.x;
#pragma omp atomic
					pj.pos.y -= correctionJ.y;
#pragma omp atomic
					pj.pos.z -= correctionJ.z;
				}
			}
		}
	}
}

void Physics3D::pausedConstraints(std::vector<ParticlePhysics3D>& pParticles, std::vector<ParticleRendering3D>& rParticles, UpdateVariables& myVar) {

	for (size_t i = 0; i < particleConstraints.size(); i++) {
		auto& constraint = particleConstraints[i];

		float prevLength = constraint.restLength;
	}
}

void Physics3D::integrateEnd3D(std::vector<ParticlePhysics3D>& pParticles3D, std::vector<ParticleRendering3D>& rParticles3D, UpdateVariables& myVar) {
#pragma omp parallel for schedule(dynamic)
	for (size_t i = 0; i < pParticles3D.size(); i++) {

		if (rParticles3D[i].isPinned) {
			continue;
		}

		pParticles3D[i].vel += pParticles3D[i].acc * (myVar.timeFactor * 0.5f);
	}
}

void Physics3D::spawnCorrection(UpdateParameters& myParam, bool& hasAVX2, const int& iterations) {

#pragma omp parallel for
	for (size_t i = 0; i < myParam.pParticles3D.size(); i++) {

		ParticlePhysics3D& pi = myParam.pParticles3D[i];

		std::vector<size_t> neighborIndices =
			QueryNeighbors3D::queryNeighbors3D(myParam, hasAVX2, 64, pi.pos);

		for (size_t j : neighborIndices) {
			size_t neighborIndex = j;

			if (neighborIndex == i) continue;

			ParticlePhysics3D& pj = myParam.pParticles3D[neighborIndex];

			if (!myParam.rParticles3D[neighborIndex].isBeingDrawn || !myParam.rParticles3D[i].isBeingDrawn) continue;

			glm::vec3 d = pj.pos - pi.pos;

			float dSq = glm::dot(d, d);

			const float minDist = 2.4f;
			const float minDistSq = minDist * minDist;

			if (dSq > 0.000001f && dSq < minDistSq) {

				float dist = std::sqrt(dSq);

				glm::vec3 dir = -d / dist;

				float penetration = minDist - dist;

				float totalMass = pi.mass + pj.mass;

				if (totalMass > 0.0f) {

					float piMove = pj.mass / totalMass;
					float pjMove = pi.mass / totalMass;

					glm::vec3 correction = dir * penetration;

					pi.pos += correction * piMove;
					pj.pos -= correction * pjMove;
				}
			}
		}

		myParam.rParticles3D[i].spawnCorrectIter++;
	}
}