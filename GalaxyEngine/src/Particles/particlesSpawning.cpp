#include "Particles/particlesSpawning.h"

#include "Physics/physics.h"
#include "Physics/quadtree.h"

#include "parameters.h"

void ParticlesSpawning::particlesInitialConditions(Quadtree* quadtree, Physics& physics, UpdateVariables& myVar, UpdateParameters& myParam) {

	if (myVar.isMouseNotHoveringUI && isSpawningAllowed) {

		Slingshot slingshot = slingshot.particleSlingshot(myVar.isDragging, myParam.myCamera);

		if (myVar.isDragging && enablePathPrediction && quadtree != nullptr) {
			predictTrajectory(myParam.pParticles, myParam.myCamera, physics, quadtree, myVar, slingshot);
		}

		if (IsMouseButtonReleased(0) && !IsKeyDown(KEY_LEFT_CONTROL) && !IsKeyDown(KEY_LEFT_ALT) && myVar.isDragging) {
			myParam.pParticles.emplace_back(
				myParam.myCamera.mouseWorldPos,
				slingshot.norm * slingshot.length,
				heavyParticleInitMass * heavyParticleWeightMultiplier,

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

		if (IsMouseButtonDown(2) && !IsKeyDown(KEY_LEFT_CONTROL) && !IsKeyDown(KEY_LEFT_ALT) && !IsKeyDown(KEY_X)) {
			myVar.isBrushDrawing = true;

			myParam.brush.brushLogic(myParam, myVar.isSPHEnabled, myVar.constraintAfterDrawing);
			if (myVar.isBrushDrawing) {
				if (myVar.isSPHEnabled) {

					particlesIterating = true;

#pragma omp parallel for
					for (int i = 0; i < correctionSubsteps; i++) {
						physics.buildGrid(myParam.pParticles, myParam.rParticles, physics, myVar.domainSize, correctionSubsteps);
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
#pragma omp parallel for
					for (int i = 0; i < correctionSubsteps * 2; i++) {
						physics.buildGrid(myParam.pParticles, myParam.rParticles, physics, myVar.domainSize, correctionSubsteps);
					}
				}
				else {

					if (myVar.constraintAfterDrawing) {
						myVar.constraintAfterDrawingFlag = true;
					}

					if (myVar.constraintAfterDrawingFlag && myVar.constraintAfterDrawing) {
						physics.createConstraints(myParam.pParticles, myParam.rParticles, 
							myVar.constraintAfterDrawingFlag, myVar);
					}

					for (size_t i = 0; i < myParam.pParticles.size(); i++) {
						myParam.rParticles[i].isBeingDrawn = false;
					}
					myVar.isBrushDrawing = false;
				}
			}
		}


		if (IsKeyReleased(KEY_ONE) && myVar.isDragging) {

			// VISIBLE MATTER

			for (int i = 0; i < static_cast<int>(40000 * particleAmountMultiplier); i++) {

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

				glm::vec2 vel = tangent * speed;

				float finalMass = 0.0f;

				if (massMultiplierEnabled) {
					finalMass = 8500000000.0f / particleAmountMultiplier;
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
				for (int i = 0; i < static_cast<int>(12000 * DMAmountMultiplier); i++) {
					glm::vec2 galaxyCenter = myParam.myCamera.mouseWorldPos;

					float outerRadius = 2000.0f;
					float radiusCore = 3.5f;

					float normalizedRand = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

					float radiusMultiplier = radiusCore * sqrt(static_cast<float>(pow(1 + pow(outerRadius / radiusCore, 2), normalizedRand) - 1));

					float angle = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2 * PI;

					glm::vec2 pos = glm::vec2(galaxyCenter.x + radiusMultiplier * cos(angle), galaxyCenter.y + radiusMultiplier * sin(angle));

					glm::vec2 vel = glm::vec2(static_cast<float>(rand() % 60 - 30), static_cast<float>(rand() % 60 - 30));

					float finalMass = 0.0f;

					if (massMultiplierEnabled) {
						finalMass = 141600000000.0f / DMAmountMultiplier;
					}
					else {
						finalMass = 141600000000.0f;
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

		if (IsKeyReleased(KEY_TWO) && myVar.isDragging) {

			// VISIBLE MATTER

			for (int i = 0; i < static_cast<int>(12000 * particleAmountMultiplier); i++) {
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

				float speed = 10.5f * sqrt(505.0f / (finalRadius + 54.7f));

				glm::vec2 vel = tangent * speed;

				float finalMass = 0.0f;

				if (massMultiplierEnabled) {
					finalMass = 8500000000.0f / particleAmountMultiplier;
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
				for (int i = 0; i < static_cast<int>(3600 * DMAmountMultiplier); i++) {
					glm::vec2 galaxyCenter = myParam.myCamera.mouseWorldPos;

					float outerRadius = 2000.0f;
					float radiusCore = 3.5f;

					float normalizedRand = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

					float radiusMultiplier = radiusCore * sqrt(static_cast<float>(pow(1 + pow(outerRadius / radiusCore, 2), normalizedRand) - 1));

					float angle = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2 * PI;

					glm::vec2 pos = glm::vec2(galaxyCenter.x + radiusMultiplier * cos(angle), galaxyCenter.y + radiusMultiplier * sin(angle));

					glm::vec2 vel = glm::vec2(static_cast<float>(rand() % 60 - 30), static_cast<float>(rand() % 60 - 30));

					float finalMass = 0.0f;

					if (massMultiplierEnabled) {
						finalMass = 141600000000.0f / DMAmountMultiplier;
					}
					else {
						finalMass = 141600000000.0f;
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

		if (IsKeyReleased(KEY_THREE)) {

			for (int i = 0; i < static_cast<int>(10000 * particleAmountMultiplier); i++) {

				float angle = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * 3.14159f;
				float distance = (sqrt(static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 5.0f) + 0.1f;

				glm::vec2 randomOffset = {
					cos(angle) * distance,
					sin(angle) * distance
				};

				glm::vec2 particlePos = myParam.myCamera.mouseWorldPos + randomOffset;

				float finalMass = 0.0f;

				if (massMultiplierEnabled) {
					finalMass = 8500000000.0f / particleAmountMultiplier;
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

		if (IO::shortcutPress(KEY_FOUR)) {

			// VISIBLE MATTER

			for (int i = 0; i < static_cast<int>(40000 * particleAmountMultiplier); i++) {
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

				if (massMultiplierEnabled) {
					finalMass = 8500000000.0f / particleAmountMultiplier;
				}
				else {
					finalMass = 8500000000.0f;
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
				for (int i = 0; i < static_cast<int>(12000 * DMAmountMultiplier); i++) {

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

					if (massMultiplierEnabled) {
						finalMass = 141600000000.0f / DMAmountMultiplier;
					}
					else {
						finalMass = 141600000000.0f;
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
	}
}

void ParticlesSpawning::copyPaste(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles,
	bool& isDragging, SceneCamera& myCamera, std::vector<ParticlePhysics>& pParticlesSelected, Physics& physics, 
	UpdateVariables& myVar, UpdateParameters& myParam) {

	if (IO::shortcutPress(KEY_H) && pParticlesSelected.size() > 0) {

		pParticlesCopied.clear();
		rParticlesCopied.clear();
		constraintsCopied.clear();

		for (size_t i = 0; i < pParticles.size(); i++) {
			if (rParticles[i].isSelected) {
				pParticlesCopied.push_back(ParticlePhysics(pParticles[i]));
				rParticlesCopied.push_back(ParticleRendering(rParticles[i]));
			}
		}

		avgPos = { 0.0f, 0.0f };

		for (ParticlePhysics& pCopy : pParticlesCopied) {

			avgPos += pCopy.pos;
		}

		avgPos /= static_cast<float>(pParticlesCopied.size());
	}

	Slingshot slingshot = slingshot.particleSlingshot(isDragging, myCamera);

	if (IsKeyReleased(KEY_J)) {

		for (ParticlePhysics pCopy : pParticlesCopied) {

			glm::vec2 copyRelPos = pCopy.pos - avgPos;

			pCopy.pos = myCamera.mouseWorldPos + copyRelPos;

			pCopy.vel += slingshot.length * slingshot.norm * 0.3f;
			pCopy.prevVel = pCopy.vel;

			pCopy.ke = 0.0f;
			pCopy.prevKe = 0.0f;

			pCopy.id = globalId++;

			pParticles.push_back(ParticlePhysics(pCopy));
		}

		for (ParticleRendering rCopy : rParticlesCopied) {

			rCopy.isSelected = false;

			rCopy.isBeingDrawn = true;

			rParticles.push_back(ParticleRendering(rCopy));
		}

		NeighborSearch::idToI(myParam.pParticles);
		myParam.neighborSearch.neighborSearchHash(myParam.pParticles, myParam.rParticles);

		bool enabled = true;
		physics.createConstraints(myParam.pParticles, myParam.rParticles, enabled, myVar);

		for (size_t i = 0; i < myParam.pParticles.size(); i++) {
			myParam.rParticles[i].isBeingDrawn = false;
		}

		isDragging = false;
	}
}

void ParticlesSpawning::predictTrajectory(const std::vector<ParticlePhysics>& pParticles,
	SceneCamera& myCamera, Physics physics, Quadtree* quadtree,
	UpdateVariables& myVar, Slingshot& slingshot) {

	if (!IsMouseButtonDown(0)) {
		return;
	}

	std::vector<ParticlePhysics> currentParticles = pParticles;

	ParticlePhysics predictedParticle(
		glm::vec2(myCamera.mouseWorldPos),
		glm::vec2{ slingshot.norm * slingshot.length },
		heavyParticleInitMass * heavyParticleWeightMultiplier,

		1.0f,
		1.0f,
		1.0f,
		1.0f
	);

	currentParticles.push_back(predictedParticle);

	int predictedIndex = static_cast<int>(currentParticles.size()) - 1;

	std::vector<glm::vec2> predictedPath;

	for (int step = 0; step < predictPathLength; ++step) {
		ParticlePhysics& p = currentParticles[predictedIndex];

		glm::vec2 netForce = physics.calculateForceFromGrid(*quadtree, currentParticles, myVar, p);

		glm::vec2 acc;
		acc = netForce / p.mass;

		p.vel += myVar.timeFactor * (1.5f * acc);

		p.pos += p.vel * myVar.timeFactor;

		predictedPath.push_back(p.pos);
	}

	for (size_t i = 1; i < predictedPath.size(); ++i) {
		DrawLineV({ predictedPath[i - 1].x,  predictedPath[i - 1].y }, { predictedPath[i].x,  predictedPath[i].y }, WHITE);
	}
}
