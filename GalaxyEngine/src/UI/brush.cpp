#include "../../include/UI/brush.h"
#include "../../include/parameters.h"
#include <cmath>

struct SPHWater water;
struct SPHRock rock;
struct SPHSand sand;
struct SPHMud mud;
struct SPHAir air;

Brush::Brush(SceneCamera myCamera, float brushRadius) {
	this->myCamera = myCamera;
	this->brushRadius = brushRadius;
	mouseWorldPos = { 0.0f, 0.0f };
}

void Brush::brushLogic(UpdateParameters& myParam, bool& isSPHEnabled) {

	if (!isSPHEnabled) {
		SPHWater = false;
		SPHRock = false;
		SPHSand = false;
		SPHMud = false;
	}

	if (!SPHWater && !SPHRock && !SPHSand && !SPHMud) {
		for (int i = 0; i < static_cast<int>(140 * myParam.particlesSpawning.particleAmountMultiplier); i++) {
			float angle = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * 3.14159f;
			float distance = sqrt(static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * brushRadius;

			Vector2 randomOffset = {
				cos(angle) * distance,
				sin(angle) * distance
			};

			Vector2 particlePos = Vector2Add(myParam.myCamera.mouseWorldPos, randomOffset);

			myParam.pParticles.emplace_back(particlePos,
				Vector2{ 0, 0 },
				8500000000.0f / myParam.particlesSpawning.particleAmountMultiplier,

				0.008f,
				1.0f,
				1.0f,
				1.0f);

			myParam.rParticles.emplace_back(Color{ 128, 128, 128, 100 }, 0.125f, false, false, false, true, true, false, true, -1.0f);
		}
	}

	if (isSPHEnabled) {
		if (SPHWater) {
			for (int i = 0; i < static_cast<int>(140 * myParam.particlesSpawning.particleAmountMultiplier); i++) {
				float angle = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * 3.14159f;
				float distance = sqrt(static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * brushRadius;

				Vector2 randomOffset = {
					cos(angle) * distance,
					sin(angle) * distance
				};

				Vector2 particlePos = Vector2Add(myParam.myCamera.mouseWorldPos, randomOffset);

				myParam.pParticles.emplace_back(particlePos,
					Vector2{ 0, 0 },
					(8500000000.0f * water.massMult) / myParam.particlesSpawning.particleAmountMultiplier,

					water.restDens,
					water.stiff,
					water.visc,
					water.cohesion);

				myParam.rParticles.emplace_back(water.color, 0.125f, false, false, false, true, true, false, true, -1.0f);

			}
		}

		if (SPHRock) {
			for (int i = 0; i < static_cast<int>(140 * myParam.particlesSpawning.particleAmountMultiplier); i++) {
				float angle = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * 3.14159f;
				float distance = sqrt(static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * brushRadius;

				Vector2 randomOffset = {
					cos(angle) * distance,
					sin(angle) * distance
				};

				Vector2 particlePos = Vector2Add(myParam.myCamera.mouseWorldPos, randomOffset);

				myParam.pParticles.emplace_back(particlePos,
					Vector2{ 0, 0 },
					(8500000000.0f * rock.massMult) / myParam.particlesSpawning.particleAmountMultiplier,

					rock.restDens,
					rock.stiff,
					rock.visc,
					rock.cohesion);

				float normalRand = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
				auto addRandom = [&](unsigned char c) -> unsigned char {
					float value = static_cast<float>(c) + (50.0f * normalRand) - 25.0f;
					value = std::clamp(value, 0.0f, 255.0f);
					return static_cast<unsigned char>(value);
					};

				myParam.rParticles.emplace_back(
					Color{
						addRandom(rock.color.r),
						addRandom(rock.color.g),
						addRandom(rock.color.b),
						addRandom(rock.color.a)
					},

					0.125f,
					false, 
					false,
					false,
					true, 
					true, 
					false,
					true, 
					-1.0f
				);

			}
		}

		if (SPHSand) {
			for (int i = 0; i < static_cast<int>(140 * myParam.particlesSpawning.particleAmountMultiplier); i++) {
				float angle = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * 3.14159f;
				float distance = sqrt(static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * brushRadius;

				Vector2 randomOffset = {
					cos(angle) * distance,
					sin(angle) * distance
				};

				Vector2 particlePos = Vector2Add(myParam.myCamera.mouseWorldPos, randomOffset);

				myParam.pParticles.emplace_back(particlePos,
					Vector2{ 0, 0 },
					(8500000000.0f * sand.massMult) / myParam.particlesSpawning.particleAmountMultiplier,

					sand.restDens,
					sand.stiff,
					sand.visc,
					sand.cohesion);

				float normalRand = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
				auto addRandom = [&](unsigned char c) -> unsigned char {
					float value = static_cast<float>(c) + (50.0f * normalRand) - 25.0f;
					value = std::clamp(value, 0.0f, 255.0f);
					return static_cast<unsigned char>(value);
					};

				myParam.rParticles.emplace_back(
					Color{
						addRandom(sand.color.r),
						addRandom(sand.color.g),
						addRandom(sand.color.b),
						addRandom(sand.color.a)
					},
					
					0.125f,
					false,
					false,
					false,
					true, 
					true,
					false, 
					true,
					-1.0f
				);

			}
		}

		if (SPHMud) {
			for (int i = 0; i < static_cast<int>(140 * myParam.particlesSpawning.particleAmountMultiplier); i++) {
				float angle = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * 3.14159f;
				float distance = sqrt(static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * brushRadius;

				Vector2 randomOffset = {
					cos(angle) * distance,
					sin(angle) * distance
				};

				Vector2 particlePos = Vector2Add(myParam.myCamera.mouseWorldPos, randomOffset);

				myParam.pParticles.emplace_back(particlePos,
					Vector2{ 0, 0 },
					(8500000000.0f * mud.massMult) / myParam.particlesSpawning.particleAmountMultiplier,

					mud.restDens,
					mud.stiff,
					mud.visc,
					mud.cohesion);

				float normalRand = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
				auto addRandom = [&](unsigned char c) -> unsigned char {
					float value = static_cast<float>(c) + (50.0f * normalRand) - 25.0f;
					value = std::clamp(value, 0.0f, 255.0f);
					return static_cast<unsigned char>(value);
					};

				myParam.rParticles.emplace_back(
					Color{
						addRandom(mud.color.r),
						addRandom(mud.color.g),
						addRandom(mud.color.b),
						addRandom(mud.color.a)
					},
					
					0.125f, 
					false,
					false,
					false,
					true, 
					true,
					false, 
					true, 
					-1.0f
				);

			}
		}
	}
}

void Brush::brushSize() {
	float wheel = GetMouseWheelMove();
	if (IsKeyDown(KEY_LEFT_CONTROL) && wheel != 0) {
		float scale = 0.2f * wheel;
		brushRadius = Clamp(expf(logf(brushRadius) + scale), 7.0f, 512.0f);
	}
}

void Brush::drawBrush(Vector2 mouseWorldPos) {
	DrawCircleLinesV({ mouseWorldPos.x, mouseWorldPos.y }, brushRadius, WHITE);
}

void Brush::eraseBrush(UpdateParameters& myParam) {

	if (IsKeyDown(KEY_X) && IsMouseButtonDown(2)) {
		for (size_t i = 0; i < myParam.pParticles.size();) {
			Vector2 distanceFromBrush = {
				myParam.pParticles[i].pos.x - myParam.myCamera.mouseWorldPos.x,
				myParam.pParticles[i].pos.y - myParam.myCamera.mouseWorldPos.y
			};

			float distance = sqrt(distanceFromBrush.x * distanceFromBrush.x +
				distanceFromBrush.y * distanceFromBrush.y);

			if (distance < brushRadius) {
				std::swap(myParam.pParticles[i], myParam.pParticles.back());
				std::swap(myParam.rParticles[i], myParam.rParticles.back());

				myParam.pParticles.pop_back();
				myParam.rParticles.pop_back();
			}
			else {
				i++;
			}
		}
	}
}

void Brush::particlesAttractor(UpdateVariables& myVar, UpdateParameters& myParam) {

	if (IsKeyDown(KEY_B)) {

		for (size_t i = 0; i < myParam.pParticles.size(); i++) {
			float dx = myParam.pParticles[i].pos.x - myParam.myCamera.mouseWorldPos.x;
			float dy = myParam.pParticles[i].pos.y - myParam.myCamera.mouseWorldPos.y;
			float distance = sqrt(dx * dx + dy * dy);

			float innerRadius = 0.0f;
			float outerRadius = brushRadius;

			float falloffFactor = 0.0f;
			if (distance > innerRadius) {
				falloffFactor = std::min(0.7f, (distance - innerRadius) / (outerRadius - innerRadius));

				falloffFactor = falloffFactor * falloffFactor;
			}

			float radiusMultiplier = 0.0f;

			if (distance < 1.0f) {
				radiusMultiplier = 1.0f;
			}
			else {
				radiusMultiplier = distance;
			}


			float acceleration = static_cast<float>(myVar.G * 600.0f * brushRadius * brushRadius) / (radiusMultiplier * radiusMultiplier);

			acceleration *= falloffFactor;

			attractorForce.x = static_cast<float>(-(dx / radiusMultiplier) * acceleration * myParam.pParticles[i].mass);
			attractorForce.y = static_cast<float>(-(dy / radiusMultiplier) * acceleration * myParam.pParticles[i].mass);

			if (IsKeyDown(KEY_LEFT_CONTROL)) {
				attractorForce = { -attractorForce.x, -attractorForce.y };
			}

			myParam.pParticles[i].vel.x += attractorForce.x * myVar.timeFactor;
			myParam.pParticles[i].vel.y += attractorForce.y * myVar.timeFactor;
		}
	}
}

void Brush::particlesSpinner(UpdateVariables& myVar, UpdateParameters& myParam) {

	if (IsKeyDown(KEY_N)) {
		for (auto& pParticle : myParam.pParticles) {
			Vector2 distanceFromBrush = { pParticle.pos.x - myParam.myCamera.mouseWorldPos.x, pParticle.pos.y - myParam.myCamera.mouseWorldPos.y };
			float distance = sqrt(distanceFromBrush.x * distanceFromBrush.x + distanceFromBrush.y * distanceFromBrush.y);
			if (distance < brushRadius) {

				float falloff = distance / brushRadius;
				falloff = powf(falloff, 2.0f);

				float inverseDistance = 1.0f / (distance + myVar.softening);
				Vector2 radialDirection = { distanceFromBrush.x * inverseDistance, distanceFromBrush.y * inverseDistance };
				Vector2 spinDirection = { -radialDirection.y, radialDirection.x };

				if (IsKeyDown(KEY_LEFT_CONTROL)) {
					spinDirection = { -spinDirection.x, -spinDirection.y };
				}

				pParticle.vel.x += spinDirection.x * spinForce * falloff * myVar.timeFactor;
				pParticle.vel.y += spinDirection.y * spinForce * falloff * myVar.timeFactor;
			}
		}
	}

}

void Brush::particlesGrabber(UpdateParameters& myParam) {

	Vector2 mouseDelta = GetMouseDelta();
	Vector2 scaledDelta = Vector2Scale(mouseDelta, 1.0f / myParam.myCamera.camera.zoom);

	lastMouseVelocity = scaledDelta;

	if (IsKeyPressed(KEY_M)) {
		dragging = true;

		for (size_t i = 0; i < myParam.pParticles.size(); i++) {
			Vector2 distanceFromBrush = {
				myParam.pParticles[i].pos.x - myParam.myCamera.mouseWorldPos.x,
				myParam.pParticles[i].pos.y - myParam.myCamera.mouseWorldPos.y
			};

			float distance = sqrt(distanceFromBrush.x * distanceFromBrush.x +
				distanceFromBrush.y * distanceFromBrush.y);
			if (distance < brushRadius) {
				myParam.rParticles[i].isGrabbed = true;
			}
		}
	}

	if (dragging) {
		for (size_t i = 0; i < myParam.pParticles.size(); i++) {
			if (myParam.rParticles[i].isGrabbed) {

				myParam.pParticles[i].pos.x += scaledDelta.x;
				myParam.pParticles[i].pos.y += scaledDelta.y;

				myParam.pParticles[i].acc = { 0.0f, 0.0f };
				myParam.pParticles[i].vel = { 0.0f, 0.0f };
			}
		}
	}

	if (IsKeyReleased(KEY_M)) {

		float impulseFactor = 5.0f;
		for (size_t i = 0; i < myParam.pParticles.size(); i++) {
			if (myParam.rParticles[i].isGrabbed) {

				myParam.pParticles[i].vel.x = lastMouseVelocity.x * impulseFactor;
				myParam.pParticles[i].vel.y = lastMouseVelocity.y * impulseFactor;

				myParam.rParticles[i].isGrabbed = false;
			}
		}
		dragging = false;
	}
}

