#include "particlesSpawning.h"
#include "physics.h"
#include "quadtree.h"
#include "parameters.h"


void ParticlesSpawning::particlesInitialConditions(Quadtree& quadtree, Physics& physics, UpdateVariables& myVar, UpdateParameters& myParam) {

	if (myVar.isMouseNotHoveringUI && isSpawningAllowed) {

		Slingshot slingshot = slingshot.planetSlingshot(myVar.isDragging, myParam.myCamera);

		if (myVar.isDragging && enablePathPrediction) {
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
				true,
				false,
				true,
				false,
				false
			);
			myVar.isDragging = false;
		}
		if (IsMouseButtonDown(2) && !IsKeyDown(KEY_LEFT_CONTROL) && !IsKeyDown(KEY_LEFT_ALT) && !IsKeyDown(KEY_X)) {
			myParam.brush.brushLogic(myParam);
		}

		if (IsKeyPressed(KEY_ONE) && !myVar.isDragging) {
			for (int i = 0; i < 40000; i++) {
				float galaxyCenterX = static_cast<float>(myVar.screenWidth / 2);
				float galaxyCenterY = static_cast<float>(myVar.screenHeight / 2);

				float angle = static_cast<float>(rand()) / RAND_MAX * 2 * PI;
				float radius = static_cast<float>(rand()) / RAND_MAX * 200.0f + 8;

				float posX = galaxyCenterX + radius * cos(angle);
				float posY = galaxyCenterY + radius * sin(angle);

				float dx = posX - galaxyCenterX;
				float dy = posY - galaxyCenterY;

				float angularSpeed = 130 / (radius + 60);
				float velocityX = -dy * angularSpeed;
				float velocityY = dx * angularSpeed;

				myParam.pParticles.emplace_back(
					Vector2{ posX, posY },
					Vector2{ velocityX, velocityY },
					50000000000.0f
				);
				myParam.rParticles.emplace_back(
					Color{ 128, 128, 128, 100 },
					0.125f,
					false,
					true,
					false,
					false,
					true,
					true
				);

			}
		}

		if (IsKeyReleased(KEY_THREE) && myVar.isDragging) {
			for (int i = 0; i < 12000; i++) {
				float galaxyCenterX = static_cast<float>(myParam.myCamera.mouseWorldPos.x);
				float galaxyCenterY = static_cast<float>(myParam.myCamera.mouseWorldPos.y);

				float angle = static_cast<float>(rand()) / RAND_MAX * 2 * PI;

				float u = static_cast<float>(rand()) / RAND_MAX;
				float biasedU = pow(u, 2.0f);
				float radius = biasedU * 100.0f + 2;

				float posX = galaxyCenterX + radius * cos(angle);
				float posY = galaxyCenterY + radius * sin(angle);

				float dx = posX - galaxyCenterX;
				float dy = posY - galaxyCenterY;

				float baseAngularSpeed = 150 / (radius + 120);
				float scalingFactor = 0.5f;
				float angularSpeed = baseAngularSpeed * scalingFactor;

				float velocityX = -dy * angularSpeed;
				float velocityY = dx * angularSpeed;

				myParam.pParticles.emplace_back(
					Vector2{ posX, posY },
					Vector2{
						velocityX + (slingshot.normalizedX * slingshot.length * 0.3f),
						velocityY + (slingshot.normalizedY * slingshot.length * 0.3f)
					},
					85000000000.0f
				);
				myParam.rParticles.emplace_back(
					Color{ 128, 128, 128, 100 },
					0.125f,
					false,
					true,
					false,
					false,
					true,
					true
				);
				myVar.isDragging = false;
			}
		}

		if (IsKeyPressed(KEY_TWO)) {
			for (int i = 0; i < 10000; i++) {
				myParam.pParticles.emplace_back(
					Vector2{ static_cast<float>(rand() % myVar.screenWidth), static_cast<float>(rand() % myVar.screenHeight) },
					Vector2{ 0, 0 },
					500000000000.0f
				);
				myParam.rParticles.emplace_back(
					Color{ 128, 128, 128, 100 },
					0.125f,
					false,
					true,
					false,
					false,
					true,
					true
				);
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
	SceneCamera& myCamera, Physics physics, Quadtree& quadtree,
	UpdateVariables& myVar, Slingshot& slingshot) {

	if (!IsMouseButtonDown(0)) {
		return;
	}

	std::vector<ParticlePhysics> currentParticles = pParticles;

	ParticlePhysics predictedParticle(
		Vector2{ static_cast<float>(myCamera.mouseWorldPos.x), static_cast<float>(myCamera.mouseWorldPos.y) },
		Vector2{ slingshot.normalizedX * slingshot.length, slingshot.normalizedY * slingshot.length },
		heavyParticleInitMass* heavyParticleWeightMultiplier
	);

	predictedParticle.prevAcc = Vector2{ 0.0f, 0.0f };

	currentParticles.push_back(predictedParticle);

	int predictedIndex = currentParticles.size() - 1;

	std::vector<Vector2> predictedPath;

	for (int step = 0; step < predictPathLength; ++step) {
		ParticlePhysics& p = currentParticles[predictedIndex];

		Vector2 netForce = physics.calculateForceFromGrid(quadtree, currentParticles, myVar, p);
		if (myVar.isDarkMatterEnabled) {
			Vector2 dmForce = physics.darkMatterForce(p, myVar);
			netForce.x += dmForce.x;
			netForce.y += dmForce.y;
		}

		Vector2 acc;
		acc.x = netForce.x / p.mass;
		acc.y = netForce.y / p.mass;

		p.velocity.x += myVar.timeFactor * ((3.0f / 2.0f) * acc.x - (1.0f / 2.0f) * p.prevAcc.x);
		p.velocity.y += myVar.timeFactor * ((3.0f / 2.0f) * acc.y - (1.0f / 2.0f) * p.prevAcc.y);

		p.pos.x += p.velocity.x * myVar.timeFactor;
		p.pos.y += p.velocity.y * myVar.timeFactor;

		p.prevAcc = acc;

		predictedPath.push_back(p.pos);
	}

	for (size_t i = 1; i < predictedPath.size(); ++i) {
		DrawLineV(predictedPath[i - 1], predictedPath[i], WHITE);
	}
}
