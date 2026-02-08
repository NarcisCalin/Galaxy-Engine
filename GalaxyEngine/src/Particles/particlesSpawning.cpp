#include "Particles/particlesSpawning.h"

#include "Physics/physics.h"
#include "Physics/physics3D.h"
#include "Physics/quadtree.h"

#include "parameters.h"

void ParticlesSpawning::particlesInitialConditions(Physics& physics, UpdateVariables& myVar, UpdateParameters& myParam) {

	if (myVar.isMouseNotHoveringUI && isSpawningAllowed) {

		Slingshot slingshot = slingshot.particleSlingshot(myVar, myParam.myCamera);

		if (myVar.isDragging && myVar.enablePathPrediction && myVar.gridExists) {
			predictTrajectory(myParam.pParticles, myParam.myCamera, physics, myVar, slingshot);
		}

		if (IO::mouseReleased(0) && myVar.toolSpawnHeavyParticle && !IO::shortcutDown(KEY_LEFT_CONTROL) && !IO::shortcutDown(KEY_LEFT_ALT) && myVar.isDragging) {

			myParam.pParticles.emplace_back(
				myParam.myCamera.mouseWorldPos,
				slingshot.norm * slingshot.length,
				heavyParticleInitMass * myVar.heavyParticleWeightMultiplier,

				0.008f,
				1.0f,
				1.0f,
				1.0f
			);
			myParam.rParticles.emplace_back(
				Color{ 255, 255, 255, 255 },
				0.3f,
				true,
				false,
				true,
				false,
				false,
				false,
				false,
				-1.0f,
				0
			);
			myVar.isDragging = false;
		}

		if (!myVar.isSPHEnabled) {
			myVar.isBrushDrawing = false;
			myVar.constraintAfterDrawingFlag = false;
		}

		if ((IO::mouseDown(2) || (IO::mouseDown(0) && myVar.toolDrawParticles)) && !IO::shortcutDown(KEY_LEFT_CONTROL) && !IO::shortcutDown(KEY_LEFT_ALT) && !IO::shortcutDown(KEY_X)) {
			myVar.isBrushDrawing = true;

			myParam.brush.brushLogic(myParam, myVar.isSPHEnabled, myVar.constraintAfterDrawing, myVar.massScatter, myVar);
			if (myVar.isBrushDrawing) {
				if (myVar.isSPHEnabled) {

					particlesIterating = true;

					for (int i = 0; i < correctionSubsteps; i++) {

						if (i % 2 == 0) {
							if (!myVar.hasAVX2) {
								myParam.neighborSearchV2.newGrid(myParam.pParticles);
								myParam.neighborSearchV2.neighborAmount(myParam.pParticles, myParam.rParticles);
							}
							else {
								myParam.neighborSearchV2AVX2.newGridAVX2(myParam.pParticles);
								myParam.neighborSearchV2AVX2.neighborAmount(myParam.pParticles, myParam.rParticles);
							}
						}

						physics.spawnCorrection(myParam, myVar.hasAVX2, 1);
					}
				}
			}
		}
		else {

			if (myVar.isSPHEnabled && myVar.isBrushDrawing) {

				particlesIterating = false;
				for (size_t i = 0; i < myParam.pParticles.size(); i++) {
					if (myParam.rParticles[i].spawnCorrectIter < correctionSubsteps && myParam.rParticles[i].isBeingDrawn) {
						particlesIterating = true;
						break;
					}
				}

				if (particlesIterating) {

					for (int i = 0; i < correctionSubsteps * 8; i++) {

						if (i % 4 == 0) {
							if (!myVar.hasAVX2) {
								myParam.neighborSearchV2.newGrid(myParam.pParticles);
								myParam.neighborSearchV2.neighborAmount(myParam.pParticles, myParam.rParticles);
							}
							else {
								myParam.neighborSearchV2AVX2.newGridAVX2(myParam.pParticles);
								myParam.neighborSearchV2AVX2.neighborAmount(myParam.pParticles, myParam.rParticles);
							}
						}

						physics.spawnCorrection(myParam, myVar.hasAVX2, 1);
						particlesIterating = false;
					}
				}
				else {

					if (myVar.constraintAfterDrawing) {
						myVar.constraintAfterDrawingFlag = true;
					}

					if (myVar.constraintAfterDrawingFlag && myVar.constraintAfterDrawing) {
						physics.createConstraints(myParam.pParticles, myParam.rParticles,
							myVar.constraintAfterDrawingFlag, myVar, myParam);
					}

					for (size_t i = 0; i < myParam.pParticles.size(); i++) {
						myParam.rParticles[i].isBeingDrawn = false;
					}

					myVar.isBrushDrawing = false;
				}
			}
		}

		if ((IO::shortcutReleased(KEY_ONE) || IO::mouseReleased(0) && myVar.toolSpawnBigGalaxy) && myVar.isDragging) {

			// VISIBLE MATTER

			for (int i = 0; i < static_cast<int>(40000 * myVar.particleAmountMultiplier); i++) {

				glm::vec2 galaxyCenter = myParam.myCamera.mouseWorldPos;

				float outerRadius = 200.0f;

				float scaleLength = 90.0f;

				float normalizedRand = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

				float angle = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2 * PI;

				float finalRadius = -scaleLength * log(1.0f - normalizedRand);

				finalRadius = std::min(finalRadius, outerRadius + 600.0f);

				finalRadius = std::max(finalRadius, 0.01f);

				glm::vec2 pos = glm::vec2(galaxyCenter.x + finalRadius * cos(angle), galaxyCenter.y + finalRadius * sin(angle));

				glm::vec2 d = pos - galaxyCenter;

				glm::vec2 tangent = glm::vec2(d.y, -d.x);

				float length = sqrt(tangent.x * tangent.x + tangent.y * tangent.y);

				tangent /= length;

				float speed = 10.5f * sqrt(1758.0f / (finalRadius + 54.7f));

				glm::vec2 vel = tangent * speed * 0.85f;

				float finalMass = 0.0f;

				float rand01 = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
				float randomMassMultiplier = 1.0f + (rand01 * 2.0f - 1.0f) * myVar.massScatter;

				if (massMultiplierEnabled) {
					finalMass = (8500000000.0f / myVar.particleAmountMultiplier) * randomMassMultiplier;
				}
				else {
					finalMass = 8500000000.0f * randomMassMultiplier;
				}

				myParam.pParticles.emplace_back(
					pos,
					vel + slingshot.norm * slingshot.length * 0.3f,
					finalMass,

					0.008f,
					1.0f,
					1.0f,
					1.0f
				);
				myParam.rParticles.emplace_back(
					Color{ 128, 128, 128, 100 },
					0.125f,
					false,
					false,
					false,
					true,
					true,
					false,
					true,
					-1.0f,
					0
				);
			}

			/*for (int i = 0; i < static_cast<int>(160000 * particleAmountMultiplier); i++) {

				glm::vec2 galaxyCenter = myParam.myCamera.mouseWorldPos;

				float outerRadius = 200.0f;

				float scaleLength = 90.0f;

				float normalizedRand = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

				float angle = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2 * PI;

				float finalRadius = -scaleLength * log(1.0f - normalizedRand);

				finalRadius = std::min(finalRadius, outerRadius + 600.0f);

				finalRadius = std::max(finalRadius, 0.01f);

				glm::vec2 pos = glm::vec2(galaxyCenter.x + finalRadius * cos(angle), galaxyCenter.y + finalRadius * sin(angle));

				glm::vec2 d = pos - galaxyCenter;

				glm::vec2 tangent = glm::vec2(d.y, -d.x);

				float length = sqrt(tangent.x * tangent.x + tangent.y * tangent.y);

				tangent /= length;

				float speed = 10.5f * sqrt(1758.0f / (finalRadius + 54.7f));

				glm::vec2 vel = tangent * speed * 0.85f;

				float finalMass = 0.0f;

				float rand01 = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
				float randomMassMultiplier = 1.0f + (rand01 * 2.0f - 1.0f) * 0.35f;

				if (massMultiplierEnabled) {
					finalMass = (8500000000.0f / particleAmountMultiplier) * randomMassMultiplier * 0.25f;
				}
				else {
					finalMass = 8500000000.0f * randomMassMultiplier * 0.25f;
				}

				myParam.pParticles.emplace_back(
					pos,
					vel + slingshot.norm * slingshot.length * 0.3f,
					finalMass,

					0.008f,
					1.0f,
					1.0f,
					1.0f
				);
				myParam.rParticles.emplace_back(
					Color{ 128, 128, 128, 100 },
					0.125f,
					false,
					false,
					false,
					true,
					true,
					false,
					true,
					-1.0f,
					0
				);
			}*/

			// DARK MATTER

			if (myVar.isDarkMatterEnabled) {
				for (int i = 0; i < static_cast<int>(12000 * myVar.DMAmountMultiplier); i++) {
					glm::vec2 galaxyCenter = myParam.myCamera.mouseWorldPos;

					float outerRadius = 2000.0f;
					float radiusCore = 3.5f;

					float normalizedRand = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

					float radiusMultiplier = radiusCore * sqrt(static_cast<float>(pow(1 + pow(outerRadius / radiusCore, 2), normalizedRand) - 1));

					float angle = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2 * PI;

					glm::vec2 pos = glm::vec2(galaxyCenter.x + radiusMultiplier * cos(angle), galaxyCenter.y + radiusMultiplier * sin(angle));

					glm::vec2 vel = glm::vec2(static_cast<float>(rand() % 60 - 30), static_cast<float>(rand() % 60 - 30)) * 0.85f;

					float finalMass = 0.0f;

					float rand01 = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
					float randomMassMultiplier = 1.0f + (rand01 * 2.0f - 1.0f) * myVar.massScatter;

					if (massMultiplierEnabled) {
						finalMass = 141600000000.0f / myVar.DMAmountMultiplier * randomMassMultiplier;
					}
					else {
						finalMass = 141600000000.0f * randomMassMultiplier;
					}

					myParam.pParticles.emplace_back(
						pos,
						vel + slingshot.norm * slingshot.length * 0.3f,
						finalMass,

						0.008f,
						1.0f,
						1.0f,
						1.0f
					);
					myParam.rParticles.emplace_back(
						Color{ 128, 128, 128, 0 },
						0.125f,
						true,
						false,
						false,
						false,
						true,
						true,
						false,
						-1.0f,
						0
					);
				}
			}

			myVar.isDragging = false;
		}

		if ((IO::shortcutReleased(KEY_TWO) || IO::mouseReleased(0) && myVar.toolSpawnSmallGalaxy) && myVar.isDragging) {

			// VISIBLE MATTER

			for (int i = 0; i < static_cast<int>(12000 * myVar.particleAmountMultiplier); i++) {
				glm::vec2 galaxyCenter = myParam.myCamera.mouseWorldPos;

				float outerRadius = 100.0f;

				float scaleLength = 45.0f;

				float normalizedRand = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

				float angle = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2 * PI;

				float finalRadius = -scaleLength * log(1.0f - normalizedRand);

				finalRadius = std::min(finalRadius, outerRadius + 300.0f);

				finalRadius = std::max(finalRadius, 0.01f);

				glm::vec2 pos = glm::vec2(galaxyCenter.x + finalRadius * cos(angle), galaxyCenter.y + finalRadius * sin(angle));

				glm::vec2 d = pos - galaxyCenter;

				glm::vec2 tangent = glm::vec2(d.y, -d.x);

				float length = sqrt(tangent.x * tangent.x + tangent.y * tangent.y);

				tangent /= length;

				float speed = 10.5f * sqrt(505.0f / (finalRadius + 54.7f)) * 0.85f;

				glm::vec2 vel = tangent * speed;

				float finalMass = 0.0f;

				float rand01 = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
				float randomMassMultiplier = 1.0f + (rand01 * 2.0f - 1.0f) * myVar.massScatter;

				if (massMultiplierEnabled) {
					finalMass = 8500000000.0f / myVar.particleAmountMultiplier * randomMassMultiplier;
				}
				else {
					finalMass = 8500000000.0f;
				}

				myParam.pParticles.emplace_back(
					pos,
					vel + slingshot.norm * slingshot.length * 0.3f,
					finalMass,

					0.008f,
					1.0f,
					1.0f,
					1.0f
				);
				myParam.rParticles.emplace_back(
					Color{ 128, 128, 128, 100 },
					0.125f,
					false,
					false,
					false,
					true,
					true,
					false,
					true,
					-1.0f,
					0
				);
			}

			// DARK MATTER
			if (myVar.isDarkMatterEnabled) {
				for (int i = 0; i < static_cast<int>(3600 * myVar.DMAmountMultiplier); i++) {
					glm::vec2 galaxyCenter = myParam.myCamera.mouseWorldPos;

					float outerRadius = 2000.0f;
					float radiusCore = 3.5f;

					float normalizedRand = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

					float radiusMultiplier = radiusCore * sqrt(static_cast<float>(pow(1 + pow(outerRadius / radiusCore, 2), normalizedRand) - 1));

					float angle = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2 * PI;

					glm::vec2 pos = glm::vec2(galaxyCenter.x + radiusMultiplier * cos(angle), galaxyCenter.y + radiusMultiplier * sin(angle));

					glm::vec2 vel = glm::vec2(static_cast<float>(rand() % 60 - 30), static_cast<float>(rand() % 60 - 30)) * 0.85f;

					float finalMass = 0.0f;

					float rand01 = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
					float randomMassMultiplier = 1.0f + (rand01 * 2.0f - 1.0f) * myVar.massScatter;

					if (massMultiplierEnabled) {
						finalMass = 141600000000.0f / myVar.DMAmountMultiplier * randomMassMultiplier;
					}
					else {
						finalMass = 141600000000.0f * randomMassMultiplier;
					}

					myParam.pParticles.emplace_back(
						pos,
						vel + slingshot.norm * slingshot.length * 0.3f,
						finalMass,

						0.008f,
						1.0f,
						1.0f,
						1.0f
					);
					myParam.rParticles.emplace_back(
						Color{ 128, 128, 128, 0 },
						0.125f,
						true,
						false,
						false,
						false,
						true,
						true,
						false,
						-1.0f,
						0
					);
				}
			}

			myVar.isDragging = false;
		}

		if ((IO::shortcutReleased(KEY_THREE) || IO::mouseReleased(0) && myVar.toolSpawnStar) && !IO::shortcutDown(KEY_LEFT_CONTROL) && !IO::shortcutDown(KEY_LEFT_ALT)) {

			for (int i = 0; i < static_cast<int>(10000 * myVar.particleAmountMultiplier); i++) {

				float angle = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * 3.14159f;
				float distance = (sqrt(static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 5.0f) + 0.1f;

				glm::vec2 randomOffset = {
					cos(angle) * distance,
					sin(angle) * distance
				};

				glm::vec2 particlePos = myParam.myCamera.mouseWorldPos + randomOffset;

				float finalMass = 0.0f;

				if (massMultiplierEnabled) {
					finalMass = 8500000000.0f / myVar.particleAmountMultiplier;
				}
				else {
					finalMass = 8500000000.0f;
				}

				myParam.pParticles.emplace_back(
					glm::vec2{ particlePos.x, particlePos.y },
					slingshot.norm * slingshot.length * 0.3f,
					finalMass,

					0.008f,
					1.0f,
					1.0f,
					1.0f
				);
				myParam.rParticles.emplace_back(
					Color{ 128, 128, 128, 100 },
					0.125f,
					false,
					false,
					false,
					true,
					true,
					false,
					true,
					-1.0f,
					0
				);
			}

			myVar.isDragging = false;
		}

		if ((IO::shortcutPress(KEY_FOUR) || IO::mouseReleased(0) && myVar.toolSpawnBigBang) && !IO::shortcutDown(KEY_LEFT_CONTROL) && !IO::shortcutDown(KEY_LEFT_ALT)) {

			// VISIBLE MATTER

			for (int i = 0; i < static_cast<int>(40000 * myVar.particleAmountMultiplier); i++) {
				float angle = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * 3.14159f;
				float distance = (sqrt(static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 20.0f) + 1.0f;

				glm::vec2 randomOffset = {
					cos(angle) * distance + static_cast<float>(GetRandomValue(-15, 15)),
					sin(angle) * distance + static_cast<float>(GetRandomValue(-15, 15))
				};

				glm::vec2 particlePos = myParam.myCamera.mouseWorldPos + randomOffset;

				glm::vec2 d = particlePos - myParam.myCamera.mouseWorldPos;

				glm::vec2 norm = d / distance;

				float speed = 300.0f;

				float adjustedSpeed = speed * (distance / 35.0f);

				glm::vec2 vel = adjustedSpeed * norm;

				float finalMass = 0.0f;

				float rand01 = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
				float randomMassMultiplier = 1.0f + (rand01 * 2.0f - 1.0f) * myVar.massScatter;

				if (massMultiplierEnabled) {
					finalMass = 8500000000.0f / myVar.particleAmountMultiplier * randomMassMultiplier;
				}
				else {
					finalMass = 8500000000.0f * randomMassMultiplier;
				}

				myParam.pParticles.emplace_back(
					particlePos,
					vel,
					finalMass,

					0.008f,
					1.0f,
					1.0f,
					1.0f
				);
				myParam.rParticles.emplace_back(
					Color{ 128, 128, 128, 100 },
					0.125f,
					false,
					false,
					false,
					true,
					true,
					false,
					true,
					-1.0f,
					0
				);
			}

			// DARK MATTER

			if (myVar.isDarkMatterEnabled) {
				for (int i = 0; i < static_cast<int>(12000 * myVar.DMAmountMultiplier); i++) {

					float angle = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * 3.14159f;
					float distance = (sqrt(static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 20.0f) + 1.0f;

					glm::vec2 randomOffset = {
						cos(angle) * distance + static_cast<float>(GetRandomValue(-15, 15)),
						sin(angle) * distance + static_cast<float>(GetRandomValue(-15, 15))
					};

					glm::vec2 particlePos = myParam.myCamera.mouseWorldPos + randomOffset;

					glm::vec2 d = particlePos - myParam.myCamera.mouseWorldPos;

					glm::vec2 norm = d / distance;

					float speed = 300.0f;

					float adjustedSpeed = speed * (distance / 35.0f);

					glm::vec2 vel = adjustedSpeed * norm;

					float finalMass = 0.0f;

					float rand01 = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
					float randomMassMultiplier = 1.0f + (rand01 * 2.0f - 1.0f) * myVar.massScatter;

					if (massMultiplierEnabled) {
						finalMass = 141600000000.0f / myVar.DMAmountMultiplier * randomMassMultiplier;
					}
					else {
						finalMass = 141600000000.0f * randomMassMultiplier;
					}

					myParam.pParticles.emplace_back(
						particlePos,
						vel,
						finalMass,

						0.008f,
						1.0f,
						1.0f,
						1.0f
					);
					myParam.rParticles.emplace_back(
						Color{ 128, 128, 128, 0 },
						0.125f,
						true,
						false,
						false,
						false,
						true,
						true,
						false,
						-1.0f,
						0
					);
				}
			}
		}
	}
	else {
		if (IsMouseButtonPressed(0)) {
			isSpawningAllowed = false;
		}
	}

	if (IsMouseButtonReleased(0)) {
		isSpawningAllowed = true;
		myVar.isDragging = false;
	}
}

void ParticlesSpawning::predictTrajectory(const std::vector<ParticlePhysics>& pParticles,
	SceneCamera& myCamera, Physics physics,
	UpdateVariables& myVar, Slingshot& slingshot) {

	if (!IsMouseButtonDown(0)) {
		return;
	}

	std::vector<ParticlePhysics> currentParticles = pParticles;

	ParticlePhysics predictedParticle(
		glm::vec2(myCamera.mouseWorldPos),
		glm::vec2{ slingshot.norm * slingshot.length },
		heavyParticleInitMass * myVar.heavyParticleWeightMultiplier,

		1.0f,
		1.0f,
		1.0f,
		1.0f
	);

	currentParticles.push_back(predictedParticle);

	int predictedIndex = static_cast<int>(currentParticles.size()) - 1;

	std::vector<glm::vec2> predictedPath;

	for (int step = 0; step < myVar.predictPathLength; ++step) {
		ParticlePhysics& p = currentParticles[predictedIndex];

		glm::vec2 netForce = physics.calculateForceFromGridOld(currentParticles, myVar, p);

		p.acc = netForce / p.mass;

		p.vel += p.acc * (myVar.timeFactor * 0.5f);

		p.pos += p.vel * myVar.timeFactor;

		if (myVar.isPeriodicBoundaryEnabled) {
			if (p.pos.x < 0.0f) p.pos.x += myVar.domainSize.x;
			else if (p.pos.x >= myVar.domainSize.x) p.pos.x -= myVar.domainSize.x;

			if (p.pos.y < 0.0f) p.pos.y += myVar.domainSize.y;
			else if (p.pos.y >= myVar.domainSize.y) p.pos.y -= myVar.domainSize.y;
		}

		netForce = physics.calculateForceFromGridOld(currentParticles, myVar, p);
		p.acc = netForce / p.mass;

		p.vel += p.acc * (myVar.timeFactor * 0.5f);

		predictedPath.push_back(p.pos);
	}

	for (size_t i = 1; i < predictedPath.size(); ++i) {
		DrawLineV({ predictedPath[i - 1].x,  predictedPath[i - 1].y }, { predictedPath[i].x,  predictedPath[i].y }, WHITE);
	}
}

// ---- 3D IMPLEMENTATION ---- //

void ParticlesSpawning3D::particlesInitialConditions(Physics3D& physics3D, UpdateVariables& myVar, UpdateParameters& myParam) {

	if (myVar.isMouseNotHoveringUI && isSpawningAllowed) {

		Slingshot3D slingshot = slingshot.particleSlingshot(myVar, myParam.brush3D.brushPos);

		if (myVar.isDragging && myVar.enablePathPrediction && myVar.grid3DExists) {
			predictTrajectory(myParam.pParticles3D, myParam.myCamera3D, physics3D, myVar, myParam, slingshot);
		}

		if (IO::mouseReleased(0) && myVar.toolSpawnHeavyParticle && !IO::shortcutDown(KEY_LEFT_CONTROL) && !IO::shortcutDown(KEY_LEFT_ALT) && myVar.isDragging) {

			myParam.pParticles3D.emplace_back(
				myParam.brush3D.brushPos,
				slingshot.norm * slingshot.length,
				heavyParticleInitMass * myVar.heavyParticleWeightMultiplier,

				0.008f,
				1.0f,
				1.0f,
				1.0f
			);
			myParam.rParticles3D.emplace_back(
				Color{ 255, 255, 255, 255 },
				0.3f,
				true,
				false,
				true,
				false,
				false,
				false,
				false,
				-1.0f,
				0
			);
			myVar.isDragging = false;
		}

		if (!myVar.isSPHEnabled) {
			myVar.isBrushDrawing = false;
			myVar.constraintAfterDrawingFlag = false;
		}

		if ((IO::mouseDown(2) || (IO::mouseDown(0) && myVar.toolDrawParticles)) && !IO::shortcutDown(KEY_LEFT_CONTROL) && !IO::shortcutDown(KEY_LEFT_ALT) && !IO::shortcutDown(KEY_X)) {
			myVar.isBrushDrawing = true;

			myParam.brush3D.brushLogic(myParam, myVar.isSPHEnabled, myVar.constraintAfterDrawing, myVar.massScatter, myVar);

			if (myVar.isBrushDrawing) {
				if (myVar.isSPHEnabled) {

					particlesIterating = true;

					//#pragma omp parallel for
					//					for (int i = 0; i < correctionSubsteps; i++) {
					//						physics.buildGrid(myParam.pParticles, myParam.rParticles, physics, myVar.domainSize, correctionSubsteps);
					//					}
				}
			}
		}
		else {

			if (myVar.isSPHEnabled && myVar.isBrushDrawing) {

				particlesIterating = false;
				for (size_t i = 0; i < myParam.pParticles3D.size(); i++) {
					if (myParam.rParticles3D[i].spawnCorrectIter < correctionSubsteps && myParam.rParticles3D[i].isBeingDrawn) {
						particlesIterating = true;
						break;
					}
				}

				if (particlesIterating) {
					//#pragma omp parallel for
										/*for (int i = 0; i < correctionSubsteps * 2; i++) {
											physics.buildGrid(myParam.pParticles, myParam.rParticles, physics, myVar.domainSize, correctionSubsteps);
										}*/
				}
				else {

					/*if (myVar.constraintAfterDrawing) {
						myVar.constraintAfterDrawingFlag = true;
					}

					if (myVar.constraintAfterDrawingFlag && myVar.constraintAfterDrawing) {
						physics.createConstraints(myParam.pParticles, myParam.rParticles,
							myVar.constraintAfterDrawingFlag, myVar, myParam);
					}*/

					for (size_t i = 0; i < myParam.pParticles3D.size(); i++) {
						myParam.rParticles3D[i].isBeingDrawn = false;
					}
					myVar.isBrushDrawing = false;
				}
			}
		}

		if ((IO::shortcutReleased(KEY_ONE) || IO::mouseReleased(0) && myVar.toolSpawnBigGalaxy) && myVar.isDragging) {

			// VISIBLE MATTER

			glm::mat4 rotationMatrix = glm::mat4(1.0f);

			rotationMatrix = glm::rotate(rotationMatrix, diskAxisX * (PI / 180.0f), glm::vec3(1.0f, 0.0f, 0.0f));

			rotationMatrix = glm::rotate(rotationMatrix, diskAxisY * (PI / 180.0f), glm::vec3(0.0f, 1.0f, 0.0f));

			rotationMatrix = glm::rotate(rotationMatrix, diskAxisZ * (PI / 180.0f), glm::vec3(0.0f, 0.0f, 1.0f));

			for (int i = 0; i < static_cast<int>(40000 * myVar.particleAmountMultiplier); i++) {

				glm::vec3 galaxyCenter = myParam.brush3D.brushPos;

				float outerRadius = 120.0f;
				float radiusCore = 2.5f;

				float normalizedRand = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
				float finalRadius = radiusCore * sqrt(static_cast<float>(pow(1 + pow(outerRadius / radiusCore, 2), normalizedRand) - 1));

				float angle = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * PI;

				float u1 = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
				float u2 = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
				if (u1 < 1e-6f) u1 = 1e-6f;

				float diskThickness = 0.5f;
				float bulgeThickness = 3.0f;
				float bulgeSize = 1200.0f;

				float currentSpread = diskThickness + bulgeThickness * exp(-finalRadius / bulgeSize);
				float normalRandom = sqrt(-2.0f * log(u1)) * cos(2.0f * PI * u2);
				float zOffset = normalRandom * currentSpread;

				glm::vec4 localPos = glm::vec4(
					finalRadius * cos(angle), // x
					finalRadius * sin(angle), // y
					zOffset,                  // z
					1.0f                      // w (Position = 1.0)
				);

				glm::vec3 localTangent = glm::vec3(-localPos.y, localPos.x, 0.0f);
				float length = sqrt(localTangent.x * localTangent.x + localTangent.y * localTangent.y);
				if (length > 0.0001f) {
					localTangent /= length;
				}
				else {
					localTangent = glm::vec3(1.0f, 0.0f, 0.0f);
				}

				float speed = 10.5f * sqrt(1758.0f / (finalRadius + 54.0f));

				glm::vec4 localVel = glm::vec4(localTangent * speed * 0.85f, 0.0f);

				glm::vec4 rotatedPos = rotationMatrix * localPos;
				glm::vec4 rotatedVel = rotationMatrix * localVel;

				glm::vec3 finalPos = glm::vec3(rotatedPos) + galaxyCenter;

				glm::vec3 finalVel = glm::vec3(rotatedVel) + (slingshot.norm * slingshot.length * 0.3f);

				float finalMass = 0.0f;
				float rand01 = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
				float randomMassMultiplier = 1.0f + (rand01 * 2.0f - 1.0f) * myVar.massScatter;

				if (massMultiplierEnabled) {
					finalMass = (8500000000.0f / myVar.particleAmountMultiplier) * randomMassMultiplier;
				}
				else {
					finalMass = 8500000000.0f * randomMassMultiplier;
				}

				myParam.pParticles3D.emplace_back(
					finalPos,
					finalVel,
					finalMass,
					0.008f,
					1.0f,
					1.0f,
					1.0f
				);

				myParam.rParticles3D.emplace_back(
					Color{ 128, 128, 128, 100 },
					0.125f,
					false,
					false,
					false,
					true,
					true,
					false,
					true,
					-1.0f,
					0
				);
			}

			// DARK MATTER

			if (myVar.isDarkMatterEnabled) {
				for (int i = 0; i < static_cast<int>(12000 * myVar.DMAmountMultiplier); i++) {

					glm::vec3 galaxyCenter = myParam.brush3D.brushPos;

					float outerRadius = 2000.0f;
					float radiusCore = 3.5f;

					float normalizedRand = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

					float radiusMultiplier = radiusCore * sqrt(static_cast<float>(pow(1 + pow(outerRadius / radiusCore, 2), normalizedRand) - 1));

					float z = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 2.0f - 1.0f;

					float theta = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 2.0f * PI;

					float radius_xy = sqrt(1.0f - z * z);

					float x = radius_xy * cos(theta);
					float y = radius_xy * sin(theta);

					glm::vec3 pos = glm::vec3(
						galaxyCenter.x + x * radiusMultiplier,
						galaxyCenter.y + y * radiusMultiplier,
						galaxyCenter.z + z * radiusMultiplier
					);

					glm::vec3 vel = glm::vec3(
						static_cast<float>(rand() % 60 - 30),
						static_cast<float>(rand() % 60 - 30),
						static_cast<float>(rand() % 60 - 30)
					) * 0.85f;

					float finalMass = 0.0f;

					float rand01 = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
					float randomMassMultiplier = 1.0f + (rand01 * 2.0f - 1.0f) * myVar.massScatter;

					if (massMultiplierEnabled) {
						finalMass = 141600000000.0f / myVar.DMAmountMultiplier * randomMassMultiplier;
					}
					else {
						finalMass = 141600000000.0f * randomMassMultiplier;
					}

					myParam.pParticles3D.emplace_back(
						pos,
						vel + slingshot.norm * slingshot.length * 0.3f,
						finalMass,

						0.008f,
						1.0f,
						1.0f,
						1.0f
					);

					myParam.rParticles3D.emplace_back(
						Color{ 128, 128, 128, 0 },
						0.125f,
						true,
						false,
						false,
						false,
						true,
						true,
						false,
						-1.0f,
						0
					);
				}

				myVar.isDragging = false;
			}

			myVar.isDragging = false;
		}

		if ((IO::shortcutReleased(KEY_THREE) || IO::mouseReleased(0) && myVar.toolSpawnStar) && !IO::shortcutDown(KEY_LEFT_CONTROL) && !IO::shortcutDown(KEY_LEFT_ALT)) {
			for (int i = 0; i < static_cast<int>(10000 * myVar.particleAmountMultiplier); i++) {

				float theta = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * 3.14159f;
				float phi = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 3.14159f;

				float distance = (cbrt(static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 5.0f) + 0.1f;

				glm::vec3 randomOffset = {
					sin(phi) * cos(theta) * distance,
					sin(phi) * sin(theta) * distance,
					cos(phi) * distance
				};

				glm::vec3 particlePos = myParam.brush3D.brushPos + randomOffset;

				float finalMass = 0.0f;

				if (massMultiplierEnabled) {
					finalMass = 8500000000.0f / myVar.particleAmountMultiplier;
				}
				else {
					finalMass = 8500000000.0f;
				}

				myParam.pParticles3D.emplace_back(
					glm::vec3{ particlePos.x, particlePos.y, particlePos.z },
					slingshot.norm * slingshot.length * 0.3f,
					finalMass,

					0.008f,
					1.0f,
					1.0f,
					1.0f
				);

				myParam.rParticles3D.emplace_back(
					Color{ 128, 128, 128, 100 },
					0.125f,
					false,
					false,
					false,
					true,
					true,
					false,
					true,
					-1.0f,
					0
				);
			}

			myVar.isDragging = false;
		}

		if ((IO::shortcutPress(KEY_FOUR) || IO::mouseReleased(0) && myVar.toolSpawnBigBang) && !IO::shortcutDown(KEY_LEFT_CONTROL) && !IO::shortcutDown(KEY_LEFT_ALT)) {

			// VISIBLE MATTER
			for (int i = 0; i < static_cast<int>(40000 * myVar.particleAmountMultiplier); i++) {

				float theta = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * 3.14159f;

				float random_neg1_to_1 = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f) - 1.0f;
				float phi = acos(random_neg1_to_1);

				float rand01_dist = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
				float distance = (cbrt(rand01_dist) * 20.0f) + 1.0f;

				glm::vec3 randomOffset = {
					distance * sin(phi) * cos(theta) + static_cast<float>(GetRandomValue(-15, 15)),
					distance * sin(phi) * sin(theta) + static_cast<float>(GetRandomValue(-15, 15)),
					distance * cos(phi) + static_cast<float>(GetRandomValue(-15, 15))
				};

				glm::vec3 particlePos = myParam.brush3D.brushPos + randomOffset;

				glm::vec3 d = particlePos - myParam.brush3D.brushPos;

				glm::vec3 norm = d / distance;

				float speed = 160.0f;
				float adjustedSpeed = speed * (distance / 35.0f);

				glm::vec3 vel = adjustedSpeed * norm;

				float finalMass = 0.0f;
				float rand01 = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
				float randomMassMultiplier = 1.0f + (rand01 * 2.0f - 1.0f) * myVar.massScatter;

				if (massMultiplierEnabled) {
					finalMass = 8500000000.0f / myVar.particleAmountMultiplier * randomMassMultiplier;
				}
				else {
					finalMass = 8500000000.0f * randomMassMultiplier;
				}

				myParam.pParticles3D.emplace_back(
					particlePos,
					vel,
					finalMass,
					0.008f,
					1.0f,
					1.0f,
					1.0f
				);

				myParam.rParticles3D.emplace_back(
					Color{ 128, 128, 128, 100 },
					0.125f,
					false,
					false,
					false,
					true,
					true,
					false,
					true,
					-1.0f,
					0
				);
			}

			// DARK MATTER
			if (myVar.isDarkMatterEnabled) {
				for (int i = 0; i < static_cast<int>(12000 * myVar.DMAmountMultiplier); i++) {

					float theta = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * 3.14159f;
					float phi = acos((static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f) - 1.0f);

					float rand01_dist = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
					float distance = (cbrt(rand01_dist) * 20.0f) + 1.0f;

					glm::vec3 randomOffset = {
						distance * sin(phi) * cos(theta) + static_cast<float>(GetRandomValue(-15, 15)),
						distance * sin(phi) * sin(theta) + static_cast<float>(GetRandomValue(-15, 15)),
						distance * cos(phi) + static_cast<float>(GetRandomValue(-15, 15))
					};

					glm::vec3 particlePos = myParam.brush3D.brushPos + randomOffset;
					glm::vec3 d = particlePos - myParam.brush3D.brushPos;
					glm::vec3 norm = d / distance;

					float speed = 160.0f;
					float adjustedSpeed = speed * (distance / 35.0f);
					glm::vec3 vel = adjustedSpeed * norm;

					float finalMass = 0.0f;
					float rand01 = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
					float randomMassMultiplier = 1.0f + (rand01 * 2.0f - 1.0f) * myVar.massScatter;

					if (massMultiplierEnabled) {
						finalMass = 141600000000.0f / myVar.DMAmountMultiplier * randomMassMultiplier;
					}
					else {
						finalMass = 141600000000.0f * randomMassMultiplier;
					}

					myParam.pParticles3D.emplace_back(
						particlePos,
						vel,
						finalMass,
						0.008f,
						1.0f,
						1.0f,
						1.0f
					);
					myParam.rParticles3D.emplace_back(
						Color{ 128, 128, 128, 0 },
						0.125f,
						true,
						false,
						false,
						false,
						true,
						true,
						false,
						-1.0f,
						0
					);
				}
			}
		}
	}
	else {
		if (IsMouseButtonPressed(0)) {
			isSpawningAllowed = false;
		}
	}

	if (IsMouseButtonReleased(0)) {
		isSpawningAllowed = true;
		myVar.isDragging = false;
	}
}

void ParticlesSpawning3D::predictTrajectory(const std::vector<ParticlePhysics3D>& pParticles,
	SceneCamera3D& myCamera, Physics3D physics3D,
	UpdateVariables& myVar, UpdateParameters& myParam, Slingshot3D& slingshot) {

	if (!IsMouseButtonDown(0)) {
		return;
	}

	std::vector<ParticlePhysics3D> currentParticles = pParticles;

	ParticlePhysics3D predictedParticle(
		glm::vec3(myParam.brush3D.brushPos),
		glm::vec3(slingshot.norm.x * slingshot.length, slingshot.norm.y * slingshot.length, slingshot.norm.z * slingshot.length),
		heavyParticleInitMass * myVar.heavyParticleWeightMultiplier,
		1.0f,
		1.0f,
		1.0f,
		1.0f
	);

	currentParticles.push_back(predictedParticle);
	int predictedIndex = static_cast<int>(currentParticles.size()) - 1;

	std::vector<glm::vec3> predictedPath;

	for (int step = 0; step < myVar.predictPathLength; ++step) {
		ParticlePhysics3D& p = currentParticles[predictedIndex];

		glm::vec3 netForce = physics3D.calculateForceFromGrid3DOld(currentParticles, myVar, p);

		p.acc = netForce / p.mass;
		p.vel += p.acc * (myVar.timeFactor * 0.5f);
		p.pos += p.vel * myVar.timeFactor;

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

		netForce = physics3D.calculateForceFromGrid3DOld(currentParticles, myVar, p);
		p.acc = netForce / p.mass;
		p.vel += p.acc * (myVar.timeFactor * 0.5f);

		predictedPath.push_back(p.pos);
	}

	for (size_t i = 1; i < predictedPath.size(); ++i) {
		DrawLine3D(
			{ predictedPath[i - 1].x, predictedPath[i - 1].y, predictedPath[i - 1].z },
			{ predictedPath[i].x, predictedPath[i].y, predictedPath[i].z },
			WHITE
		);
	}
}

void ParticlesSpawning3D::drawGalaxyDisplay(UpdateParameters& myParam) {

	glm::mat4 rotationMatrix = glm::mat4(1.0f);

	rotationMatrix = glm::rotate(rotationMatrix, diskAxisX * (PI / 180.0f), glm::vec3(1.0f, 0.0f, 0.0f));

	rotationMatrix = glm::rotate(rotationMatrix, diskAxisY * (PI / 180.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	rotationMatrix = glm::rotate(rotationMatrix, diskAxisZ * (PI / 180.0f), glm::vec3(0.0f, 0.0f, 1.0f));

	glm::vec4 defaultNormal = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
	glm::vec4 rotatedNormal = rotationMatrix * defaultNormal;

	glm::vec3 direction = glm::vec3(rotatedNormal);


	float diskVisualThickness = 2.0f;

	glm::vec3 startPos = myParam.brush3D.brushPos - (direction * diskVisualThickness);
	glm::vec3 endPos = myParam.brush3D.brushPos + (direction * diskVisualThickness);

	Vector3 raylibStart = { startPos.x, startPos.y, startPos.z };
	Vector3 raylibEnd = { endPos.x, endPos.y, endPos.z };

	DrawCylinderEx(
		raylibStart,
		raylibEnd,
		130.0f,
		130.0f,
		24,
		Color{ 255, 0, 0, 100 }
	);
}
