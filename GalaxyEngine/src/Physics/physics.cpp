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
			float gridAverageTemp = grid.gridTemp / (grid.endIndex - grid.startIndex);

			float temperatureDifference = gridAverageTemp - pParticle.temp;

			float distance = sqrt(distanceSq);
			float heatTransfer = myVar.globalHeatConductivity * temperatureDifference / distance;

			pParticle.temp += heatTransfer * myVar.timeFactor;
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

void Physics::createConstraints(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, bool& constraintCreateSpecialFlag,
	UpdateVariables& myVar) {

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

		for (uint32_t& id : pParticles[i].neighborIds) {
			auto it = NeighborSearch::idToIndex.find(id);
			if (it == NeighborSearch::idToIndex.end()) continue;

			size_t neighborIndex = it->second;
			if (neighborIndex == i) continue;

			ParticlePhysics& pj = pParticles[neighborIndex];

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
	}

	if (myVar.deleteSelectedConstraints) {
		constraintMap.clear();
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

			if ((rParticles[it1->second].isSelected || rParticles[it2->second].isSelected) && myVar.deleteSelectedConstraints) {
				constraint.isBroken = true;
			}
		}
		myVar.deleteSelectedConstraints = false;
	}

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

				if ((rParticles[it1->second].isSelected || rParticles[it2->second].isSelected) && myVar.deleteSelectedConstraints) {
					constraint.isBroken = true;
				}

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

				if (myVar.isPeriodicBoundaryEnabled) {
					delta.x = fmod(delta.x + myVar.domainSize.x * 1.5f, myVar.domainSize.x) - myVar.domainSize.x * 0.5f;
					delta.y = fmod(delta.y + myVar.domainSize.y * 1.5f, myVar.domainSize.y) - myVar.domainSize.y * 0.5f;
				}

				float currentLength = glm::length(delta);
				if (currentLength < 0.0001f) continue;

				glm::vec2 dir = delta / currentLength;
				constraint.displacement = currentLength - constraint.restLength;

				if (!myVar.unbreakableConstraints) {
					// This actually uses a percentage of the rest length as reference. More intuitive than arbitrary numbers IMO
					// The percentage in this case is constraint.hardness which is normalized from 0 - 1
					if (pMatI && pMatJ) {
						if (!pMatI->isPlastic || !pMatJ->isPlastic) {

							constraint.isPlastic = false;

							if (constraint.displacement >= (constraint.resistance * myVar.globalConstraintResistance) * constraint.restLength ||
								constraint.displacement <= -((constraint.resistance * myVar.globalConstraintResistance) * constraint.restLength)) {
								constraint.isBroken = true;
							}
						}
						else {

							constraint.isPlastic = true;

							if (constraint.displacement >= constraint.plasticityPoint * constraint.originalLength ||
								constraint.displacement <= -(constraint.plasticityPoint * constraint.originalLength)) {

								constraint.restLength += constraint.displacement;

							}

							if (constraint.restLength >= (constraint.originalLength + constraint.originalLength *
								(constraint.resistance * myVar.globalConstraintResistance)) *
								(pMatI->constraintPlasticPointMult + pMatJ->constraintPlasticPointMult) * 0.5f ||

								constraint.restLength <= -(constraint.originalLength + constraint.originalLength *
									(constraint.resistance * myVar.globalConstraintResistance) *
									(pMatI->constraintPlasticPointMult + pMatJ->constraintPlasticPointMult) * 0.5f)) {

								constraint.isBroken = true;
							}
						}
					}

					if (pMatI && pMatJ) {
						if (pi.isHotPoint || pj.isHotPoint) {
							constraint.isBroken = true;
						}
					}
				}

				if (myVar.timeFactor > 0.0f && myVar.gridExists) {
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
				if (pParticles[i].pos.x <= 0.0f || pParticles[i].pos.x >= myVar.domainSize.x || pParticles[i].pos.y <= 0.0f || pParticles[i].pos.y >= myVar.domainSize.y) {
					std::swap(pParticles[i], pParticles.back());
					std::swap(rParticles[i], rParticles.back());

					pParticles.pop_back();
					rParticles.pop_back();
				}
				else {
					i++;
				}
			}
			else {
				i++;
			}
			
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
	for (int x = 0; x < cellAmountX; ++x) {
		for (int y = 0; y < cellAmountY; ++y) {
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
