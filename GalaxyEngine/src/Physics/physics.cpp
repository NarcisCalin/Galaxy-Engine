#include "Physics/physics.h"

glm::vec2 Physics::calculateForceFromGrid(const Quadtree& grid, std::vector<ParticlePhysics>& pParticles, UpdateVariables& myVar, ParticlePhysics& pParticle) {
	glm::vec2 totalForce = { 0.0f, 0.0f };

	if (grid.gridMass <= 0)
		return totalForce;

	glm::vec2 d = grid.centerOfMass - pParticle.pos;

	if (myVar.isPeriodicBoundaryEnabled) {
		d.x -= myVar.domainSize.x * ((d.x > myVar.halfDomainWidth) - (d.x < -myVar.halfDomainWidth));
		d.y -= myVar.domainSize.y * ((d.y > myVar.halfDomainHeight) - (d.y < -myVar.halfDomainHeight));
	}

	float distanceSq = d.x * d.x + d.y * d.y + myVar.softening * myVar.softening;

	if ((grid.size * grid.size < (myVar.theta * myVar.theta) * distanceSq) || grid.subGrids.empty()) {
		if ((grid.endIndex - grid.startIndex) == 1 &&
			fabs(pParticles[grid.startIndex].pos.x - pParticle.pos.x) < 0.001f &&
			fabs(pParticles[grid.startIndex].pos.y - pParticle.pos.y) < 0.001f) {


			return totalForce;
		}

		float invDistance = 1.0f / sqrt(distanceSq);
		float forceMagnitude = static_cast<float>(myVar.G) * pParticle.mass * grid.gridMass * invDistance * invDistance * invDistance;
		totalForce = d * forceMagnitude;

		// Heat transfer
		if (myVar.isTempEnabled) {
			float temperatureDifference = grid.gridTemp / (grid.endIndex - grid.startIndex) - pParticle.temp;

			float heatTransfer = myVar.globalHeatConductivity * temperatureDifference * invDistance * invDistance;

			pParticle.temp += heatTransfer;
		}
	}
	else {
		for (const auto& subGridPtr : grid.subGrids) {
			glm::vec2 childForce = calculateForceFromGrid(*subGridPtr, pParticles, myVar, pParticle);
			totalForce += childForce;
		}
	}
	return totalForce;
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

void Physics::createConstraints(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, bool& constraintAllSolids) {

	for (size_t i = 0; i < pParticles.size(); i++) {

		ParticlePhysics& pi = pParticles[i];

		SPHMaterial* pMatI = nullptr;
		auto matItI = SPHMaterials::idToMaterial.find(rParticles[i].sphLabel);
		if (matItI != SPHMaterials::idToMaterial.end()) {
			pMatI = matItI->second;
		}

		if (IO::shortcutPress(KEY_P) || constraintAllSolids) {
			if (pMatI) {
				if (pMatI->coldPoint == 0.0f) {
					if (pi.temp >= pMatI->hotPoint) {
						continue;
					}
				}
				else {
					if (pi.temp >= pMatI->coldPoint) {
						continue;
					}
				}
			}
		}
		else {
			if (!pi.hasSolidified) {
				continue;
			}
			else {
				pi.hasSolidified = false;
			}
		}

		for (uint32_t& id : pParticles[i].neighborIds) {
			auto it = NeighborSearch::idToIndex.find(id);
			if (it != NeighborSearch::idToIndex.end()) {
				size_t neighborIndex = it->second;

				if (neighborIndex == i) continue;

				auto& pj = pParticles[neighborIndex];

				SPHMaterial* pMatJ = nullptr;
				auto matItJ = SPHMaterials::idToMaterial.find(rParticles[neighborIndex].sphLabel);
				if (matItJ != SPHMaterials::idToMaterial.end()) {
					pMatJ = matItJ->second;
				}

				/*if (pj.temp >= pMatJ->hotPoint) {
					continue;
				}*/

				// This code avoids creating a new constraint if there already is one that includes both particles
				uint64_t key = makeKey(pi.id, pj.id);
				if (constraintMap.find(key) != constraintMap.end()) {
					continue;
				}

				float hardness = 0.6f;
				if (pMatI && pMatJ) {
					hardness = (pMatI->constraintHardness + pMatJ->constraintHardness) * 0.5f;
				}

				if (pi.id < pj.id) {

					float currentDist = glm::distance(pi.pos, pj.pos);
					bool broken = false;
					particleConstraints.push_back({ pi.id, pj.id, currentDist, globalStiffness, hardness, broken });
					constraintMap[key] = &particleConstraints.back();
				}
			}
		}
	}
	constraintAllSolids = false;
}

void Physics::constraints(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles,
	bool& isPeriodicBoundaryEnabled, glm::vec2& domainSize) {

	if (!particleConstraints.empty()) {
		const int substeps = 15;

		auto new_end = std::remove_if(particleConstraints.begin(), particleConstraints.end(),
			[](const auto& constraint) {
				auto it1 = NeighborSearch::idToIndex.find(constraint.id1);
				auto it2 = NeighborSearch::idToIndex.find(constraint.id2);
				return it1 == NeighborSearch::idToIndex.end() ||
					it2 == NeighborSearch::idToIndex.end() ||
					constraint.isBroken;
			});

		for (auto it = new_end; it != particleConstraints.end(); ++it) {
			constraintMap.erase(makeKey(it->id1, it->id2));
		}

		particleConstraints.erase(new_end, particleConstraints.end());

		for (int step = 0; step < substeps; step++) {

#pragma omp parallel for schedule(dynamic)
			for (size_t i = 0; i < particleConstraints.size(); i++) {
				auto& constraint = particleConstraints[i];

				auto it1 = NeighborSearch::idToIndex.find(constraint.id1);
				auto it2 = NeighborSearch::idToIndex.find(constraint.id2);
				if (it1 == NeighborSearch::idToIndex.end() ||
					it2 == NeighborSearch::idToIndex.end()) {
					constraint.isBroken = true;
					continue;
				}

				ParticlePhysics& pi = pParticles[it1->second];
				ParticlePhysics& pj = pParticles[it2->second];

				SPHMaterial* pMatI = nullptr;
				auto matItI = SPHMaterials::idToMaterial.find(rParticles[it1->second].sphLabel);
				if (matItI != SPHMaterials::idToMaterial.end()) {
					pMatI = matItI->second;
				}

				SPHMaterial* pMatJ = nullptr;
				auto matItJ = SPHMaterials::idToMaterial.find(rParticles[it2->second].sphLabel);
				if (matItJ != SPHMaterials::idToMaterial.end()) {
					pMatJ = matItJ->second;
				}

				glm::vec2 delta = pj.pos - pi.pos;

				if (isPeriodicBoundaryEnabled) {
					delta.x = fmod(delta.x + domainSize.x * 1.5f, domainSize.x) - domainSize.x * 0.5f;
					delta.y = fmod(delta.y + domainSize.y * 1.5f, domainSize.y) - domainSize.y * 0.5f;
				}

				float currentLength = glm::length(delta);
				if (currentLength < 0.0001f) continue;

				glm::vec2 dir = delta / currentLength;
				float displacement = currentLength - constraint.restLength;

				if (displacement > constraint.hardness || displacement < -constraint.hardness) {
					constraint.isBroken = true;
				}

				if (pMatI && pMatJ) {
					if (pi.isHotPoint || pj.isHotPoint) {
						constraint.isBroken = true;
					}
				}

				glm::vec2 springForce = constraintStiffness * displacement * dir * pi.mass;
				glm::vec2 relVel = pj.vel - pi.vel;
				glm::vec2 dampForce = -constraintDamping * glm::dot(relVel, dir) * dir * pi.mass;
				glm::vec2 totalForce = springForce + dampForce;

#pragma omp atomic
				pi.acc.x += totalForce.x / pi.mass;
#pragma omp atomic
				pi.acc.y += totalForce.y / pi.mass;
#pragma omp atomic
				pj.acc.x -= totalForce.x / pj.mass;
#pragma omp atomic
				pj.acc.y -= totalForce.y / pj.mass;

				float correctionFactor = constraintStiffness * stiffCorrectionRatio;
				glm::vec2 correction = dir * displacement * correctionFactor;
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

void Physics::physicsUpdate(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, UpdateVariables& myVar, bool& sphGround) {
	if (myVar.isPeriodicBoundaryEnabled) {
		for (size_t i = 0; i < pParticles.size(); i++) {
			ParticlePhysics& pParticle = pParticles[i];

			pParticle.prevVel = pParticle.vel;

			pParticle.vel += myVar.timeFactor * 1.5f * pParticle.acc;

			// Max velocity for SPH
			if (myVar.isSPHEnabled) {
				const float sphMaxVelSq = myVar.sphMaxVel * myVar.sphMaxVel;
				float vSq = pParticle.vel.x * pParticle.vel.x + pParticle.vel.y * pParticle.vel.y;
				float prevVSq = pParticle.prevVel.x * pParticle.prevVel.x + pParticle.prevVel.y * pParticle.prevVel.y;
				if (vSq > sphMaxVelSq) {
					float invPrevLen = myVar.sphMaxVel / sqrtf(prevVSq);
					float invLen = myVar.sphMaxVel / sqrtf(vSq);
					pParticle.prevVel *= invPrevLen;
					pParticle.vel *= invLen;
				}
			}

			pParticle.pos += pParticle.vel * myVar.timeFactor;

			if (!sphGround) {
				if (pParticle.pos.x < 0)
					pParticle.pos.x += myVar.domainSize.x;
				else if (pParticle.pos.x >= myVar.domainSize.x)
					pParticle.pos.x -= myVar.domainSize.x;

				if (pParticle.pos.y < 0)
					pParticle.pos.y += myVar.domainSize.y;
				else if (pParticle.pos.y >= myVar.domainSize.y)
					pParticle.pos.y -= myVar.domainSize.y;
			}
		}
	}
	else {
		for (size_t i = 0; i < pParticles.size(); ) {
			ParticlePhysics& pParticle = pParticles[i];

			pParticle.prevVel = pParticle.vel;

			pParticle.vel += myVar.timeFactor * 1.5f * pParticle.acc;

			// Max velocity for SPH
			if (myVar.isSPHEnabled) {
				const float sphMaxVelSq = myVar.sphMaxVel * myVar.sphMaxVel;
				float vSq = pParticle.vel.x * pParticle.vel.x + pParticle.vel.y * pParticle.vel.y;
				float prevVSq = pParticle.prevVel.x * pParticle.prevVel.x + pParticle.prevVel.y * pParticle.prevVel.y;
				if (vSq > sphMaxVelSq) {
					float invPrevLen = myVar.sphMaxVel / sqrtf(prevVSq);
					float invLen = myVar.sphMaxVel / sqrtf(vSq);
					pParticle.prevVel *= invPrevLen;
					pParticle.vel *= invLen;
				}
			}

			pParticle.pos += pParticle.vel * myVar.timeFactor;

			if (!sphGround) {
				if (pParticles[i].pos.x < 0 || pParticles[i].pos.x >= myVar.domainSize.x || pParticles[i].pos.y < 0 || pParticles[i].pos.y >= myVar.domainSize.y) {
					std::swap(pParticles[i], pParticles.back());
					std::swap(rParticles[i], rParticles.back());

					pParticles.pop_back();
					rParticles.pop_back();
				}
				else {
					i++;
				}
			}
		}
	}
}

void Physics::collisions(ParticlePhysics& pParticleA, ParticlePhysics& pParticleB,
	ParticleRendering& rParticleA, ParticleRendering& rParticleB, UpdateVariables& myVar, float& dt) {

	if (rParticleA.isDarkMatter || rParticleB.isDarkMatter) {
		return;
	}

	ParticlePhysics& pA = pParticleA;
	ParticlePhysics& pB = pParticleB;

	glm::vec2 posA = pA.pos;
	glm::vec2 posB = pB.pos;

	glm::vec2 d = posB - posA;
	float distanceSq = d.x * d.x + d.y * d.y;

	float radiiSum = rParticleA.size * myVar.particleTextureHalfSize + rParticleB.size * myVar.particleTextureHalfSize;
	float radiiSumSq = radiiSum * radiiSum;


	if (distanceSq > radiiSumSq || distanceSq <= 1e-6f) {
		return;
	}

	glm::vec2 velA = pA.vel;
	glm::vec2 velB = pB.vel;
	glm::vec2 relativeVel = velB - velA;

	float velocityNormal = glm::dot(d, relativeVel);

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


	if (velocityNormal >= 0.0f) {
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
	pB.pos += pB.vel * t;
}
