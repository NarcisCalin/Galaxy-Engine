#include "../../include/Physics/physics.h"
#include "omp.h"

Vector2 Physics::calculateForceFromGrid(const Quadtree& grid, std::vector<ParticlePhysics>& pParticles, UpdateVariables& myVar, ParticlePhysics& pParticle) {
	Vector2 totalForce = { 0.0f, 0.0f };

	if (grid.gridMass <= 0)
		return totalForce;

	float dx = grid.centerOfMass.x - pParticle.pos.x;
	float dy = grid.centerOfMass.y - pParticle.pos.y;

	if (myVar.isPeriodicBoundaryEnabled) {
		dx -= myVar.domainSize.x * ((dx > myVar.halfDomainWidth) - (dx < -myVar.halfDomainWidth));
		dy -= myVar.domainSize.y * ((dy > myVar.halfDomainHeight) - (dy < -myVar.halfDomainHeight));
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
					if (dx > myVar.domainSize.x / 2)
						dx -= myVar.domainSize.x;
					else if (dx < -myVar.domainSize.x / 2)
						dx += myVar.domainSize.x;

					if (dy > myVar.domainSize.y / 2)
						dy -= myVar.domainSize.y;
					else if (dy < -myVar.domainSize.y / 2)
						dy += myVar.domainSize.y;
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

			pParticleA.vel.x += (myVar.fixedDeltaTime * ((3.0f / 2.0f)) * accelPlanetAX - ((1.0f / 2.0f)) * prevAccAX) * myVar.timeStepMultiplier;
			pParticleA.vel.y += (myVar.fixedDeltaTime * ((3.0f / 2.0f)) * accelPlanetAY - ((1.0f / 2.0f)) * prevAccAY) * myVar.timeStepMultiplier;

			pParticleB.vel.x -= (myVar.fixedDeltaTime * ((3.0f / 2.0f)) * accelPlanetBX - ((1.0f / 2.0f)) * prevAccBX) * myVar.timeStepMultiplier;
			pParticleB.vel.y -= (myVar.fixedDeltaTime * ((3.0f / 2.0f)) * accelPlanetBY - ((1.0f / 2.0f)) * prevAccBY) * myVar.timeStepMultiplier;

		}
	}
}

void Physics::physicsUpdate(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, UpdateVariables& myVar) {
	if (myVar.isPeriodicBoundaryEnabled) {
		for (size_t i = 0; i < pParticles.size(); i++) {
			ParticlePhysics& pParticle = pParticles[i];

			pParticle.vel.x += myVar.timeFactor * (1.5f * pParticle.acc.x);
			pParticle.vel.y += myVar.timeFactor * (1.5f * pParticle.acc.y);

			pParticle.pos.x += pParticle.vel.x * myVar.timeFactor;
			pParticle.pos.y += pParticle.vel.y * myVar.timeFactor;

			if (pParticle.pos.x < 0) pParticle.pos.x += myVar.domainSize.x;
			else if (pParticle.pos.x >= myVar.domainSize.x) pParticle.pos.x -= myVar.domainSize.x;

			if (pParticle.pos.y < 0) pParticle.pos.y += myVar.domainSize.y;
			else if (pParticle.pos.y >= myVar.domainSize.y) pParticle.pos.y -= myVar.domainSize.y;
		}
	}
	else {
		for (size_t i = 0; i < pParticles.size(); i++) {
			ParticlePhysics& pParticle = pParticles[i];

			pParticle.vel.x += myVar.timeFactor * (1.5f * pParticle.acc.x);
			pParticle.vel.y += myVar.timeFactor * (1.5f * pParticle.acc.y);

			pParticle.pos.x += pParticle.vel.x * myVar.timeFactor;
			pParticle.pos.y += pParticle.vel.y * myVar.timeFactor;

			if (pParticles[i].pos.x < 0 || pParticles[i].pos.x >= myVar.domainSize.x || pParticles[i].pos.y < 0 || pParticles[i].pos.y >= myVar.domainSize.y) {
				pParticles.erase(pParticles.begin() + i);
				rParticles.erase(rParticles.begin() + i);

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

	Vector2 posA = pA.pos;
	Vector2 posB = pB.pos;

	Vector2 d = posB - posA;
	float distanceSq = d.x * d.x + d.y * d.y;

	float radiiSum = rParticleA.size * myVar.particleTextureHalfSize + rParticleB.size * myVar.particleTextureHalfSize;
	float radiiSumSq = radiiSum * radiiSum;


	if (distanceSq > radiiSumSq || distanceSq <= 1e-6f) {
		return;
	}

	Vector2 velA = pA.vel;
	Vector2 velB = pB.vel;
	Vector2 relativeVel = velB - velA;

	float velocityNormal = Vector2DotProduct(d, relativeVel);

	float weightA = pB.mass / (pA.mass + pB.mass);
	float weightB = pA.mass / (pA.mass + pB.mass);

	float distance = sqrt(distanceSq);

	float penetration = radiiSum - distance;
	const float percent = 0.8f;
	const float slop = 0.01f;
	float correctionMag = std::max(penetration - slop, 0.0f) * percent;

	Vector2 normal = d / distance;
	Vector2 correction = normal * correctionMag;

	pA.pos.x -= weightA * correction.x;
	pA.pos.y -= weightA * correction.y;

	pB.pos.x += weightB * correction.x;
	pB.pos.y += weightB * correction.y;


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

	float impulseNumerator = -(1.0f + myVar.particleBounciness) * Vector2DotProduct(relativeVel, normal);
	float impulseDenominator = (1.0f / pA.mass + 1.0f / pB.mass);
	float j = impulseNumerator / impulseDenominator;

	Vector2 impulse = normal * j;

	pA.vel = velA - impulse / pA.mass;
	pB.vel = velB + impulse / pB.mass;


	pA.pos += pA.vel * t;
	pB.pos += pB.vel * t;
}
