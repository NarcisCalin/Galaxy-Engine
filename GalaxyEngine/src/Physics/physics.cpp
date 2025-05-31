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
	}
	else {
		for (const auto& subGridPtr : grid.subGrids) {
			glm::vec2 childForce = calculateForceFromGrid(*subGridPtr, pParticles, myVar, pParticle);
			totalForce += childForce;
		}
	}
	return totalForce;
}

void Physics::physicsUpdate(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, UpdateVariables& myVar, bool& sphGround) {
	if (myVar.isPeriodicBoundaryEnabled) {
		for (size_t i = 0; i < pParticles.size(); i++) {
			ParticlePhysics& pParticle = pParticles[i];

			pParticle.vel += myVar.timeFactor * 1.5f * pParticle.acc;

			// Max velocity for SPH
			if (myVar.isSPHEnabled) {
				const float sphMaxVelSq = myVar.sphMaxVel * myVar.sphMaxVel;
				float vSq = pParticle.vel.x * pParticle.vel.x + pParticle.vel.y * pParticle.vel.y;
				if (vSq > sphMaxVelSq) {
					float invLen = myVar.sphMaxVel / sqrtf(vSq);
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
		for (size_t i = 0; i < pParticles.size(); i++) {
			ParticlePhysics& pParticle = pParticles[i];

			pParticle.vel += myVar.timeFactor * 1.5f * pParticle.acc;

			pParticle.pos += pParticle.vel * myVar.timeFactor;

			if (!sphGround) {
				if (pParticles[i].pos.x < 0 || pParticles[i].pos.x >= myVar.domainSize.x || pParticles[i].pos.y < 0 || pParticles[i].pos.y >= myVar.domainSize.y) {
					pParticles.erase(pParticles.begin() + i);
					rParticles.erase(rParticles.begin() + i);
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
