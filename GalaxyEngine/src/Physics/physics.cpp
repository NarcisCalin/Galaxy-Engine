#include "../../include/Physics/physics.h"
#include "omp.h"

Vector2 Physics::calculateForceFromGrid(const Quadtree& grid, std::vector<ParticlePhysics>& pParticles, UpdateVariables& myVar, ParticlePhysics& pParticle) {
	Vector2 totalForce = { 0.0f, 0.0f };

	if (grid.gridMass <= 0)
		return totalForce;

	float dx = grid.centerOfMass.x - pParticle.pos.x;
	float dy = grid.centerOfMass.y - pParticle.pos.y;

	if (myVar.isPeriodicBoundaryEnabled) {
		dx -= myVar.screenWidth * ((dx > myVar.halfScreenWidth) - (dx < -myVar.halfScreenWidth));
		dy -= myVar.screenHeight * ((dy > myVar.halfScreenHeight) - (dy < -myVar.halfScreenHeight));
	}

	float distanceSq = dx * dx + dy * dy + myVar.softening * myVar.softening;

	if ((grid.size * grid.size < myVar.theta * myVar.theta * distanceSq) || grid.subGrids.empty()) {
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

Vector2 Physics::darkMatterForce(const ParticlePhysics& pParticle, UpdateVariables& myVar) {
	float centerX = myVar.screenWidth / 2.0f;
	float centerY = myVar.screenHeight / 2.0f;

	float dx = pParticle.pos.x - centerX;
	float dy = pParticle.pos.y - centerY;
	float radius = sqrt(dx * dx + dy * dy);
	if (radius < 1.0f) radius = 1.0f;

	const double haloMass = 7e17;
	const float haloRadius = 650.0;

	float concentration = 10;
	float r_ratio = radius / haloRadius;
	float M_enclosed = static_cast<float>(haloMass * (log(1 + r_ratio) - r_ratio / (1 + r_ratio)))
		/ (log(1 + concentration) - concentration / (1 + concentration));

	float acceleration = static_cast<float>(myVar.G * M_enclosed) / (radius * radius);

	Vector2 force;
	force.x = static_cast<float>(-(dx / radius) * acceleration * pParticle.mass);
	force.y = static_cast<float>(-(dy / radius) * acceleration * pParticle.mass);

	return force;
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
					if (dx > myVar.screenWidth / 2)
						dx -= myVar.screenWidth;
					else if (dx < -myVar.screenWidth / 2)
						dx += myVar.screenWidth;

					if (dy > myVar.screenHeight / 2)
						dy -= myVar.screenHeight;
					else if (dy < -myVar.screenHeight / 2)
						dy += myVar.screenHeight;
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
			pParticle.pos.x += pParticle.velocity.x * myVar.timeFactor;
			pParticle.pos.y += pParticle.velocity.y * myVar.timeFactor;
			if (pParticle.pos.x < 0) pParticle.pos.x += myVar.screenWidth;
			else if (pParticle.pos.x >= myVar.screenWidth) pParticle.pos.x -= myVar.screenWidth;

			if (pParticle.pos.y < 0) pParticle.pos.y += myVar.screenHeight;
			else if (pParticle.pos.y >= myVar.screenHeight) pParticle.pos.y -= myVar.screenHeight;
		}
	}
	else {
		for (size_t i = 0; i < pParticles.size(); i++) {
			ParticlePhysics& pParticle = pParticles[i];
			pParticle.pos.x += pParticle.velocity.x * myVar.timeFactor;
			pParticle.pos.y += pParticle.velocity.y * myVar.timeFactor;
			if (pParticles[i].pos.x < 0 || pParticles[i].pos.x >= myVar.screenWidth || pParticles[i].pos.y < 0 || pParticles[i].pos.y >= myVar.screenHeight) {
				pParticles.erase(pParticles.begin() + i);
				rParticles.erase(rParticles.begin() + i);

			}
		}
	}
}

void Physics::collisions(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, float& softening) {
	size_t n = pParticles.size();
#pragma omp parallel for schedule(dynamic)
	for (size_t i = 0; i < n; i++) {
		ParticlePhysics& a = pParticles[i];
		for (size_t j = i + 1; j < n; j++) {
			ParticlePhysics& b = pParticles[j];
			float dx = a.pos.x - b.pos.x;
			float dy = a.pos.y - b.pos.y;
			float distSq = dx * dx + dy * dy + softening * softening;
			// TODO: MAYBE PRECOMPUTE radiiSum
			float radiiSum = rParticles[i].size * 16.0f + rParticles[j].size * 16.0f;
			float rsq = radiiSum * radiiSum;
			if (distSq > rsq) continue;

			float distance = std::sqrt(distSq);
			if (distance == 0.0f) distance = 0.001f;
			float normalX = dx / distance;
			float normalY = dy / distance;
			float relVel = (a.velocity.x - b.velocity.x) * normalX +
				(a.velocity.y - b.velocity.y) * normalY;
			if (relVel > 0) continue;
			float e = 0.65f;
			float impulse = -(1 + e) * relVel / (1 / a.mass + 1 / b.mass);
			float ix = impulse * normalX;
			float iy = impulse * normalY;
			a.velocity.x += ix / a.mass;
			a.velocity.y += iy / a.mass;
			b.velocity.x -= ix / b.mass;
			b.velocity.y -= iy / b.mass;
			float penetration = radiiSum - distance;
			if (penetration > 0) {
				float percent = 0.2f, slop = 0.01f;
				float correction = std::max(penetration - slop, 0.0f) / (1 / a.mass + 1 / b.mass) * percent;
				a.pos.x += (correction * normalX) / a.mass;
				a.pos.y += (correction * normalY) / a.mass;
				b.pos.x -= (correction * normalX) / b.mass;
				b.pos.y -= (correction * normalY) / b.mass;
			}
		}
	}
}
