#include "../../include/Particles/particlesSpawning.h"
#include "../../include/Physics/physics.h"
#include "../../include/Physics/quadtree.h"
#include "../../include/parameters.h"
#include <cmath>


void ParticlesSpawning::particlesInitialConditions(Quadtree* quadtree, Physics& physics, UpdateVariables& myVar, UpdateParameters& myParam) {

	if (myVar.isMouseNotHoveringUI && isSpawningAllowed) {

		Slingshot slingshot = slingshot.planetSlingshot(myVar.isDragging, myParam.myCamera);

		if (myVar.isDragging && enablePathPrediction && quadtree != nullptr) {
			predictTrajectory(myParam.pParticles, myParam.myCamera, physics, quadtree, myVar, slingshot);
		}

		if (IsMouseButtonReleased(0) && !IsKeyDown(KEY_LEFT_CONTROL) && !IsKeyDown(KEY_LEFT_ALT) && myVar.isDragging) {
			myParam.pParticles.emplace_back(
				Vector2{ static_cast<float>(myParam.myCamera.mouseWorldPos.x), static_cast<float>(myParam.myCamera.mouseWorldPos.y) },
				Vector2{ slingshot.normalizedX * slingshot.length, slingshot.normalizedY * slingshot.length },
				heavyParticleInitMass * heavyParticleWeightMultiplier
			);
			myParam.rParticles.emplace_back(
				Color{ 255, 255, 255, 255 },
				0.3f,
				true,
				false,
				true,
				false,
				false,
				false
			);
			myVar.isDragging = false;
		}
		if (IsMouseButtonDown(2) && !IsKeyDown(KEY_LEFT_CONTROL) && !IsKeyDown(KEY_LEFT_ALT) && !IsKeyDown(KEY_X)) {
			myParam.brush.brushLogic(myParam);
		}

		if (IsKeyReleased(KEY_ONE) && myVar.isDragging) {

			// VISIBLE MATTER

			for (int i = 0; i < 40000; i++) {
				float galaxyCenterX = static_cast<float>(myParam.myCamera.mouseWorldPos.x);
				float galaxyCenterY = static_cast<float>(myParam.myCamera.mouseWorldPos.y);

				float outerRadius = 200.0f;
				float innerRadius = -1.0f;

				float scaleLength = 90.0f;

				float normalizedRand = static_cast<float>(rand()) / RAND_MAX;

				float angle = static_cast<float>(rand()) / RAND_MAX * 2 * PI;

				float randRadius = outerRadius * normalizedRand;

				//float curveSteepness = -0.023f;
				//float finalRadius = innerRadius + (outerRadius - innerRadius) / (1.0f + exp(-curveSteepness * (randRadius)));

				float finalRadius = -scaleLength * log(1.0f - normalizedRand);

				finalRadius = std::min(finalRadius, outerRadius + 500.0f);

				finalRadius = std::max(finalRadius, 0.01f);

				float posX = galaxyCenterX + finalRadius * cos(angle);
				float posY = galaxyCenterY + finalRadius * sin(angle);

				float dx = posX - galaxyCenterX;
				float dy = posY - galaxyCenterY;

				float tangentX = dy;
				float tangentY = -dx;

				float length = sqrt(tangentX * tangentX + tangentY * tangentY);
				tangentX /= length;
				tangentY /= length;

				float speed = 10.5f * sqrt(1758.0f / (finalRadius + 54.7f));

				float velocityX = tangentX * speed;
				float velocityY = tangentY * speed;

				myParam.pParticles.emplace_back(
					Vector2{ posX, posY },
					Vector2{ velocityX + (slingshot.normalizedX * slingshot.length * 0.3f),
						velocityY + (slingshot.normalizedY * slingshot.length * 0.3f) },
					8500000000.0f
				);
				myParam.rParticles.emplace_back(
					Color{ 128, 128, 128, 100 },
					0.125f,
					false,
					false,
					false,
					true,
					true,
					false
				);
			}

			// DARK MATTER

			if (myVar.isDarkMatterEnabled) {
				for (int i = 0; i < 12000; i++) {
					float galaxyCenterX = static_cast<float>(myParam.myCamera.mouseWorldPos.x);
					float galaxyCenterY = static_cast<float>(myParam.myCamera.mouseWorldPos.y);

					float outerRadius = 2000.0f;
					float radiusCore = 3.5f;

					float normalizedRand = static_cast<float>(rand()) / RAND_MAX;

					float radius = radiusCore * sqrt(static_cast<float>(pow(1 + pow(outerRadius / radiusCore, 2), normalizedRand) - 1));

					float angle = static_cast<float>(rand()) / RAND_MAX * 2 * PI;

					float posX = galaxyCenterX + radius * cos(angle);
					float posY = galaxyCenterY + radius * sin(angle);

					float velocityX = static_cast<float>(rand() % 60 - 30);
					float velocityY = static_cast<float>(rand() % 60 - 30);

					myParam.pParticles.emplace_back(
						Vector2{ posX, posY },
						Vector2{
							velocityX + (slingshot.normalizedX * slingshot.length * 0.3f),
							velocityY + (slingshot.normalizedY * slingshot.length * 0.3f)
						},
						141600000000.0f
					);
					myParam.rParticles.emplace_back(
						Color{ 128, 128, 128, 0 },
						0.125f,
						true,
						false,
						false,
						false,
						true,
						true
					);
				}
			}

			myVar.isDragging = false;
		}

		if (IsKeyReleased(KEY_TWO) && myVar.isDragging) {

			// VISIBLE MATTER

			for (int i = 0; i < 12000; i++) {
				float galaxyCenterX = static_cast<float>(myParam.myCamera.mouseWorldPos.x);
				float galaxyCenterY = static_cast<float>(myParam.myCamera.mouseWorldPos.y);

				float outerRadius = 100.0f;
				float innerRadius = -1.0f;

				float scaleLength = 45.0f;

				float normalizedRand = static_cast<float>(rand()) / RAND_MAX;

				float angle = static_cast<float>(rand()) / RAND_MAX * 2 * PI;

				float randRadius = outerRadius * normalizedRand;

				//float curveSteepness = -0.023f;
				//float finalRadius = innerRadius + (outerRadius - innerRadius) / (1.0f + exp(-curveSteepness * (randRadius)));

				float finalRadius = -scaleLength * log(1.0f - normalizedRand);

				finalRadius = std::min(finalRadius, outerRadius + 250.0f);

				finalRadius = std::max(finalRadius, 0.01f);

				float posX = galaxyCenterX + finalRadius * cos(angle);
				float posY = galaxyCenterY + finalRadius * sin(angle);

				float dx = posX - galaxyCenterX;
				float dy = posY - galaxyCenterY;

				float tangentX = dy;
				float tangentY = -dx;

				float length = sqrt(tangentX * tangentX + tangentY * tangentY);
				tangentX /= length;
				tangentY /= length;

				float speed = 10.5f * sqrt(505.0f / (finalRadius + 54.7f));

				float velocityX = tangentX * speed;
				float velocityY = tangentY * speed;

				myParam.pParticles.emplace_back(
					Vector2{ posX, posY },
					Vector2{
						velocityX + (slingshot.normalizedX * slingshot.length * 0.3f),
						velocityY + (slingshot.normalizedY * slingshot.length * 0.3f)
					},
					8500000000.0f
				);
				myParam.rParticles.emplace_back(
					Color{ 128, 128, 128, 100 },
					0.125f,
					false,
					false,
					false,
					true,
					true,
					false
				);
			}

			// DARK MATTER
			if (myVar.isDarkMatterEnabled) {
				for (int i = 0; i < 3600; i++) {
					float galaxyCenterX = static_cast<float>(myParam.myCamera.mouseWorldPos.x);
					float galaxyCenterY = static_cast<float>(myParam.myCamera.mouseWorldPos.y);

					float outerRadius = 2000.0f;
					float radiusCore = 3.5f;

					float normalizedRand = static_cast<float>(rand()) / RAND_MAX;

					float radius = radiusCore * sqrt(static_cast<float>(pow(1 + pow(outerRadius / radiusCore, 2), normalizedRand) - 1));

					float angle = static_cast<float>(rand()) / RAND_MAX * 2 * PI;

					float posX = galaxyCenterX + radius * cos(angle);
					float posY = galaxyCenterY + radius * sin(angle);

					float velocityX = static_cast<float>(rand() % 60 - 30);
					float velocityY = static_cast<float>(rand() % 60 - 30);

					myParam.pParticles.emplace_back(
						Vector2{ posX, posY },
						Vector2{
							velocityX + (slingshot.normalizedX * slingshot.length * 0.3f),
							velocityY + (slingshot.normalizedY * slingshot.length * 0.3f)
						},
						141600000000.0f
					);
					myParam.rParticles.emplace_back(
						Color{ 128, 128, 128, 0 },
						0.125f,
						true,
						false,
						false,
						false,
						true,
						true
					);
				}
			}
			myVar.isDragging = false;
		}

		if (IsKeyReleased(KEY_THREE)) {

			for (int i = 0; i < 10000; i++) {

				float angle = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * 3.14159f;
				float distance = (sqrt(static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 5.0f) + 0.1f;

				Vector2 randomOffset = {
					cos(angle) * distance,
					sin(angle) * distance
				};

				Vector2 particlePos = Vector2Add(myParam.myCamera.mouseWorldPos, randomOffset);

				float dx = particlePos.x - myParam.myCamera.mouseWorldPos.x;
				float dy = particlePos.y - myParam.myCamera.mouseWorldPos.y;

				myParam.pParticles.emplace_back(
					Vector2{ particlePos.x, particlePos.y },
					Vector2{ (slingshot.normalizedX * slingshot.length * 0.3f),  +(slingshot.normalizedY * slingshot.length * 0.3f) },
					8500000000.0f
				);
				myParam.rParticles.emplace_back(
					Color{ 128, 128, 128, 100 },
					0.125f,
					false,
					false,
					false,
					true,
					true,
					false
				);
			}

			myVar.isDragging = false;
		}

		if (IsKeyPressed(KEY_FOUR)) {

			// VISIBLE MATTER

			for (int i = 0; i < 40000; i++) {
				float angle = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * 3.14159f;
				float distance = (sqrt(static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 20.0f) + 1.0f;

				Vector2 randomOffset = {
					cos(angle) * distance + static_cast<float>(GetRandomValue(-15, 15)),
					sin(angle) * distance + static_cast<float>(GetRandomValue(-15, 15))
				};

				Vector2 particlePos = Vector2Add(myParam.myCamera.mouseWorldPos, randomOffset);

				float dx = particlePos.x - myParam.myCamera.mouseWorldPos.x;
				float dy = particlePos.y - myParam.myCamera.mouseWorldPos.y;

				float normalX = dx / distance;
				float normalY = dy / distance;

				float speed = 300.0f;

				float adjustedSpeed = speed * (distance / 35.0f);

				Vector2 velocity = { adjustedSpeed * normalX, adjustedSpeed * normalY };

				myParam.pParticles.emplace_back(
					Vector2{ particlePos.x, particlePos.y },
					Vector2{ velocity.x, velocity.y },
					8500000000.0f
				);
				myParam.rParticles.emplace_back(
					Color{ 128, 128, 128, 100 },
					0.125f,
					false,
					false,
					false,
					true,
					true,
					false
				);
			}

			// DARK MATTER

			if (myVar.isDarkMatterEnabled) {
				for (int i = 0; i < 12000; i++) {

					float angle = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * 3.14159f;
					float distance = (sqrt(static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 20.0f) + 1.0f;

					Vector2 randomOffset = {
						cos(angle) * distance + static_cast<float>(GetRandomValue(-15, 15)),
						sin(angle) * distance + static_cast<float>(GetRandomValue(-15, 15))
					};

					Vector2 particlePos = Vector2Add(myParam.myCamera.mouseWorldPos, randomOffset);

					float dx = particlePos.x - myParam.myCamera.mouseWorldPos.x;
					float dy = particlePos.y - myParam.myCamera.mouseWorldPos.y;

					float normalX = dx / distance;
					float normalY = dy / distance;

					float speed = 300.0f;

					float adjustedSpeed = speed * (distance / 35.0f);

					Vector2 velocity = { adjustedSpeed * normalX, adjustedSpeed * normalY };

					myParam.pParticles.emplace_back(
						Vector2{ particlePos.x, particlePos.y },
						Vector2{ velocity.x, velocity.y },
						141600000000.0f
					);
					myParam.rParticles.emplace_back(
						Color{ 128, 128, 128, 0 },
						0.125f,
						true,
						false,
						false,
						false,
						true,
						true
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

void ParticlesSpawning::predictTrajectory(const std::vector<ParticlePhysics>& pParticles,
	SceneCamera& myCamera, Physics physics, Quadtree* quadtree,
	UpdateVariables& myVar, Slingshot& slingshot) {

	if (!IsMouseButtonDown(0)) {
		return;
	}

	std::vector<ParticlePhysics> currentParticles = pParticles;

	ParticlePhysics predictedParticle(
		Vector2{ static_cast<float>(myCamera.mouseWorldPos.x), static_cast<float>(myCamera.mouseWorldPos.y) },
		Vector2{ slingshot.normalizedX * slingshot.length, slingshot.normalizedY * slingshot.length },
		heavyParticleInitMass * heavyParticleWeightMultiplier
	);

	predictedParticle.prevAcc = Vector2{ 0.0f, 0.0f };

	currentParticles.push_back(predictedParticle);

	int predictedIndex = static_cast<int>(currentParticles.size()) - 1;

	std::vector<Vector2> predictedPath;

	for (int step = 0; step < predictPathLength; ++step) {
		ParticlePhysics& p = currentParticles[predictedIndex];

		Vector2 netForce = physics.calculateForceFromGrid(*quadtree, currentParticles, myVar, p);

		Vector2 acc;
		acc.x = netForce.x / p.mass;
		acc.y = netForce.y / p.mass;

		p.velocity.x += myVar.timeFactor * ((3.0f / 2.0f) * acc.x);
		p.velocity.y += myVar.timeFactor * ((3.0f / 2.0f) * acc.y);

		p.pos.x += p.velocity.x * myVar.timeFactor;
		p.pos.y += p.velocity.y * myVar.timeFactor;

		predictedPath.push_back(p.pos);
	}

	for (size_t i = 1; i < predictedPath.size(); ++i) {
		DrawLineV(predictedPath[i - 1], predictedPath[i], WHITE);
	}
}
