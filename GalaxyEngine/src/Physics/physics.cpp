#include "../../include/Physics/physics.h"
#include "omp.h"

Vector2 Physics::calculateForceFromGrid(const Quadtree& grid, std::vector<ParticlePhysics>& pParticles, UpdateVariables& myVar, ParticlePhysics& pParticle) {
	Vector2 totalForce = { 0.0f, 0.0f };

	if (grid.gridMass <= 0)
		return totalForce;

	float dx = grid.centerOfMass.x - pParticle.pos.x;
	float dy = grid.centerOfMass.y - pParticle.pos.y;

	if (myVar.isPeriodicBoundaryEnabled) {
		dx -= myVar.domainWidth * ((dx > myVar.halfDomainWidth) - (dx < -myVar.halfDomainWidth));
		dy -= myVar.domainHeight * ((dy > myVar.halfDomainHeight) - (dy < -myVar.halfDomainHeight));
	}

	float distanceSq = dx * dx + dy * dy + myVar.softening * myVar.softening;

	if ((grid.size * grid.size < (myVar.theta * myVar.theta) * distanceSq) || grid.subGrids.empty()) {
		if ((grid.endIndex - grid.startIndex) == 1 &&
			fabs(pParticles[grid.startIndex].pos.x - pParticle.pos.x) < 0.001f &&
			fabs(pParticles[grid.startIndex].pos.y - pParticle.pos.y) < 0.001f) {
			return totalForce;
		}

		float invDistance = 1.0f / sqrt(distanceSq);
		float forceMagnitude = static_cast<float>(myVar.G) * pParticle.mass * grid.gridMass * invDistance * invDistance * invDistance;
		totalForce.x = dx * forceMagnitude;
		totalForce.y = dy * forceMagnitude;
	}
	else {
		for (const auto& subGridPtr : grid.subGrids) {
			Vector2 childForce = calculateForceFromGrid(*subGridPtr, pParticles, myVar, pParticle);
			totalForce.x += childForce.x;
			totalForce.y += childForce.y;
		}
	}
	return totalForce;
}

void Physics::pairWiseGravity(std::vector<ParticlePhysics>& pParticles, UpdateVariables& myVar) {
#pragma omp parallel for schedule(dynamic)
	for (size_t i = 0; i < pParticles.size(); ++i) {
		for (size_t j = i + 1; j < pParticles.size(); ++j) {
			ParticlePhysics& pParticleA = pParticles[i];
			ParticlePhysics& pParticleB = pParticles[j];

			float accelPlanetAX = 0;
			float accelPlanetAY = 0;

			float prevAccAX = accelPlanetAX;
			float prevAccAY = accelPlanetAY;

			float accelPlanetBX = 0;
			float accelPlanetBY = 0;

			float prevAccBX = accelPlanetBX;
			float prevAccBY = accelPlanetBY;

			float dx = pParticleB.pos.x - pParticleA.pos.x;
			float dy = pParticleB.pos.y - pParticleA.pos.y;

			if (myVar.isPeriodicBoundaryEnabled) {
				if (myVar.isPeriodicBoundaryEnabled) {
					if (dx > myVar.domainWidth / 2)
						dx -= myVar.domainWidth;
					else if (dx < -myVar.domainWidth / 2)
						dx += myVar.domainWidth;

					if (dy > myVar.domainHeight / 2)
						dy -= myVar.domainHeight;
					else if (dy < -myVar.domainHeight / 2)
						dy += myVar.domainHeight;
				}
			}

			float distanceSq = dx * dx + dy * dy + myVar.softening * myVar.softening;

			float distance = sqrt(distanceSq);
			float force = static_cast<float>(myVar.G * pParticleA.mass * pParticleB.mass / distanceSq);

			float fx = (dx / distance) * force;
			float fy = (dy / distance) * force;

			accelPlanetAX = fx / pParticleA.mass;
			accelPlanetAY = fy / pParticleA.mass;


			accelPlanetBX = fx / pParticleB.mass;
			accelPlanetBY = fy / pParticleB.mass;

			pParticleA.velocity.x += (myVar.fixedDeltaTime * ((3.0f / 2.0f)) * accelPlanetAX - ((1.0f / 2.0f)) * prevAccAX) * myVar.timeStepMultiplier;
			pParticleA.velocity.y += (myVar.fixedDeltaTime * ((3.0f / 2.0f)) * accelPlanetAY - ((1.0f / 2.0f)) * prevAccAY) * myVar.timeStepMultiplier;

			pParticleB.velocity.x -= (myVar.fixedDeltaTime * ((3.0f / 2.0f)) * accelPlanetBX - ((1.0f / 2.0f)) * prevAccBX) * myVar.timeStepMultiplier;
			pParticleB.velocity.y -= (myVar.fixedDeltaTime * ((3.0f / 2.0f)) * accelPlanetBY - ((1.0f / 2.0f)) * prevAccBY) * myVar.timeStepMultiplier;

		}
	}
}

void Physics::physicsUpdate(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, UpdateVariables& myVar) {
	if (myVar.isPeriodicBoundaryEnabled) {
		for (size_t i = 0; i < pParticles.size(); i++) {
			ParticlePhysics& pParticle = pParticles[i];

			pParticle.velocity.x += (myVar.timeFactor * (1.5f * pParticle.acc.x));
			pParticle.velocity.y += (myVar.timeFactor * (1.5f * pParticle.acc.y));

			pParticle.pos.x += pParticle.velocity.x * myVar.timeFactor;
			pParticle.pos.y += pParticle.velocity.y * myVar.timeFactor;

			if (pParticle.pos.x < 0) pParticle.pos.x += myVar.domainWidth;
			else if (pParticle.pos.x >= myVar.domainWidth) pParticle.pos.x -= myVar.domainWidth;

			if (pParticle.pos.y < 0) pParticle.pos.y += myVar.domainHeight;
			else if (pParticle.pos.y >= myVar.domainHeight) pParticle.pos.y -= myVar.domainHeight;
		}
	}
	else {
		for (size_t i = 0; i < pParticles.size(); i++) {
			ParticlePhysics& pParticle = pParticles[i];

			pParticle.velocity.x += (myVar.timeFactor * (1.5f * pParticle.acc.x));
			pParticle.velocity.y += (myVar.timeFactor * (1.5f * pParticle.acc.y));

			pParticle.pos.x += pParticle.velocity.x * myVar.timeFactor;
			pParticle.pos.y += pParticle.velocity.y * myVar.timeFactor;

			if (pParticles[i].pos.x < 0 || pParticles[i].pos.x >= myVar.domainWidth || pParticles[i].pos.y < 0 || pParticles[i].pos.y >= myVar.domainHeight) {
				pParticles.erase(pParticles.begin() + i);
				rParticles.erase(rParticles.begin() + i);

			}
		}
	}
}

void Physics::collisions(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, float& softening, float& particleTextureHalfSize) {

#pragma omp parallel for schedule(dynamic)
	for (size_t i = 0; i < pParticles.size(); i++) {

		ParticlePhysics& pParticleA = pParticles[i];

		for (size_t j = i + 1; j < pParticles.size(); j++) {
			ParticlePhysics& pParticleB = pParticles[j];

			float dx = pParticleA.pos.x - pParticleB.pos.x;
			float dy = pParticleA.pos.y - pParticleB.pos.y;
			float distanceSq = dx * dx + dy * dy + softening * softening;

			float radiiSum = rParticles[i].size * particleTextureHalfSize + rParticles[j].size * particleTextureHalfSize;

			Vector2 relativeVel = pParticleA.velocity - pParticleB.velocity;

			Vector2 collisionNormal = Vector2Normalize(pParticleA.pos - pParticleB.pos);
			float velocityNormal = Vector2DotProduct(relativeVel, collisionNormal);


			if (velocityNormal <= 0) {

				if (distanceSq < radiiSum * radiiSum) {

					float e = 0.6f;

					float invMassA = 1.0f / pParticleA.mass;
					float invMassB = 1.0f / pParticleB.mass;

					float force = -(1.0f + e) * velocityNormal;
					force /= (invMassA + invMassB);

					Vector2 impulse = Vector2Scale(collisionNormal, force);

					Vector2 finalForceA = { impulse.x * invMassA, impulse.y * invMassA };
					Vector2 finalForceB = { impulse.x * invMassB, impulse.y * invMassB };

					pParticleA.velocity.x += finalForceA.x;
					pParticleA.velocity.y += finalForceA.y;

					pParticleB.velocity.x -= finalForceB.x;
					pParticleB.velocity.y -= finalForceB.y;

					float penetration = radiiSum - sqrt(distanceSq);
					const float percent = 0.8f;
					const float slop = 0.01f;

					if (penetration > slop) {
						float correctionMagnitude = (penetration / (invMassA + invMassB)) * percent;
						Vector2 correction = Vector2Scale(collisionNormal, correctionMagnitude);

						pParticleA.pos.x += correction.x * invMassA;
						pParticleA.pos.y += correction.y * invMassA;
						pParticleB.pos.x -= correction.x * invMassB;
						pParticleB.pos.y -= correction.y * invMassB;
					}
				}
			}
		}
	}
}
