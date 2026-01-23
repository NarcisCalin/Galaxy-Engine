#include "Physics/physics.h"

glm::vec2 Physics::calculateForceFromGrid(std::vector<ParticlePhysics>& pParticles, UpdateVariables& myVar, ParticlePhysics& pParticle) {

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

			float invDistance = 1.0f / std::sqrt(distanceSq);
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
	}
}
__attribute__((target("avx")))
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
	}
}

void Physics::temperatureCalculation(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, UpdateVariables& myVar) {

	for (size_t i = 0; i < pParticles.size(); i++) {
		ParticlePhysics& p = pParticles[i];
		auto it = SPHMaterials::idToMaterial.find(rParticles[i].sphLabel);
		if (it != SPHMaterials::idToMaterial.end()) {
			SPHMaterial* pMat = it->second;

			float pTotalVel = sqrt(p.vel.x * p.vel.x + p.vel.y * p.vel.y);
			float pTotalPrevVel = sqrt(p.prevVel.x * p.prevVel.x + p.prevVel.y * p.prevVel.y);

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
			float pTotalVel = sqrt(p.vel.x * p.vel.x + p.vel.y * p.vel.y);
			float pTotalPrevVel = sqrt(p.prevVel.x * p.prevVel.x + p.prevVel.y * p.prevVel.y);

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

		for (uint32_t k = 0; k < pi.neighborCount; k++) {
			uint32_t nID = myParam.neighborSearch.globalNeighborList[pi.neighborOffset + k];

			size_t neighborIndex = myParam.neighborSearch.idToIndexTable[nID];

			ParticlePhysics& pj = myParam.pParticles[neighborIndex];

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

		for (int j = p.neighborCount - 1; j >= 0; j--) {
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
					float fullGrowthSize = sqrt(area1 + area2);
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
					float fullGrowthSize = sqrt(area1 + area2);
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

void Physics::collisions(ParticlePhysics& pParticleA, ParticlePhysics& pParticleB,
	ParticleRendering& rParticleA, ParticleRendering& rParticleB, float& radius) {

	// IN THIS CODE I ONLY KEEP THE POSITION CORRECTION. COLLISION MODE IS NOT SUPPORTED ANYMORE BUT THE COMMENTED CODE MIGHT STILL BE OF USE SOME TIME

	if (rParticleA.isDarkMatter || rParticleB.isDarkMatter) {
		return;
	}

	ParticlePhysics& pA = pParticleA;
	ParticlePhysics& pB = pParticleB;

	glm::vec2 posA = pA.pos;
	glm::vec2 posB = pB.pos;

	glm::vec2 d = posB - posA;
	float distanceSq = d.x * d.x + d.y * d.y;

	float radiiSum = radius + radius;
	float radiiSumSq = radiiSum * radiiSum;


	if (distanceSq > radiiSumSq || distanceSq <= 1e-6f) {
		return;
	}

	/*glm::vec2 velA = pA.vel;
	glm::vec2 velB = pB.vel;
	glm::vec2 relativeVel = velB - velA;

	float velocityNormal = glm::dot(d, relativeVel);*/

	float weightA = pB.mass / (pA.mass + pB.mass);
	float weightB = pA.mass / (pA.mass + pB.mass);

	float distance = sqrt(distanceSq);

	float penetration = radiiSum - distance;
	const float percent = 0.8f;
	const float slop = 0.01f;
	float correctionMag = std::max(penetration - slop, 0.0f) * percent;

	glm::vec2 normal = d / distance;
	glm::vec2 correction = normal * correctionMag;

	pA.pos -= weightA * correction;

	pB.pos += weightB * correction;


	/*if (velocityNormal >= 0.0f) {
		return;
	}

	float relativeVelSq = relativeVel.x * relativeVel.x + relativeVel.y * relativeVel.y;
	float discr = velocityNormal * velocityNormal - relativeVelSq * (distanceSq - radiiSumSq);

	if (relativeVelSq <= 0.0f || discr < 0.0f) {
		return;
	}

	float t = (-velocityNormal - sqrt(discr)) / relativeVelSq;

	const float epsilon = 1e-4f * dt;
	t = std::clamp(t, 0.0f, dt - epsilon);


	pA.pos -= velA * t;
	pB.pos -= velB * t;

	posA = pA.pos;
	posB = pB.pos;

	d = posB - posA;
	distanceSq = d.x * d.x + d.y * d.y;
	normal = d / sqrt(distanceSq);

	float impulseNumerator = -(1.0f + myVar.particleBounciness) * glm::dot(relativeVel, normal);
	float impulseDenominator = (1.0f / pA.mass + 1.0f / pB.mass);
	float j = impulseNumerator / impulseDenominator;

	glm::vec2 impulse = normal * j;

	pA.vel = velA - impulse / pA.mass;
	pB.vel = velB + impulse / pB.mass;

	pA.pos += pA.vel * t;
	pB.pos += pB.vel * t;*/
}

void Physics::buildGrid(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles,
	Physics& physics, glm::vec2& domainSize, const int& iterations) {

	// Some code from here is not needed anymore. I keep it because it might be useful some time

	//for (size_t i = 0; i < pParticles.size(); i++) {
 //       
	//	float thisSize = 8.0f; // Heuristic

	//	if (thisSize > cellSize) {
	//		cellSize = thisSize;
	//	}
	//}

	constexpr float cellSize = 8.0f; // <- Heuristic

	int cellAmountX = static_cast<int>(domainSize.x / cellSize);
	int cellAmountY = static_cast<int>(domainSize.y / cellSize);

	int totalCells = cellAmountX * cellAmountY;
	std::vector<std::vector<size_t>> cellList(totalCells);

	for (size_t i = 0; i < pParticles.size(); ++i) {

		if (!rParticles[i].isBeingDrawn || !rParticles[i].isBeingDrawn) {
			continue;
		}

		rParticles[i].spawnCorrectIter++;

		int xIdx = static_cast<int>((pParticles[i].pos.x - 0) / cellSize);
		int yIdx = static_cast<int>((pParticles[i].pos.y - 0) / cellSize);

		if (xIdx >= 0 && xIdx < cellAmountX &&
			yIdx >= 0 && yIdx < cellAmountY) {

			int cellId = xIdx + yIdx * cellAmountX;
			cellList[cellId].push_back(i);
		}
	}

	//std::vector<std::mutex> particleLocks(pParticles.size());


	auto checkCollision = [&](size_t a, size_t b) {

		if (a == b) return;

		float r = 1.3f; // <- Heuristic

		glm::vec2 delta = pParticles[a].pos - pParticles[b].pos;
		float distSq = delta.x * delta.x + delta.y * delta.y;
		float sumR = r + r;

		if (distSq < sumR * sumR) {

			if (a < b) {
				// std::scoped_lock lock(particleLocks[a], particleLocks[b]);
				physics.collisions(pParticles[a], pParticles[b],
					rParticles[a], rParticles[b], r);
			}
			else {
				//std::scoped_lock lock(particleLocks[b], particleLocks[a]);
				physics.collisions(pParticles[a], pParticles[b],
					rParticles[a], rParticles[b], r);
			}
		}
		};


#pragma omp parallel for collapse(2) schedule(dynamic)
	for (int y = 0; y < cellAmountY; ++y) {
		for (int x = 0; x < cellAmountX; ++x) {
			int baseId = x + y * cellAmountX;
			auto& cell = cellList[baseId];

			for (int dx = -1; dx <= 1; ++dx) {
				for (int dy = -1; dy <= 1; ++dy) {
					int nx = x + dx, ny = y + dy;
					if (nx < 0 || ny < 0 || nx >= cellAmountX || ny >= cellAmountY)
						continue;

					int neighborId = nx + ny * cellAmountX;
					auto& other = cellList[neighborId];

					if (neighborId == baseId) {

						for (size_t i = 0; i < cell.size(); ++i) {
							for (size_t j = i + 1; j < cell.size(); ++j) {
								checkCollision(cell[i], cell[j]);
							}
						}
					}
					else {

						for (size_t i : cell) {
							for (size_t j : other) {
								checkCollision(i, j);
							}
						}
					}
				}
			}
		}
	}
}
