#include "Physics/armorMode.h"

#include "parameters.h"

extern SPHWater water;
extern SPHRock rock;
extern SPHIron iron;
extern SPHSand sand;
extern SPHSoil soil;
extern SPHMud mud;
extern SPHRubber rubber;

void ArmorMode::boxDrawTool(UpdateParameters& myParam, UpdateVariables& myVar, Physics& physics) {
	if (!wasArmorEnabled && myVar.isArmorModeEnabled) {
		originalConstraintsEnabled = myVar.constraintsEnabled;
		originalGravityMultiplier = myVar.gravityMultiplier;

		myVar.drawConstraints = true;
	}

	if (myVar.isArmorModeEnabled) {
		myVar.constraintsEnabled = true;
		myVar.gravityMultiplier = 0.0f;
	}
	else if (wasArmorEnabled && !myVar.isArmorModeEnabled) {
		myVar.constraintsEnabled = originalConstraintsEnabled;
		myVar.gravityMultiplier = originalGravityMultiplier;
	}

	wasArmorEnabled = myVar.isArmorModeEnabled;

	glm::vec2 dir = glm::normalize(b - a);

	float dist = glm::distance(a, b);

	glm::vec2 tan = { dir.y, -dir.x };

	if (isDrawingRect) {

		if (setB && !startThicknessFlag) {
			DrawLineV({ a.x, a.y }, { myVar.mouseWorldPos.x,myVar.mouseWorldPos.y }, RED);
		}
		else if (startThicknessFlag) {
			DrawLineV({ a.x, a.y }, { b.x, b.y }, RED);

			float mouseDist = glm::dot(myVar.mouseWorldPos - b, tan);

			if (mouseDist >= 0.0f) {
				mouseDist = glm::max(mouseDist, spacing * 2.2f);
			}
			else {
				mouseDist = glm::min(mouseDist, -spacing * 2.2f);
			}

			c = { b.x + mouseDist * tan.x, b.y + mouseDist * tan.y };

			glm::vec2 d = { a.x + mouseDist * tan.x, a.y + mouseDist * tan.y };

			DrawLineV({ b.x, b.y }, { c.x, c.y }, RED);

			DrawLineV({ a.x, a.y }, { d.x, d.y }, RED);

			DrawLineV({ c.x, c.y }, { d.x, d.y }, RED);
		}
	}

	if (disableBoxFlag) {
		setB = false;
		isDrawingRect = false;
		startThicknessFlag = false;
		disableBoxFlag = false;
	}

	if (IO::mouseReleased(1) && isDrawingRect) {

		disableBoxFlag = true;
	}

	if (IO::mousePress(0) && !IO::shortcutDown(KEY_X)) {
		if (!startThicknessFlag && !setB) {
			a = myVar.mouseWorldPos;
			setB = true;

			isDrawingRect = true;
		}
		else if (!startThicknessFlag) {
			b = myVar.mouseWorldPos;
			startThicknessFlag = true;
			setB = false;
		}
		else {

			glm::vec2 tanContribution = myVar.mouseWorldPos * tan;

			float thickness = glm::distance(b, c);

			int amount = int(dist / spacing);
			int thickAmount = int(thickness / spacing);

			if (amount > 1) {

				for (int i = 0; i < amount; i++) {
					for (int j = 0; j < thickAmount; j++) {

						float finalMass = 0.0f;

						if (myParam.particlesSpawning.massMultiplierEnabled) {
							finalMass = (8500000000.0f * iron.massMult) / myVar.particleAmountMultiplier;
						}
						else {
							finalMass = (8500000000.0f * iron.massMult);
						}

						glm::vec2 thickDir = glm::normalize(c - b);

						glm::vec2 pos = { a.x + (dir.x * i * spacing) + (thickDir.x * j * spacing), a.y + (dir.y * i * spacing) + (thickDir.y * j * spacing) };

						myParam.pParticles.emplace_back(pos,
							glm::vec2(0.0f, 0.0f),
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

						myParam.rParticles.back().isBeingDrawn = true;
					}
				}

				if (!myVar.hasAVX2) {
					myParam.neighborSearchV2.newGrid(myParam.pParticles);
					myParam.neighborSearchV2.neighborAmount(myParam.pParticles, myParam.rParticles);
				}
				else {
					myParam.neighborSearchV2AVX2.newGridAVX2(myParam.pParticles);
					myParam.neighborSearchV2AVX2.neighborAmount(myParam.pParticles, myParam.rParticles);
				}

				bool createConstraints = true;

				physics.createConstraints(myParam.pParticles, myParam.rParticles,
					createConstraints, myVar, myParam);

				for (size_t i = 0; i < myParam.pParticles.size(); i++) {
					if (myParam.rParticles[i].isBeingDrawn) {
						myParam.pParticles[i].vel = { 0.0f, 0.0f };
						myParam.pParticles[i].acc = { 0.0f, 0.0f };
					}

					myParam.rParticles[i].isBeingDrawn = false;
				}

				startThicknessFlag = false;
				setB = false;
				isDrawingRect = false;
			}
			else {
				startThicknessFlag = false;
				setB = false;
				isDrawingRect = false;
			}
		}
	}
}

void ArmorMode::circleTool(UpdateParameters& myParam, UpdateVariables& myVar, Physics& physics, float& brushSize) {

	if (!wasArmorEnabled && myVar.isArmorModeEnabled) {
		originalConstraintsEnabled = myVar.constraintsEnabled;
		originalGravityMultiplier = myVar.gravityMultiplier;

		myVar.drawConstraints = true;
	}

	if (myVar.isArmorModeEnabled) {
		myVar.constraintsEnabled = true;
		myVar.gravityMultiplier = 0.0f;
	}
	else if (wasArmorEnabled && !myVar.isArmorModeEnabled) {
		myVar.constraintsEnabled = originalConstraintsEnabled;
		myVar.gravityMultiplier = originalGravityMultiplier;
	}

	wasArmorEnabled = myVar.isArmorModeEnabled;

	if (disableCircleFlag) {
		setCircleRadius = false;
		spawnCircleFlag = false;
		disableCircleFlag = false;
	}

	if (IO::mouseReleased(1) && setCircleRadius) {
		disableCircleFlag = true;
	}

	if (spawnCircleFlag) {

		int amountY = int((circleRadius * 2.0f) / spacingCircle);

		for (int y = 0; y < amountY; y++) {

			float posY = (circleA.y - circleRadius) + (y * spacingCircle) + spacingCircle * 0.5f;

			float d = glm::abs(circleA.y - posY);

			float chordLength = 2.0f * glm::sqrt((circleRadius * circleRadius) - (d * d));

			int amountX = int(chordLength / spacingCircle);

			for (int x = 0; x < amountX; x++) {

				float posX = 0.0f;

				if (x < amountX / 2) {
					posX = circleA.x - (x * spacingCircle) - (spacingCircle * 0.5f);
				}
				else {
					posX = circleA.x + ((x - amountX / 2 ) * spacingCircle) + (spacingCircle * 0.5f);
				}

				float finalMass = 0.0f;

				if (myParam.particlesSpawning.massMultiplierEnabled) {
					finalMass = (8500000000.0f * iron.massMult) / myVar.particleAmountMultiplier;
				}
				else {
					finalMass = (8500000000.0f * iron.massMult);
				}

				myParam.pParticles.emplace_back(glm::vec2(posX, posY),
					glm::vec2(0.0f, 0.0f),
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

				myParam.rParticles.back().isBeingDrawn = true;
			}
		}

		if (!myVar.hasAVX2) {
			myParam.neighborSearchV2.newGrid(myParam.pParticles);
			myParam.neighborSearchV2.neighborAmount(myParam.pParticles, myParam.rParticles);
		}
		else {
			myParam.neighborSearchV2AVX2.newGridAVX2(myParam.pParticles);
			myParam.neighborSearchV2AVX2.neighborAmount(myParam.pParticles, myParam.rParticles);
		}

		bool createConstraints = true;

		physics.createConstraints(myParam.pParticles, myParam.rParticles,
			createConstraints, myVar, myParam);

		for (size_t i = 0; i < myParam.pParticles.size(); i++) {
			if (myParam.rParticles[i].isBeingDrawn) {
				myParam.pParticles[i].vel = { 0.0f, 0.0f };
				myParam.pParticles[i].acc = { 0.0f, 0.0f };
			}

			myParam.rParticles[i].isBeingDrawn = false;
		}

		setCircleRadius = false;
		spawnCircleFlag = false;
	}

	if (setCircleRadius) {
		DrawLineV({ circleA.x, circleA.y }, { myVar.mouseWorldPos.x, myVar.mouseWorldPos.y }, RED);

		circleRadius = glm::distance(circleA, myVar.mouseWorldPos);

		DrawCircleLinesV({ circleA.x, circleA.y }, circleRadius, RED);

		if (IO::mousePress(0) && !IO::shortcutDown(KEY_X)) {
			spawnCircleFlag = true;
		}
	}
	else {
		if (IO::mousePress(0) && !IO::shortcutDown(KEY_X)) {
			setCircleRadius = true;
			circleA = myVar.mouseWorldPos;
		}
	}
}