#include "Physics/physics.h"

// This is used in predict trajectory inside particleSpawning.cpp
glm::vec2 Physics::calculateForceFromGridOld(std::vector<ParticlePhysics>& pParticles, UpdateVariables& myVar, ParticlePhysics& pParticle) {

	glm::vec2 totalForce = { 0.0f,0.0f };

	uint32_t gridIdx = 0;
	const uint32_t nodeCount = static_cast<uint32_t>(globalNodes.size());

	const float thetaSq = myVar.theta * myVar.theta;
	const float softeningSq = myVar.softening * myVar.softening;
	const float Gf = static_cast<float>(myVar.G);
	const float pmass = pParticle.mass;
	auto* particlesPtr = pParticles.data();

	while (gridIdx < nodeCount) {
		Node& grid = globalNodes[gridIdx];

		float gridMass = grid.gridMass;
		if (gridMass <= 0.0f) {
			gridIdx += grid.next + 1;
			continue;
		}

		const glm::vec2 gridCOM = grid.centerOfMass;
		const float gridSize = grid.size;

		glm::vec2 d = gridCOM - pParticle.pos;

		if (myVar.isPeriodicBoundaryEnabled) {
			d.x -= myVar.domainSize.x * ((d.x > myVar.halfDomainWidth) - (d.x < -myVar.halfDomainWidth));
			d.y -= myVar.domainSize.y * ((d.y > myVar.halfDomainHeight) - (d.y < -myVar.halfDomainHeight));
		}

		float distanceSq = d.x * d.x + d.y * d.y + softeningSq;

		bool isSubgridsEmty = true;
		uint32_t s00 = grid.subGrids[0][0];
		uint32_t s01 = grid.subGrids[0][1];
		uint32_t s10 = grid.subGrids[1][0];
		uint32_t s11 = grid.subGrids[1][1];
		if (s00 != UINT32_MAX || s01 != UINT32_MAX || s10 != UINT32_MAX || s11 != UINT32_MAX) {
			isSubgridsEmty = false;
		}

		float gridSizeSq = gridSize * gridSize;

		if ((gridSizeSq < thetaSq * distanceSq) || isSubgridsEmty) {

			if ((grid.endIndex - grid.startIndex) == 1) {
				const ParticlePhysics& other = particlesPtr[grid.startIndex];
				if (std::abs(other.pos.x - pParticle.pos.x) < 0.001f && std::abs(other.pos.y - pParticle.pos.y) < 0.001f) {
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
					float temperatureDifference = gridAverageTemp - pParticle.temp;

					float distance = 0.0f;
					if (distanceSq > 1e-16f) distance = 1.0f / invDistance;
					if (distance > 1e-8f) {
						float heatTransfer = myVar.globalHeatConductivity * temperatureDifference / distance;
						pParticle.temp += heatTransfer * myVar.timeFactor;
					}
				}
			}

			gridIdx += grid.next + 1;
		}
		else {
			++gridIdx;
		}
	}

	return totalForce;
}

void Physics::calculateForceFromGrid(UpdateVariables& myVar) {
#pragma omp parallel for schedule(dynamic)
	for (size_t i = 0; i < posX.size(); i++) {
		glm::vec2 totalForce = { 0.0f,0.0f };

		uint32_t gridIdx = 0;
		const uint32_t nodeCount = static_cast<uint32_t>(globalNodes.size());

		const float thetaSq = myVar.theta * myVar.theta;
		const float softeningSq = myVar.softening * myVar.softening;
		const float Gf = myVar.G;
		const float pmass = mass[i];

		while (gridIdx < nodeCount) {
			Node& grid = globalNodes[gridIdx];

			float gridMass = grid.gridMass;
			if (gridMass <= 0.0f) {
				gridIdx += grid.next + 1;
				continue;
			}

			const glm::vec2 gridCOM = grid.centerOfMass;
			const float gridSize = grid.size;

			glm::vec2 d = gridCOM - glm::vec2{ posX[i], posY[i] };

			if (myVar.isPeriodicBoundaryEnabled) {
				d.x -= myVar.domainSize.x * ((d.x > myVar.halfDomainWidth) - (d.x < -myVar.halfDomainWidth));
				d.y -= myVar.domainSize.y * ((d.y > myVar.halfDomainHeight) - (d.y < -myVar.halfDomainHeight));
			}

			float distanceSq = d.x * d.x + d.y * d.y + softeningSq;

			bool isSubgridsEmty = true;
			uint32_t s00 = grid.subGrids[0][0];
			uint32_t s01 = grid.subGrids[0][1];
			uint32_t s10 = grid.subGrids[1][0];
			uint32_t s11 = grid.subGrids[1][1];
			if (s00 != UINT32_MAX || s01 != UINT32_MAX || s10 != UINT32_MAX || s11 != UINT32_MAX) {
				isSubgridsEmty = false;
			}

			float gridSizeSq = gridSize * gridSize;

			if ((gridSizeSq < thetaSq * distanceSq) || isSubgridsEmty) {

				if ((grid.endIndex - grid.startIndex) == 1) {
					if (std::abs(posX[grid.startIndex] - posX[i]) < 0.001f && std::abs(posY[grid.startIndex] - posY[i]) < 0.001f) {
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
	}
}

void Physics::flattenParticles(std::vector<ParticlePhysics>& pParticles) {

	size_t particleCount = pParticles.size();

	posX.resize(particleCount);
	posY.resize(particleCount);
	accX.resize(particleCount);
	accY.resize(particleCount);
	velX.resize(particleCount);
	velY.resize(particleCount);
	prevVelX.resize(particleCount);
	prevVelY.resize(particleCount);
	mass.resize(particleCount);
	temp.resize(particleCount);

#pragma omp parallel for schedule(static)
	for (int i = 0; i < static_cast<int>(particleCount); i++) {

		const auto& particle = pParticles[i];

		posX[i] = particle.pos.x;
		posY[i] = particle.pos.y;

		accX[i] = particle.acc.x;
		accY[i] = particle.acc.y;

		velX[i] = particle.vel.x;
		velY[i] = particle.vel.y;

		prevVelX[i] = particle.prevVel.x;
		prevVelY[i] = particle.prevVel.y;

		mass[i] = particle.mass;

		temp[i] = particle.temp;
	}
}

void Physics::naiveGravity(std::vector<ParticlePhysics>& pParticles, UpdateVariables& myVar) {
	int n = static_cast<int>(posX.size());
	__m256 gVec = _mm256_set1_ps(myVar.G);
	__m256 softVec = _mm256_set1_ps(myVar.softening);
	const float* posXPtr = posX.data();
	const float* posYPtr = posY.data();
	const float* massPtr = mass.data();
	float* accXPtr = accX.data();
	float* accYPtr = accY.data();

	__m256 domainSizeX = _mm256_set1_ps(myVar.domainSize.x);
	__m256 domainSizeY = _mm256_set1_ps(myVar.domainSize.y);
	__m256 halfDomainWidth = _mm256_set1_ps(myVar.halfDomainWidth);
	__m256 halfDomainHeight = _mm256_set1_ps(myVar.halfDomainHeight);
	__m256 negHalfDomainWidth = _mm256_set1_ps(-myVar.halfDomainWidth);
	__m256 negHalfDomainHeight = _mm256_set1_ps(-myVar.halfDomainHeight);

#pragma omp parallel for schedule(static)
	for (int i = 0; i < n; i++) {
		accX[i] = 0.0f;
		accY[i] = 0.0f;
	}

#pragma omp parallel for schedule(dynamic, 64)
	for (int i = 0; i < n; i++) {
		float p_ix = posXPtr[i];
		float p_iy = posYPtr[i];
		__m256 pxi = _mm256_set1_ps(p_ix);
		__m256 pyi = _mm256_set1_ps(p_iy);
		__m256 totalAccX = _mm256_setzero_ps();
		__m256 totalAccY = _mm256_setzero_ps();

		int j;
		for (j = 0; j <= n - 8; j += 8) {
			__m256 pxj = _mm256_loadu_ps(&posXPtr[j]);
			__m256 pyj = _mm256_loadu_ps(&posYPtr[j]);
			__m256 mj = _mm256_loadu_ps(&massPtr[j]);

			__m256 dx = _mm256_sub_ps(pxj, pxi);
			__m256 dy = _mm256_sub_ps(pyj, pyi);

			if (myVar.isPeriodicBoundaryEnabled) {
				__m256 mask_pos_x = _mm256_cmp_ps(dx, halfDomainWidth, _CMP_GT_OQ);
				__m256 mask_neg_x = _mm256_cmp_ps(dx, negHalfDomainWidth, _CMP_LT_OQ);
				__m256 correction_x = _mm256_sub_ps(
					_mm256_and_ps(mask_pos_x, domainSizeX),
					_mm256_and_ps(mask_neg_x, domainSizeX)
				);
				dx = _mm256_sub_ps(dx, correction_x);

				__m256 mask_pos_y = _mm256_cmp_ps(dy, halfDomainHeight, _CMP_GT_OQ);
				__m256 mask_neg_y = _mm256_cmp_ps(dy, negHalfDomainHeight, _CMP_LT_OQ);
				__m256 correction_y = _mm256_sub_ps(
					_mm256_and_ps(mask_pos_y, domainSizeY),
					_mm256_and_ps(mask_neg_y, domainSizeY)
				);
				dy = _mm256_sub_ps(dy, correction_y);
			}

			__m256 distSq =
				_mm256_add_ps(
					_mm256_add_ps(_mm256_mul_ps(dx, dx),
						_mm256_mul_ps(dy, dy)),
					softVec);
			__m256 invDist = _mm256_div_ps(
				_mm256_set1_ps(1.0f),
				_mm256_sqrt_ps(distSq));
			__m256 invDist3 =
				_mm256_mul_ps(invDist,
					_mm256_mul_ps(invDist, invDist));
			__m256 factor =
				_mm256_mul_ps(gVec,
					_mm256_mul_ps(mj, invDist3));
			totalAccX = _mm256_add_ps(totalAccX, _mm256_mul_ps(dx, factor));
			totalAccY = _mm256_add_ps(totalAccY, _mm256_mul_ps(dy, factor));
		}

		float accX_array[8], accY_array[8];
		_mm256_storeu_ps(accX_array, totalAccX);
		_mm256_storeu_ps(accY_array, totalAccY);
		float finalAccX =
			accX_array[0] + accX_array[1] + accX_array[2] + accX_array[3] +
			accX_array[4] + accX_array[5] + accX_array[6] + accX_array[7];
		float finalAccY =
			accY_array[0] + accY_array[1] + accY_array[2] + accY_array[3] +
			accY_array[4] + accY_array[5] + accY_array[6] + accY_array[7];

		for (; j < n; j++) {
			float dx = posXPtr[j] - p_ix;
			float dy = posYPtr[j] - p_iy;

			if (myVar.isPeriodicBoundaryEnabled) {
				dx -= myVar.domainSize.x * ((dx > myVar.halfDomainWidth) - (dx < -myVar.halfDomainWidth));
				dy -= myVar.domainSize.y * ((dy > myVar.halfDomainHeight) - (dy < -myVar.halfDomainHeight));
			}

			float distSq = dx * dx + dy * dy + myVar.softening;
			float invDist = 1.0f / std::sqrt(distSq);
			float invDist3 = invDist * invDist * invDist;
			float factor = myVar.G * massPtr[j] * invDist3;
			finalAccX += dx * factor;
			finalAccY += dy * factor;
		}

		accXPtr[i] = finalAccX;
		accYPtr[i] = finalAccY;
	}
}

void Physics::readFlattenBack(std::vector<ParticlePhysics>& pParticles) {

	size_t particleCount = pParticles.size();

#pragma omp parallel for schedule(static)
	for (int i = 0; i < static_cast<int>(particleCount); i++) {

		pParticles[i].pos.x = posX[i];
		pParticles[i].pos.y = posY[i];

		pParticles[i].vel.x = velX[i];
		pParticles[i].vel.y = velY[i];

		pParticles[i].acc.x = accX[i];
		pParticles[i].acc.y = accY[i];

		pParticles[i].prevVel.x = prevVelX[i];
		pParticles[i].prevVel.y = prevVelY[i];

		pParticles[i].mass = mass[i];

		pParticles[i].temp = temp[i];
	}
}

void Physics::temperatureCalculation(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, UpdateVariables& myVar) {

	for (size_t i = 0; i < pParticles.size(); i++) {
		ParticlePhysics& p = pParticles[i];
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
			float pTotalVel = sqrtf(p.vel.x * p.vel.x + p.vel.y * p.vel.y);
			float pTotalPrevVel = sqrtf(p.prevVel.x * p.prevVel.x + p.prevVel.y * p.prevVel.y);

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

void Physics::createConstraints(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, bool& constraintCreateSpecialFlag,
	UpdateVariables& myVar, UpdateParameters& myParam) {

	bool shouldCreateConstraints = IO::shortcutPress(KEY_P) || myVar.constraintAllSolids || constraintCreateSpecialFlag || myVar.constraintSelected;

	for (size_t i = 0; i < pParticles.size(); i++) {
		ParticlePhysics& pi = pParticles[i];

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

		std::vector<size_t> neighborIndices = QueryNeighbors::queryNeighbors(myParam, myVar.hasAVX2, 64, pi.pos);

		for (size_t j : neighborIndices) {
			size_t neighborIndex = j;

			if (neighborIndex == i) continue;

			ParticlePhysics& pj = myParam.pParticles[neighborIndex];

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

void Physics::constraints(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, UpdateVariables& myVar) {

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

				ParticlePhysics& pi = pParticles[idx1];
				ParticlePhysics& pj = pParticles[idx2];

				SPHMaterial* pMatI = SPHMaterials::idToMaterial[rParticles[idx1].sphLabel];
				SPHMaterial* pMatJ = SPHMaterials::idToMaterial[rParticles[idx2].sphLabel];

				glm::vec2 delta = pj.pos - pi.pos;

				if (myVar.isPeriodicBoundaryEnabled) {
					delta.x = fmod(delta.x + myVar.domainSize.x * 1.5f, myVar.domainSize.x) - myVar.domainSize.x * 0.5f;
					delta.y = fmod(delta.y + myVar.domainSize.y * 1.5f, myVar.domainSize.y) - myVar.domainSize.y * 0.5f;
				}

				float currentLength = glm::length(delta);
				if (currentLength < 0.0001f) continue;

				glm::vec2 dir = delta / currentLength;
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

					glm::vec2 springForce = constraint.stiffness * constraint.displacement * dir * pi.mass * myVar.globalConstraintStiffnessMult;
					glm::vec2 relVel = pj.vel - pi.vel;
					glm::vec2 dampForce = -globalConstraintDamping * glm::dot(relVel, dir) * dir * pi.mass;
					glm::vec2 totalForce = springForce + dampForce;

#pragma omp atomic
					pi.acc.x += totalForce.x / pi.mass;
#pragma omp atomic
					pi.acc.y += totalForce.y / pi.mass;
#pragma omp atomic
					pj.acc.x -= totalForce.x / pj.mass;
#pragma omp atomic
					pj.acc.y -= totalForce.y / pj.mass;

					float correctionFactor = constraint.stiffness * stiffCorrectionRatio * myVar.globalConstraintStiffnessMult;
					glm::vec2 correction = dir * constraint.displacement * correctionFactor;
					float massSum = pi.mass + pj.mass;
					glm::vec2 correctionI = correction * (pj.mass / massSum);
					glm::vec2 correctionJ = correction * (pi.mass / massSum);

#pragma omp atomic
					pi.pos.x += correctionI.x;
#pragma omp atomic
					pi.pos.y += correctionI.y;
#pragma omp atomic
					pj.pos.x -= correctionJ.x;
#pragma omp atomic
					pj.pos.y -= correctionJ.y;
				}
			}
		}
	}
}

void Physics::pausedConstraints(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, UpdateVariables& myVar) {

	for (size_t i = 0; i < particleConstraints.size(); i++) {
		auto& constraint = particleConstraints[i];

		float prevLength = constraint.restLength;
	}
}

void Physics::mergerSolver(
	std::vector<ParticlePhysics>& pParticles,
	std::vector<ParticleRendering>& rParticles,
	UpdateVariables& myVar,
	UpdateParameters& myParam)
{
	std::unordered_set<uint32_t> particleIdsToDelete;
	std::vector<size_t> indicesToDelete;

	int originalSize = static_cast<int>(pParticles.size());

	for (int i = originalSize - 1; i >= 0; i--) {
		ParticlePhysics& p = pParticles[i];
		ParticleRendering& r = rParticles[i];

		if (r.isDarkMatter || particleIdsToDelete.count(p.id)) continue;

		for (int j = r.neighbors - 1; j >= 0; j--) {
			uint32_t neighborId = myParam.neighborSearch.globalNeighborList[p.neighborOffset + j];

			size_t neighborIndex = myParam.neighborSearch.idToIndexTable[neighborId];

			if (neighborIndex >= pParticles.size()) continue;
			if (neighborIndex == i) continue;

			ParticlePhysics& pn = pParticles[neighborIndex];
			ParticleRendering& rn = rParticles[neighborIndex];

			if (rn.isDarkMatter || particleIdsToDelete.count(pn.id)) continue;

			glm::vec2 d = pn.pos - p.pos;
			float distanceSq = glm::dot(d, d);

			float combinedRadiusSq = (r.totalRadius + rn.totalRadius) * (r.totalRadius + rn.totalRadius);

			if (distanceSq <= combinedRadiusSq) {
				float originalMassP = p.mass;
				float originalMassN = pn.mass;

				if (originalMassP >= originalMassN) {
					p.mass = originalMassP + originalMassN;
					p.vel = (p.vel * originalMassP + pn.vel * originalMassN) / p.mass;

					float area1 = r.previousSize * r.previousSize;
					float area2 = rn.previousSize * rn.previousSize;
					float fullGrowthSize = sqrtf(area1 + area2);
					float maxOriginalSize = std::max(r.previousSize, rn.previousSize);
					float growthFactor = 0.25f;

					r.previousSize = maxOriginalSize + (fullGrowthSize - maxOriginalSize) * growthFactor;

					particleIdsToDelete.insert(pn.id);
					indicesToDelete.push_back(neighborIndex);
				}
				else {
					pn.mass = originalMassP + originalMassN;
					pn.vel = (pn.vel * originalMassN + p.vel * originalMassP) / pn.mass;

					float area1 = r.previousSize * r.previousSize;
					float area2 = rn.previousSize * rn.previousSize;
					float fullGrowthSize = sqrtf(area1 + area2);
					float maxOriginalSize = std::max(r.previousSize, rn.previousSize);
					float growthFactor = 0.25f;

					rn.previousSize = maxOriginalSize + (fullGrowthSize - maxOriginalSize) * growthFactor;

					particleIdsToDelete.insert(p.id);
					indicesToDelete.push_back(i);
				}

				break;
			}
		}
	}

	std::sort(indicesToDelete.begin(), indicesToDelete.end());
	indicesToDelete.erase(std::unique(indicesToDelete.begin(), indicesToDelete.end()), indicesToDelete.end());

	for (int k = static_cast<int>(indicesToDelete.size()) - 1; k >= 0; k--) {
		size_t index = indicesToDelete[k];
		if (index >= pParticles.size()) continue;

		uint32_t removedId = pParticles[index].id;

		std::swap(pParticles[index], pParticles.back());
		std::swap(rParticles[index], rParticles.back());

		uint32_t swappedId = pParticles[index].id;
		myParam.neighborSearch.idToIndexTable[swappedId] = index;

		pParticles.pop_back();
		rParticles.pop_back();

		if (removedId < myParam.neighborSearch.idToIndexTable.size()) {
			myParam.neighborSearch.idToIndexTable[removedId] = static_cast<size_t>(-1);
		}
	}
}

void Physics::integrateStart(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, UpdateVariables& myVar) {
	float dt = myVar.timeFactor;
	float halfDt = dt * 0.5f;
	float sphMaxVelSq = myVar.sphMaxVel * myVar.sphMaxVel;

#pragma omp parallel for schedule(dynamic)
	for (size_t i = 0; i < pParticles.size(); i++) {
		ParticlePhysics& p = pParticles[i];

		p.prevVel = p.vel;

		p.vel += p.acc * halfDt;

		if (myVar.isSPHEnabled) {
			float vSq = p.vel.x * p.vel.x + p.vel.y * p.vel.y;

			if (vSq > sphMaxVelSq) {
				float prevVSq = p.prevVel.x * p.prevVel.x + p.prevVel.y * p.prevVel.y;

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
			if (p.pos.x < 0.0f) p.pos.x += myVar.domainSize.x;
			else if (p.pos.x >= myVar.domainSize.x) p.pos.x -= myVar.domainSize.x;

			if (p.pos.y < 0.0f) p.pos.y += myVar.domainSize.y;
			else if (p.pos.y >= myVar.domainSize.y) p.pos.y -= myVar.domainSize.y;
		}
	}
}

void Physics::integrateEnd(std::vector<ParticlePhysics>& pParticles, UpdateVariables& myVar) {

#pragma omp parallel for schedule(dynamic)
	for (size_t i = 0; i < pParticles.size(); i++) {
		pParticles[i].vel += pParticles[i].acc * (myVar.timeFactor * 0.5f);
	}
}

void Physics::pruneParticles(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, UpdateVariables& myVar) {

	for (size_t i = 0; i < pParticles.size(); ) {
		float x = pParticles[i].pos.x;
		float y = pParticles[i].pos.y;

		if (x <= 0.0f || x >= myVar.domainSize.x || y <= 0.0f || y >= myVar.domainSize.y) {

			if (pParticles.size() > 1) {
				std::swap(pParticles[i], pParticles.back());
				std::swap(rParticles[i], rParticles.back());
			}
			pParticles.pop_back();
			rParticles.pop_back();
		}
		else {
			i++;
		}
	}
}

void Physics::spawnCorrection(UpdateParameters& myParam, bool& hasAVX2, const int& iterations) {

#pragma omp parallel for
	for (size_t i = 0; i < myParam.pParticles.size(); i++) {

		ParticlePhysics& pi = myParam.pParticles[i];

		std::vector<size_t> neighborIndices =
			QueryNeighbors::queryNeighbors(myParam, hasAVX2, 64, pi.pos);

		for (size_t j : neighborIndices) {
			size_t neighborIndex = j;

			if (neighborIndex == i) continue;

			ParticlePhysics& pj = myParam.pParticles[neighborIndex];

			if (!myParam.rParticles[neighborIndex].isBeingDrawn || !myParam.rParticles[i].isBeingDrawn) continue;

			glm::vec2 d = pj.pos - pi.pos;

			float dSq = glm::dot(d, d);

			const float minDist = 2.4f;
			const float minDistSq = minDist * minDist;

			if (dSq > 0.000001f && dSq < minDistSq) {

				float dist = std::sqrt(dSq);

				glm::vec2 dir = -d / dist;

				float penetration = minDist - dist;

				float totalMass = pi.mass + pj.mass;

				if (totalMass > 0.0f) {

					float piMove = pj.mass / totalMass;
					float pjMove = pi.mass / totalMass;

					glm::vec2 correction = dir * penetration;

					pi.pos += correction * piMove;
					pj.pos -= correction * pjMove;
				}
			}
		}

		myParam.rParticles[i].spawnCorrectIter++;
	}
}

// ----- Unused. Test code ----- //
void Physics::gravityGrid(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, UpdateVariables& myVar, glm::vec3& bb) {

	if (pParticles.empty()) return;

#pragma omp parallel for
	for (long long i = 0; i < static_cast<long long>(cells.size()); i++) {
		cells[i].mass = 0.0f;
		cells[i].particles.clear();
		cells[i].force = { 0.0f, 0.0f };
	}

	struct DepthInfo {
		int gridRes;
		float cellSize;
		size_t startIndex;
	};

	std::vector<DepthInfo> depthInfos;
	size_t currentStartIdx = 0;

	for (int depth = 0; depth < maxDepth; depth++) {
		int res = std::pow(2, depth + 1);
		float size = bb.z / static_cast<float>(res);
		depthInfos.push_back({ res, size, currentStartIdx });
		currentStartIdx += (res * res);
	}

	for (ParticlePhysics& p : pParticles) {
		for (int depth = 0; depth < maxDepth; depth++) {
			const auto& info = depthInfos[depth];

			float relX = p.pos.x - bb.x;
			float relY = p.pos.y - bb.y;
			int x = static_cast<int>(relX / info.cellSize);
			int y = static_cast<int>(relY / info.cellSize);

			if (x >= 0 && x < info.gridRes && y >= 0 && y < info.gridRes) {
				size_t cellIdx = info.startIndex + (y * info.gridRes + x);
				if (cellIdx < cells.size()) {
					cells[cellIdx].mass += p.mass;
					cells[cellIdx].particles.push_back(&p);
				}
			}
		}
	}

	for (const auto& info : depthInfos) {
		long long start = static_cast<long long>(info.startIndex);
		long long end = static_cast<long long>(info.startIndex + (info.gridRes * info.gridRes));

#pragma omp parallel for schedule(dynamic)
		for (long long i = start; i < end; i++) {
			GravityCell& ci = cells[i];

			if (ci.mass == 0.0f) continue;

			size_t localIdx = i - info.startIndex;
			int cix = localIdx % info.gridRes;
			int ciy = localIdx / info.gridRes;

			for (int dy = -11; dy <= 11; dy++) {
				for (int dx = -11; dx <= 11; dx++) {
					if (dx == 0 && dy == 0) continue;

					int cjx = cix + dx;
					int cjy = ciy + dy;

					if (cjx < 0 || cjx >= info.gridRes || cjy < 0 || cjy >= info.gridRes) continue;

					size_t neighborLocalIdx = cjy * info.gridRes + cjx;
					size_t finalNeighborIdx = info.startIndex + neighborLocalIdx;

					const GravityCell& cj = cells[finalNeighborIdx];

					if (cj.mass == 0.0f) continue;

					glm::vec2 d = cj.pos - ci.pos;
					float distSq = glm::dot(d, d);

					if (distSq < 1e-4f) continue;
					float dist = std::sqrt(distSq);

					float force = (myVar.G * ci.mass * cj.mass) / distSq;
					ci.force += force * (d / dist);
				}
			}
		}
	}

#pragma omp parallel for
	for (long long i = 0; i < static_cast<long long>(pParticles.size()); i++) {
		ParticlePhysics& p = pParticles[i];

		for (int depth = 0; depth < maxDepth; depth++) {
			const auto& info = depthInfos[depth];

			float relX = p.pos.x - bb.x;
			float relY = p.pos.y - bb.y;
			int x = static_cast<int>(relX / info.cellSize);
			int y = static_cast<int>(relY / info.cellSize);

			if (x >= 0 && x < info.gridRes && y >= 0 && y < info.gridRes) {
				size_t cellIdx = info.startIndex + (y * info.gridRes + x);

				if (cellIdx < cells.size() && cells[cellIdx].mass > 0.0f) {
					const GravityCell& c = cells[cellIdx];

					p.acc += c.force / c.mass;
				}
			}
		}
	}
}
// ----- Unused. Test code ----- //

