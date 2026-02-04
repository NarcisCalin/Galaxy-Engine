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
	}
}

void Physics3D::calculateForceFromGrid3D(UpdateVariables& myVar) {
	// Parallelize the outer loop over all particles
#pragma omp parallel for schedule(dynamic)
	for (size_t i = 0; i < posX.size(); i++) {

		// 1. Initialize Total Force as a 3D vector
		glm::vec3 totalForce = { 0.0f, 0.0f, 0.0f };

		uint32_t gridIdx = 0;
		const uint32_t nodeCount = static_cast<uint32_t>(globalNodes3D.size());

		// Pre-calculate squared constants for performance
		const float thetaSq = myVar.theta * myVar.theta;
		const float softeningSq = myVar.softening * myVar.softening;
		const float Gf = myVar.G;
		const float pmass = mass[i];

		// Barnes-Hut Tree Traversal
		while (gridIdx < nodeCount) {
			Node3D& grid = globalNodes3D[gridIdx];

			float gridMass = grid.gridMass;

			// Optimization: If a node has no mass, it exerts no force. 
			// Skip it and all its children.
			if (gridMass <= 0.0f) {
				gridIdx += grid.next + 1;
				continue;
			}

			// 2. Get 3D Center of Mass and calculate distance vector
			const glm::vec3 gridCOM = grid.centerOfMass;
			const float gridSize = grid.size;

			// Vector d points FROM particle i TO the grid center
			glm::vec3 d = gridCOM - glm::vec3{ posX[i], posY[i], posZ[i] };

			// 3. Periodic Boundary Handling (Added Z-axis)
			if (myVar.isPeriodicBoundaryEnabled) {
				// X Axis
				d.x -= myVar.domainSize3D.x * ((d.x > myVar.halfDomain3DWidth) - (d.x < -myVar.halfDomain3DWidth));
				// Y Axis
				d.y -= myVar.domainSize3D.y * ((d.y > myVar.halfDomain3DHeight) - (d.y < -myVar.halfDomain3DHeight));
				// Z Axis (New)
				d.z -= myVar.domainSize3D.z * ((d.z > myVar.halfDomain3DDepth) - (d.z < -myVar.halfDomain3DDepth));
			}

			// 4. Distance Squared in 3D (x*x + y*y + z*z)
			float distanceSq = d.x * d.x + d.y * d.y + d.z * d.z + softeningSq;

			// 5. Check if this node is a Leaf (has no children)
			// In 3D, we have [2][2][2] children. We check if they are all empty.
			bool isSubgridsEmpty = true;
			for (int x = 0; x < 2; ++x) {
				for (int y = 0; y < 2; ++y) {
					for (int z = 0; z < 2; ++z) {
						if (grid.subGrids[x][y][z] != UINT32_MAX) {
							isSubgridsEmpty = false;
							goto skip_check; // Break out of nested loops early
						}
					}
				}
			}
		skip_check:;

			float gridSizeSq = gridSize * gridSize;

			// Barnes-Hut Condition:
			// If the node is far enough away (size < theta * dist) OR it is a leaf...
			if ((gridSizeSq < thetaSq * distanceSq) || isSubgridsEmpty) {

				// Self-Interaction Check:
				// If it's a leaf containing only 1 particle, check if it is THIS particle.
				if ((grid.endIndex - grid.startIndex) == 1) {
					// Added Z-check
					if (std::abs(posX[grid.startIndex] - posX[i]) < 0.001f &&
						std::abs(posY[grid.startIndex] - posY[i]) < 0.001f &&
						std::abs(posZ[grid.startIndex] - posZ[i]) < 0.001f) {

						gridIdx += grid.next + 1;
						continue;
					}
				}

				// Calculate Force Magnitude
				float invDistance = 1.0f / std::sqrtf(distanceSq);
				float invDist2 = invDistance * invDistance;
				float invDist3 = invDist2 * invDistance;

				float forceMagnitude = Gf * pmass * gridMass * invDist3;

				// Accumulate 3D Force
				totalForce += d * forceMagnitude;

				// 6. Heat Transfer (Optional)
				//if (myVar.isTempEnabled) {
				//	uint32_t count = grid.endIndex - grid.startIndex;
				//	if (count > 0) {
				//		float gridAverageTemp = grid.gridTemp / static_cast<float>(count);
				//		float temperatureDifference = gridAverageTemp - temp[i];

				//		float distance = 0.0f;
				//		// Avoid division by zero
				//		if (distanceSq > 1e-16f) distance = 1.0f / invDistance;

				//		if (distance > 1e-8f) {
				//			float heatTransfer = myVar.globalHeatConductivity * temperatureDifference / distance;
				//			temp[i] += heatTransfer * myVar.timeFactor;
				//		}
				//	}
				//}

				// Skip the children of this node (we already approximated them)
				gridIdx += grid.next + 1;
			}
			else {
				// The node is too close and is not a leaf. Open it and check children.
				++gridIdx;
			}
		}

		// 7. Apply Acceleration to Struct-of-Arrays
		accX[i] = totalForce.x / mass[i];
		accY[i] = totalForce.y / mass[i];
		accZ[i] = totalForce.z / mass[i]; // Apply Z acceleration
	}
}

void Physics3D::naiveGravity3D(std::vector<ParticlePhysics3D>& pParticles3D, UpdateVariables& myVar) {
	int n = static_cast<int>(posX.size());

	// Load Gravity Constant and Softening into vectors
	__m256 gVec = _mm256_set1_ps(myVar.G);
	__m256 softVec = _mm256_set1_ps(myVar.softening);

	// Load pointers for fast access
	const float* posXPtr = posX.data();
	const float* posYPtr = posY.data();
	const float* posZPtr = posZ.data();
	const float* massPtr = mass.data();

	float* accXPtr = accX.data();
	float* accYPtr = accY.data();
	float* accZPtr = accZ.data();

	// Domain constants for Periodic Boundaries
	__m256 domainSizeX = _mm256_set1_ps(myVar.domainSize3D.x);
	__m256 domainSizeY = _mm256_set1_ps(myVar.domainSize3D.y);
	__m256 domainSizeZ = _mm256_set1_ps(myVar.domainSize3D.z);

	__m256 halfDomainWidth = _mm256_set1_ps(myVar.halfDomain3DWidth);
	__m256 halfDomainHeight = _mm256_set1_ps(myVar.halfDomain3DHeight);
	__m256 halfDomainDepth = _mm256_set1_ps(myVar.halfDomain3DDepth);

	__m256 negHalfDomainWidth = _mm256_set1_ps(-myVar.halfDomain3DWidth);
	__m256 negHalfDomainHeight = _mm256_set1_ps(-myVar.halfDomain3DHeight);
	__m256 negHalfDomainDepth = _mm256_set1_ps(-myVar.halfDomain3DDepth);

	// Constants for Newton-Raphson iteration (for rsqrt accuracy)
	__m256 threeHalfs = _mm256_set1_ps(1.5f);
	__m256 pointFive = _mm256_set1_ps(0.5f);

	// Initialize acceleration arrays to 0
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
		// Main AVX Loop (Processes 8 particles at a time)
		for (j = 0; j <= n - 8; j += 8) {
			__m256 pxj = _mm256_loadu_ps(&posXPtr[j]);
			__m256 pyj = _mm256_loadu_ps(&posYPtr[j]);
			__m256 pzj = _mm256_loadu_ps(&posZPtr[j]);
			__m256 mj = _mm256_loadu_ps(&massPtr[j]);

			__m256 dx = _mm256_sub_ps(pxj, pxi);
			__m256 dy = _mm256_sub_ps(pyj, pyi);
			__m256 dz = _mm256_sub_ps(pzj, pzi);

			// Periodic Boundary Correction (Minimum Image Convention)
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

			// Distance Squared = dx*dx + dy*dy + dz*dz + softening
			__m256 distSq =
				_mm256_add_ps(
					_mm256_add_ps(
						_mm256_mul_ps(dx, dx),
						_mm256_mul_ps(dy, dy)),
					_mm256_add_ps(
						_mm256_mul_ps(dz, dz),
						softVec));

			// ---------------------------------------------------------
			// OPTIMIZATION: Fast Inverse Square Root (1 / sqrt(distSq))
			// ---------------------------------------------------------

			// 1. Initial approximation (fast but low precision)
			__m256 invDist = _mm256_rsqrt_ps(distSq);

			// 2. Newton-Raphson Iteration: y = y * (1.5 - 0.5 * x * y * y)
			// This step corrects the error from the approximation.
			// Calculate (0.5 * x * y * y)
			__m256 nrTerm = _mm256_mul_ps(pointFive, distSq);
			nrTerm = _mm256_mul_ps(nrTerm, invDist);
			nrTerm = _mm256_mul_ps(nrTerm, invDist);

			// Calculate (1.5 - nrTerm)
			__m256 nrFactor = _mm256_sub_ps(threeHalfs, nrTerm);

			// Final invDist
			invDist = _mm256_mul_ps(invDist, nrFactor);
			// ---------------------------------------------------------

			// invDist3 = invDist * invDist * invDist
			__m256 invDist3 = _mm256_mul_ps(invDist, _mm256_mul_ps(invDist, invDist));

			// Force Factor = G * mass * invDist^3
			__m256 factor = _mm256_mul_ps(gVec, _mm256_mul_ps(mj, invDist3));

			// Accumulate Acceleration
			totalAccX = _mm256_add_ps(totalAccX, _mm256_mul_ps(dx, factor));
			totalAccY = _mm256_add_ps(totalAccY, _mm256_mul_ps(dy, factor));
			totalAccZ = _mm256_add_ps(totalAccZ, _mm256_mul_ps(dz, factor));
		}

		// Horizontal Sum: Reduce the 8-wide AVX vectors into single float values
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

		// Remainder Loop (For particles that didn't fit in the group of 8)
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

			// Standard scalar math for remainder
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

		// 1. Velocity Verlet / Leapfrog Integration Step 1
		// Save previous velocity for other calculations (like drag or collisions)
		p.prevVel = p.vel;

		// Update velocity by half a timestep based on current acceleration
		p.vel += p.acc * halfDt;

		// 2. Velocity Clamping (Safety measure for SPH/Gravity stability)
		if (myVar.isSPHEnabled) {
			float vSq =
				p.vel.x * p.vel.x +
				p.vel.y * p.vel.y +
				p.vel.z * p.vel.z;

			if (vSq > sphMaxVelSq) {
				// Clamp Previous Velocity
				float prevVSq =
					p.prevVel.x * p.prevVel.x +
					p.prevVel.y * p.prevVel.y +
					p.prevVel.z * p.prevVel.z;

				if (prevVSq > 0.00001f) {
					float invPrevLen = myVar.sphMaxVel / sqrtf(prevVSq);
					p.prevVel *= invPrevLen;
				}

				// Clamp Current Velocity
				float invLen = myVar.sphMaxVel / sqrtf(vSq);
				p.vel *= invLen;
			}
		}

		// 3. Update Position
		p.pos += p.vel * dt;

		// 4. Periodic Boundary Conditions (Centered at 0)
		// We check against -HalfWidth and +HalfWidth instead of 0 and Width.
		if (myVar.isPeriodicBoundaryEnabled) {
			// --- X Axis ---
			// If particle goes too far left (past -50)
			if (p.pos.x < -myVar.halfDomain3DWidth)
				p.pos.x += myVar.domainSize3D.x; // Teleport to right (+50)

			// If particle goes too far right (past +50)
			// FIX: Changed from domainSize3D.x to halfDomain3DWidth
			else if (p.pos.x >= myVar.halfDomain3DWidth)
				p.pos.x -= myVar.domainSize3D.x; // Teleport to left (-50)

			// --- Y Axis ---
			if (p.pos.y < -myVar.halfDomain3DHeight)
				p.pos.y += myVar.domainSize3D.y;
			else if (p.pos.y >= myVar.halfDomain3DHeight) // FIX
				p.pos.y -= myVar.domainSize3D.y;

			// --- Z Axis ---
			if (p.pos.z < -myVar.halfDomain3DDepth)
				p.pos.z += myVar.domainSize3D.z;
			else if (p.pos.z >= myVar.halfDomain3DDepth) // FIX
				p.pos.z -= myVar.domainSize3D.z;
		}
	}
}

void Physics3D::integrateEnd3D(std::vector<ParticlePhysics3D>& pParticles3D, UpdateVariables& myVar) {
#pragma omp parallel for schedule(dynamic)
	for (size_t i = 0; i < pParticles3D.size(); i++) {
		pParticles3D[i].vel += pParticles3D[i].acc * (myVar.timeFactor * 0.5f);
	}
}