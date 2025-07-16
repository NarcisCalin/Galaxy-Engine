#include "UI/brush.h"

#include "parameters.h"

struct SPHWater water;
struct SPHRock rock;
struct SPHIron iron;
struct SPHSand sand;
struct SPHSoil soil;
struct SPHMud mud;
struct SPHRubber rubber;

Brush::Brush(SceneCamera myCamera, float brushRadius) {
	this->myCamera = myCamera;
	this->brushRadius = brushRadius;
	mouseWorldPos = { 0.0f, 0.0f };
}

void Brush::brushLogic(UpdateParameters& myParam, bool& isSPHEnabled, bool& constraintAfterDrawing) {

	// This entire function is a crime against programming and perhaps humanity as well. I don't know what destiny shall await for whoever reads such cursed code. 
	// But I'm too lazy to change this for now. Will change it some time in the future

	if (!isSPHEnabled) {
		SPHWater = false;
		SPHRock = false;
		SPHIron = false;
		SPHSand = false;
		SPHSoil = false;
		SPHIce = false;
		SPHMud = false;
		SPHRubber = false;
	}

	if (!SPHWater && !SPHRock && !SPHSand && !SPHSoil && !SPHIce && !SPHMud && !SPHGas && !SPHIron && !SPHRubber) {
		for (int i = 0; i < static_cast<int>(140 * myParam.particlesSpawning.particleAmountMultiplier); i++) {
			float angle = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * 3.14159f;
			float distance = sqrt(static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * brushRadius;

			glm::vec2 randomOffset = {
				cos(angle) * distance,
				sin(angle) * distance
			};

			glm::vec2 particlePos = myParam.myCamera.mouseWorldPos + randomOffset;

			float finalMass = 0.0f;

			if (myParam.particlesSpawning.massMultiplierEnabled) {
				finalMass = 8500000000.0f / myParam.particlesSpawning.particleAmountMultiplier;
			}
			else {
				finalMass = 8500000000.0f;
			}

			myParam.pParticles.emplace_back(particlePos,
				glm::vec2{ 0, 0 },
				finalMass,

				0.008f,
				1.0f,
				1.0f,
				1.0f);

			myParam.rParticles.emplace_back(Color{ 128, 128, 128, 100 }, 0.125f, false, false, false, true, true, false, true, -1.0f, 0);

			if (isSPHEnabled) {
				myParam.rParticles.back().spawnCorrectIter = 0;
				myParam.rParticles.back().isBeingDrawn = true;
			}
			else {
				myParam.rParticles.back().spawnCorrectIter = 10000000;
			myParam.rParticles.back().isBeingDrawn = false;
			}
		}
	}

	if (isSPHEnabled) {
		if (SPHWater) {
			for (int i = 0; i < static_cast<int>(140 * myParam.particlesSpawning.particleAmountMultiplier); i++) {
				float angle = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * 3.14159f;
				float distance = sqrt(static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * brushRadius;

				glm::vec2 randomOffset = {
					cos(angle) * distance,
					sin(angle) * distance
				};

				glm::vec2 particlePos = myParam.myCamera.mouseWorldPos + randomOffset;

				float finalMass = 0.0f;

				if (myParam.particlesSpawning.massMultiplierEnabled) {
					finalMass = (8500000000.0f * water.massMult) / myParam.particlesSpawning.particleAmountMultiplier;
				}
				else {
					finalMass = (8500000000.0f * water.massMult);
				}

				myParam.pParticles.emplace_back(particlePos,
					glm::vec2{ 0, 0 },
					finalMass,

					water.restDens,
					water.stiff,
					water.visc,
					water.cohesion);

				myParam.rParticles.emplace_back(water.color, 0.125f, false, false, false, true, true, false, true, -1.0f, water.id);

				myParam.rParticles.back().sphColor = water.color;

				myParam.rParticles.back().spawnCorrectIter = 0;

				myParam.rParticles.back().isBeingDrawn = true;
			}
		}

		if (SPHRock) {
			for (int i = 0; i < static_cast<int>(140 * myParam.particlesSpawning.particleAmountMultiplier); i++) {
				float angle = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * 3.14159f;
				float distance = sqrt(static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * brushRadius;

				glm::vec2 randomOffset = {
					cos(angle) * distance,
					sin(angle) * distance
				};

				glm::vec2 particlePos = myParam.myCamera.mouseWorldPos + randomOffset;

				float finalMass = 0.0f;

				if (myParam.particlesSpawning.massMultiplierEnabled) {
					finalMass = (8500000000.0f * rock.massMult) / myParam.particlesSpawning.particleAmountMultiplier;
				}
				else {
					finalMass = (8500000000.0f * rock.massMult);
				}

				myParam.pParticles.emplace_back(particlePos,
					glm::vec2{ 0, 0 },
					finalMass,

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
						rock.color.a
					},

					0.125f,
					false, 
					false,
					false,
					true, 
					true, 
					false,
					true, 
					-1.0f,
					rock.id
				);

				myParam.rParticles.back().sphColor = Color{
						addRandom(rock.color.r),
						addRandom(rock.color.g),
						addRandom(rock.color.b),
						rock.color.a
				};

				myParam.rParticles.back().spawnCorrectIter = 0;

				myParam.rParticles.back().isBeingDrawn = true;
			}
		}

		if (SPHIron) {
			for (int i = 0; i < static_cast<int>(140 * myParam.particlesSpawning.particleAmountMultiplier); i++) {
				float angle = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * 3.14159f;
				float distance = sqrt(static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * brushRadius;

				glm::vec2 randomOffset = {
					cos(angle) * distance,
					sin(angle) * distance
				};

				glm::vec2 particlePos = myParam.myCamera.mouseWorldPos + randomOffset;

				float finalMass = 0.0f;

				if (myParam.particlesSpawning.massMultiplierEnabled) {
					finalMass = (8500000000.0f * iron.massMult) / myParam.particlesSpawning.particleAmountMultiplier;
				}
				else {
					finalMass = (8500000000.0f * iron.massMult);
				}

				myParam.pParticles.emplace_back(particlePos,
					glm::vec2{ 0, 0 },
					finalMass,

					iron.restDens,
					iron.stiff,
					iron.visc,
					iron.cohesion);

				float normalRand = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
				auto addRandom = [&](unsigned char c) -> unsigned char {
					float value = static_cast<float>(c) + (40.0f * normalRand) - 20.0f;
					value = std::clamp(value, 0.0f, 255.0f);
					return static_cast<unsigned char>(value);
					};

				myParam.rParticles.emplace_back(
					Color{
						addRandom(iron.color.r),
						addRandom(iron.color.g),
						addRandom(iron.color.b),
						iron.color.a
					},

					0.125f,
					false,
					false,
					false,
					true,
					true,
					false,
					true,
					-1.0f,
					iron.id
				);

				myParam.rParticles.back().sphColor = Color{
						addRandom(iron.color.r),
						addRandom(iron.color.g),
						addRandom(iron.color.b),
						iron.color.a
				};

				myParam.rParticles.back().spawnCorrectIter = 0;

				myParam.rParticles.back().isBeingDrawn = true;
			}
		}

		if (SPHSand) {
			for (int i = 0; i < static_cast<int>(140 * myParam.particlesSpawning.particleAmountMultiplier); i++) {
				float angle = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * 3.14159f;
				float distance = sqrt(static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * brushRadius;

				glm::vec2 randomOffset = {
					cos(angle) * distance,
					sin(angle) * distance
				};

				glm::vec2 particlePos = myParam.myCamera.mouseWorldPos + randomOffset;

				float finalMass = 0.0f;

				if (myParam.particlesSpawning.massMultiplierEnabled) {
					finalMass = (8500000000.0f * sand.massMult) / myParam.particlesSpawning.particleAmountMultiplier;
				}
				else {
					finalMass = (8500000000.0f * sand.massMult);
				}

				myParam.pParticles.emplace_back(particlePos,
					glm::vec2{ 0, 0 },
					finalMass,

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
						sand.color.a
					},
					
					0.125f,
					false,
					false,
					false,
					true, 
					true,
					false, 
					true,
					-1.0f,
					sand.id
				);

				myParam.rParticles.back().sphColor = Color{
						addRandom(sand.color.r),
						addRandom(sand.color.g),
						addRandom(sand.color.b),
						sand.color.a
				};

				myParam.rParticles.back().spawnCorrectIter = 0;

				myParam.rParticles.back().isBeingDrawn = true;
			}
		}

		if (SPHSoil) {
			for (int i = 0; i < static_cast<int>(140 * myParam.particlesSpawning.particleAmountMultiplier); i++) {
				float angle = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * 3.14159f;
				float distance = sqrt(static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * brushRadius;

				glm::vec2 randomOffset = {
					cos(angle) * distance,
					sin(angle) * distance
				};

				glm::vec2 particlePos = myParam.myCamera.mouseWorldPos + randomOffset;

				float finalMass = 0.0f;

				if (myParam.particlesSpawning.massMultiplierEnabled) {
					finalMass = (8500000000.0f * soil.massMult) / myParam.particlesSpawning.particleAmountMultiplier;
				}
				else {
					finalMass = (8500000000.0f * soil.massMult);
				}

				myParam.pParticles.emplace_back(particlePos,
					glm::vec2{ 0, 0 },
					finalMass,

					soil.restDens,
					soil.stiff,
					soil.visc,
					soil.cohesion);

				float normalRand = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
				auto addRandom = [&](unsigned char c) -> unsigned char {
					float value = static_cast<float>(c) + (50.0f * normalRand) - 25.0f;
					value = std::clamp(value, 0.0f, 255.0f);
					return static_cast<unsigned char>(value);
					};

				myParam.rParticles.emplace_back(
					Color{
						addRandom(soil.color.r),
						addRandom(soil.color.g),
						addRandom(soil.color.b),
						soil.color.a
					},
					
					0.125f, 
					false,
					false,
					false,
					true, 
					true,
					false, 
					true, 
					-1.0f,
					soil.id
				);

				myParam.rParticles.back().sphColor = Color{
						addRandom(soil.color.r),
						addRandom(soil.color.g),
						addRandom(soil.color.b),
						soil.color.a
				};

				myParam.rParticles.back().spawnCorrectIter = 0;

				myParam.rParticles.back().isBeingDrawn = true;
			}
		}

		if (SPHIce) {
			for (int i = 0; i < static_cast<int>(140 * myParam.particlesSpawning.particleAmountMultiplier); i++) {
				float angle = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * 3.14159f;
				float distance = sqrt(static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * brushRadius;

				glm::vec2 randomOffset = {
					cos(angle) * distance,
					sin(angle) * distance
				};

				glm::vec2 particlePos = myParam.myCamera.mouseWorldPos + randomOffset;

				float finalMass = 0.0f;

				if (myParam.particlesSpawning.massMultiplierEnabled) {
					finalMass = (8500000000.0f * water.massMult) / myParam.particlesSpawning.particleAmountMultiplier;
				}
				else {
					finalMass = (8500000000.0f * water.massMult);
				}

				myParam.pParticles.emplace_back(particlePos,
					glm::vec2{ 0, 0 },
					finalMass,

					water.restDens,
					water.stiff,
					water.visc,
					water.cohesion);

				myParam.rParticles.emplace_back(water.color, 0.125f, false, false, false, true, true, false, true, -1.0f, water.id);

				myParam.rParticles.back().sphColor = water.color;
				myParam.pParticles.back().temp = 1.0f;

				myParam.rParticles.back().spawnCorrectIter = 0;

				myParam.rParticles.back().isBeingDrawn = true;
			}
		}

		if (SPHMud) {
			for (int i = 0; i < static_cast<int>(140 * myParam.particlesSpawning.particleAmountMultiplier); i++) {
				float angle = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * 3.14159f;
				float distance = sqrt(static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * brushRadius;

				glm::vec2 randomOffset = {
					cos(angle) * distance,
					sin(angle) * distance
				};

				glm::vec2 particlePos = myParam.myCamera.mouseWorldPos + randomOffset;

				float finalMass = 0.0f;

				if (myParam.particlesSpawning.massMultiplierEnabled) {
					finalMass = (8500000000.0f * mud.massMult) / myParam.particlesSpawning.particleAmountMultiplier;
				}
				else {
					finalMass = (8500000000.0f * mud.massMult);
				}

				myParam.pParticles.emplace_back(particlePos,
					glm::vec2{ 0, 0 },
					finalMass,

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
						mud.color.a
					},

					0.125f,
					false,
					false,
					false,
					true,
					true,
					false,
					true,
					-1.0f,
					mud.id
				);

				myParam.rParticles.back().sphColor = Color{
						addRandom(mud.color.r),
						addRandom(mud.color.g),
						addRandom(mud.color.b),
						mud.color.a
				};

				myParam.rParticles.back().spawnCorrectIter = 0;

				myParam.rParticles.back().isBeingDrawn = true;
			}
		}

		if (SPHRubber) {
			for (int i = 0; i < static_cast<int>(140 * myParam.particlesSpawning.particleAmountMultiplier); i++) {
				float angle = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * 3.14159f;
				float distance = sqrt(static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * brushRadius;

				glm::vec2 randomOffset = {
					cos(angle) * distance,
					sin(angle) * distance
				};

				glm::vec2 particlePos = myParam.myCamera.mouseWorldPos + randomOffset;

				float finalMass = 0.0f;

				if (myParam.particlesSpawning.massMultiplierEnabled) {
					finalMass = (8500000000.0f * rubber.massMult) / myParam.particlesSpawning.particleAmountMultiplier;
				}
				else {
					finalMass = (8500000000.0f * rubber.massMult);
				}

				myParam.pParticles.emplace_back(particlePos,
					glm::vec2{ 0, 0 },
					finalMass,

					rubber.restDens,
					rubber.stiff,
					rubber.visc,
					rubber.cohesion);

				float normalRand = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
				auto addRandom = [&](unsigned char c) -> unsigned char {
					float value = static_cast<float>(c) + (40.0f * normalRand) - 20.0f;
					value = std::clamp(value, 0.0f, 255.0f);
					return static_cast<unsigned char>(value);
					};

				myParam.rParticles.emplace_back(
					Color{
						addRandom(rubber.color.r),
						addRandom(rubber.color.g),
						addRandom(rubber.color.b),
						rubber.color.a
					},

					0.125f,
					false,
					false,
					false,
					true,
					true,
					false,
					true,
					-1.0f,
					rubber.id
				);

				myParam.rParticles.back().sphColor = Color{
						addRandom(rubber.color.r),
						addRandom(rubber.color.g),
						addRandom(rubber.color.b),
						rubber.color.a
				};

				myParam.rParticles.back().spawnCorrectIter = 0;

				myParam.rParticles.back().isBeingDrawn = true;
			}
		}

		if (SPHGas) {
			for (int i = 0; i < static_cast<int>(140 * myParam.particlesSpawning.particleAmountMultiplier); i++) {
				float angle = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * 3.14159f;
				float distance = sqrt(static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * brushRadius;

				glm::vec2 randomOffset = {
					cos(angle) * distance,
					sin(angle) * distance
				};

				glm::vec2 particlePos = myParam.myCamera.mouseWorldPos + randomOffset;

				float finalMass = 0.0f;

				if (myParam.particlesSpawning.massMultiplierEnabled) {
					finalMass = (8500000000.0f * water.massMult) / myParam.particlesSpawning.particleAmountMultiplier;
				}
				else {
					finalMass = (8500000000.0f * water.massMult);
				}

				myParam.pParticles.emplace_back(particlePos,
					glm::vec2{ 0, 0 },
					finalMass,

					water.restDens,
					water.stiff,
					water.visc,
					water.cohesion);

				myParam.rParticles.emplace_back(water.color, 0.125f, false, false, false, true, true, false, true, -1.0f, water.id);

				myParam.rParticles.back().sphColor = water.color;
				myParam.pParticles.back().temp = 440.0f;

				myParam.rParticles.back().spawnCorrectIter = 0;

				myParam.rParticles.back().isBeingDrawn = true;
			}
		}
	}
}

void Brush::brushSize() {
	float wheel = GetMouseWheelMove();
	if (IO::shortcutDown(KEY_LEFT_CONTROL) && wheel != 0) {
		float scale = 0.2f * wheel;
		brushRadius = Clamp(expf(logf(brushRadius) + scale), 4.5f, 512.0f);
	}
}

void Brush::drawBrush(glm::vec2 mouseWorldPos) {
	DrawCircleLinesV({ mouseWorldPos.x, mouseWorldPos.y }, brushRadius, WHITE);
}

void Brush::eraseBrush(UpdateVariables& myVar, UpdateParameters& myParam) {

	if ((IO::shortcutDown(KEY_X) && IO::mouseDown(2)) || IO::mouseDown(0) && myVar.toolErase) {
		for (size_t i = 0; i < myParam.pParticles.size();) {
			glm::vec2 distanceFromBrush = {
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

	if (IO::shortcutDown(KEY_B) || (IO::mouseDown(0) && myVar.toolRadialForce)) {

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

			if (IO::shortcutDown(KEY_LEFT_CONTROL)) {
				attractorForce = { -attractorForce.x, -attractorForce.y };
			}

			myParam.pParticles[i].vel.x += attractorForce.x * myVar.timeFactor;
			myParam.pParticles[i].vel.y += attractorForce.y * myVar.timeFactor;
		}
	}
}

void Brush::particlesSpinner(UpdateVariables& myVar, UpdateParameters& myParam) {

	if (IO::shortcutDown(KEY_N) || (IO::mouseDown(0) && myVar.toolSpin)) {
		for (auto& pParticle : myParam.pParticles) {
			glm::vec2 distanceFromBrush = { pParticle.pos.x - myParam.myCamera.mouseWorldPos.x, pParticle.pos.y - myParam.myCamera.mouseWorldPos.y };
			float distance = sqrt(distanceFromBrush.x * distanceFromBrush.x + distanceFromBrush.y * distanceFromBrush.y);
			if (distance < brushRadius) {

				float falloff = distance / brushRadius;
				falloff = powf(falloff, 2.0f);

				float inverseDistance = 1.0f / (distance + myVar.softening);
				glm::vec2 radialDirection = { distanceFromBrush.x * inverseDistance, distanceFromBrush.y * inverseDistance };
				glm::vec2 spinDirection = { -radialDirection.y, radialDirection.x };

				if (IO::shortcutDown(KEY_LEFT_CONTROL)) {
					spinDirection = { -spinDirection.x, -spinDirection.y };
				}

				pParticle.vel.x += spinDirection.x * spinForce * falloff * myVar.timeFactor;
				pParticle.vel.y += spinDirection.y * spinForce * falloff * myVar.timeFactor;
			}
		}
	}

}

void Brush::particlesGrabber(UpdateVariables& myVar, UpdateParameters& myParam) {

	glm::vec2 mouseDelta = glm::vec2(GetMouseDelta().x, GetMouseDelta().y);
	glm::vec2 scaledDelta = mouseDelta * (1.0f / myParam.myCamera.camera.zoom);

	lastMouseVelocity = scaledDelta;

	if (IO::shortcutPress(KEY_M) || (IO::mousePress(0) && myVar.toolMove)) {
		dragging = true;

		for (size_t i = 0; i < myParam.pParticles.size(); i++) {
			glm::vec2 distanceFromBrush = { myParam.pParticles[i].pos - myParam.myCamera.mouseWorldPos };

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
				myParam.pParticles[i].prevVel = { 0.0f, 0.0f };
			}
		}
	}

	if (IO::shortcutReleased(KEY_M) || (IO::mouseReleased(0) && myVar.toolMove)) {

		float impulseFactor = 5.0f;
		for (size_t i = 0; i < myParam.pParticles.size(); i++) {
			if (myParam.rParticles[i].isGrabbed) {

				myParam.pParticles[i].vel = lastMouseVelocity * impulseFactor;
				myParam.pParticles[i].prevVel = myParam.pParticles[i].vel;


				myParam.rParticles[i].isGrabbed = false;
			}
		}
		dragging = false;
	}
}

void Brush::temperatureBrush(UpdateVariables& myVar, UpdateParameters& myParam) {

	if (IO::shortcutDown(KEY_K) || IO::shortcutDown(KEY_L) || (IO::mouseDown(0) && myVar.toolRaiseTemp) || (IO::mouseDown(0) && myVar.toolLowerTemp)) {
		for (size_t i = 0; i < myParam.pParticles.size(); i++) {
			glm::vec2 distanceFromBrush = { myParam.pParticles[i].pos - myParam.myCamera.mouseWorldPos };

			float distance = sqrt(distanceFromBrush.x * distanceFromBrush.x +
				distanceFromBrush.y * distanceFromBrush.y);

			if(IO::shortcutDown(KEY_K) || (IO::mouseDown(0) && myVar.toolRaiseTemp)){
				if (distance < brushRadius) {
					myParam.pParticles[i].temp += 40.0f;
				}
			}

			if (IO::shortcutDown(KEY_L) || (IO::mouseDown(0) && myVar.toolLowerTemp)) {
				if (distance < brushRadius && myParam.pParticles[i].temp > 1.0f) {
					myParam.pParticles[i].temp -= 40.0f;
				}
			}
		}
	}
}

