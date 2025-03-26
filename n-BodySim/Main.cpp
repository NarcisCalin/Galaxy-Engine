#include <iostream>
#include <vector>
#include "raylib.h"
#include <cmath>
#include <array>
#include <omp.h>
#include <thread>
#include <algorithm>

#include "slingshot.h"
#include "quadtree.h"
#include "planet.h"
#include "mouseTrailDot.h"
#include "button.h"
#include "screenCapture.h"
#include "morton.h"
#include "slider.h"
#include "camera.h"
#include "rlgl.h"
#include "raymath.h"
#include "brush.h"

int screenWidth = 1024;
int screenHeight = 1024;
float screenRatioX;
float screenRatioY;

int targetFPS = 144;

double G = 6.674e-11;
float gravityMultiplier = 1.0f;
float softening = 2.0f;
float theta = 0.7f;
float timeStepMultiplier = 1.0f;
const float fixedDeltaTime = 0.03f;

float timeFactor;

bool trailsEnabled = false;
bool isPixelDrawingEnabled = false;
bool isPeriodicBoundaryEnabled = true;
bool isMultiThreadingEnabled = true;
bool isBarnesHutEnabled = true;
bool isDarkMatterEnabled = false;
bool isCollisionsEnabled = false;

bool solidColor = false;
bool densityColor = true;
bool velocityColor = false;

bool isSpawningAllowed = true;

int blendMode = 1;

int densityR = 200;
int densityG = 128;
int densityB = 40;
int densityA = 255;

float densityRadius = 2.4f;
int maxNeighbors = 58;



static Quadtree gridFunction(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles) {
	Quadtree grid(
		0.0f,
		0.0f,
		static_cast<float>(screenWidth),
		0,
		static_cast<int>(pParticles.size()),
		pParticles,
		rParticles,
		nullptr
	);

	grid.calculateMasses(pParticles);

	return grid;
}

bool isMouse0Pressed;
bool isMouse2ControlPressed;
bool isDragging = false;

int planetIndex = 0;


static Vector2 calculateForceFromGrid(ParticlePhysics& pParticle,
	const Quadtree& grid,
	const std::vector<ParticlePhysics>& pParticles) {
	Vector2 totalForce = { 0.0f, 0.0f };

	if (grid.gridMass <= 0)
		return totalForce;

	float dx = grid.centerOfMass.x - pParticle.pos.x;
	float dy = grid.centerOfMass.y - pParticle.pos.y;


	if (isPeriodicBoundaryEnabled) {
		if (dx > screenWidth * 0.5f)
			dx -= screenWidth;
		else if (dx < -screenWidth * 0.5f)
			dx += screenWidth;

		if (dy > screenHeight * 0.5f)
			dy -= screenHeight;
		else if (dy < -screenHeight * 0.5f)
			dy += screenHeight;
	}

	float distanceSq = dx * dx + dy * dy + softening * softening;

	float distance = sqrt(distanceSq);

	if (grid.size / distance < theta || grid.subGrids.empty()) {

		if ((grid.endIndex - grid.startIndex) == 1 &&
			fabs(pParticles[grid.startIndex].pos.x - pParticle.pos.x) < 0.001f &&
			fabs(pParticles[grid.startIndex].pos.y - pParticle.pos.y) < 0.001f) {
			return totalForce;
		}
		float force = static_cast<float>(G * pParticle.mass * grid.gridMass / distanceSq);
		totalForce.x = (dx / distance) * force;
		totalForce.y = (dy / distance) * force;
	}
	else {
		for (const auto& subGridPtr : grid.subGrids) {
			Vector2 childForce = calculateForceFromGrid(pParticle, *subGridPtr, pParticles);
			totalForce.x += childForce.x;
			totalForce.y += childForce.y;
		}
	}
	return totalForce;
}


struct DarkMatterHalo {
	Vector2 pos;
	double mass;
	double radius;

	DarkMatterHalo(Vector2 position = { 0.0f, 0.0f },
		double m = 1e19,
		double r = 200.0)
		: pos(position), mass(m), radius(r) {
	}
};


static Vector2 darkMatterForce(const ParticlePhysics& pParticles) {
	float centerX = screenWidth / 2.0f;
	float centerY = screenHeight / 2.0f;

	float dx = pParticles.pos.x - centerX;
	float dy = pParticles.pos.y - centerY;
	float radius = sqrt(dx * dx + dy * dy);
	if (radius < 1.0f) radius = 1.0f;

	const double haloMass = 7e17;
	const float haloRadius = 650.0;

	float concentration = 10;
	float r_ratio = radius / haloRadius;
	float M_enclosed = static_cast<float>(haloMass * (log(1 + r_ratio) - r_ratio / (1 + r_ratio)))
		/ (log(1 + concentration) - concentration / (1 + concentration));

	float acceleration = static_cast<float>(G * M_enclosed) / (radius * radius);

	Vector2 force;
	force.x = static_cast<float>(-(dx / radius) * acceleration * pParticles.mass);
	force.y = static_cast<float>(-(dy / radius) * acceleration * pParticles.mass);

	return force;
}


void pairWiseGravity(std::vector<ParticlePhysics>& pParticles) {

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

			if (isPeriodicBoundaryEnabled) {
				if (isPeriodicBoundaryEnabled) {
					if (dx > screenWidth / 2)
						dx -= screenWidth;
					else if (dx < -screenWidth / 2)
						dx += screenWidth;

					if (dy > screenHeight / 2)
						dy -= screenHeight;
					else if (dy < -screenHeight / 2)
						dy += screenHeight;
				}
			}

			float distanceSq = dx * dx + dy * dy + softening * softening;

			float distance = sqrt(distanceSq);
			float force = static_cast<float>(G * pParticleA.mass * pParticleB.mass / distanceSq);

			float fx = (dx / distance) * force;
			float fy = (dy / distance) * force;

			accelPlanetAX = fx / pParticleA.mass;
			accelPlanetAY = fy / pParticleA.mass;


			accelPlanetBX = fx / pParticleB.mass;
			accelPlanetBY = fy / pParticleB.mass;

			pParticleA.velocity.x += (fixedDeltaTime * ((3.0f / 2.0f)) * accelPlanetAX - ((1.0f / 2.0f)) * prevAccAX) * timeStepMultiplier;
			pParticleA.velocity.y += (fixedDeltaTime * ((3.0f / 2.0f)) * accelPlanetAY - ((1.0f / 2.0f)) * prevAccAY) * timeStepMultiplier;

			pParticleB.velocity.x -= (fixedDeltaTime * ((3.0f / 2.0f)) * accelPlanetBX - ((1.0f / 2.0f)) * prevAccBX) * timeStepMultiplier;
			pParticleB.velocity.y -= (fixedDeltaTime * ((3.0f / 2.0f)) * accelPlanetBY - ((1.0f / 2.0f)) * prevAccBY) * timeStepMultiplier;

		}
	}
}

Vector2 mouseWorldPos;
static void updateScene(std::vector<ParticlePhysics>& pParticles,
	std::vector<ParticleRendering>& rParticles, bool& isMouseNotHoveringUI, SceneCamera& myCamera, Brush& brush) {

	G = 6.674e-11 * gravityMultiplier;

	timeFactor = fixedDeltaTime * timeStepMultiplier;

	Quadtree grid = gridFunction(pParticles, rParticles);

	mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), myCamera.camera);

	brush.brushSize(mouseWorldPos);

	if (isMouseNotHoveringUI && isSpawningAllowed) {
		Slingshot slingshot = slingshot.planetSlingshot(isDragging, isMouse0Pressed, isMouse2ControlPressed, myCamera);
		if (IsMouseButtonReleased(0) && !IsKeyDown(KEY_LEFT_CONTROL) && isDragging) {
			pParticles.emplace_back(
				Vector2{ static_cast<float>(mouseWorldPos.x), static_cast<float>(mouseWorldPos.y) },
				Vector2{ slingshot.normalizedX * slingshot.length, slingshot.normalizedY * slingshot.length },
				1000000000000000.0f
			);
			rParticles.emplace_back(
				Color{ 255, 255, 255, 255 },
				0.3f,
				true,
				true
			);
			isDragging = false;
		}
		if (IsMouseButtonDown(2)) {
			brush.brushLogic(pParticles, rParticles, mouseWorldPos);
		}

		if (IsKeyPressed(KEY_ONE) && !isDragging) {
			for (int i = 0; i < 40000; i++) {
				float galaxyCenterX = static_cast<float>(screenWidth / 2);
				float galaxyCenterY = static_cast<float>(screenHeight / 2);

				float angle = static_cast<float>(rand()) / RAND_MAX * 2 * PI;
				float radius = static_cast<float>(rand()) / RAND_MAX * 200.0f + 8;

				float posX = galaxyCenterX + radius * cos(angle);
				float posY = galaxyCenterY + radius * sin(angle);

				float dx = posX - galaxyCenterX;
				float dy = posY - galaxyCenterY;

				float angularSpeed = 130 / (radius + 60);
				float velocityX = -dy * angularSpeed;
				float velocityY = dx * angularSpeed;

				pParticles.emplace_back(
					Vector2{ posX, posY },
					Vector2{ velocityX, velocityY },
					50000000000.0f
				);
				rParticles.emplace_back(
					Color{ 128, 128, 128, 100 },
					0.125f,
					false,
					true
				);

			}
		}

		if ((IsMouseButtonReleased(0) && isDragging) || (IsKeyReleased(KEY_LEFT_CONTROL) && isDragging)) {
			for (int i = 0; i < 12000; i++) {
				float galaxyCenterX = static_cast<float>(mouseWorldPos.x);
				float galaxyCenterY = static_cast<float>(mouseWorldPos.y);

				float angle = static_cast<float>(rand()) / RAND_MAX * 2 * PI;
				float radius = static_cast<float>(rand()) / RAND_MAX * 100.0f + 2;

				float posX = galaxyCenterX + radius * cos(angle);
				float posY = galaxyCenterY + radius * sin(angle);

				float dx = posX - galaxyCenterX;
				float dy = posY - galaxyCenterY;

				float angularSpeed = 60 / (radius + 60);
				float velocityX = -dy * angularSpeed;
				float velocityY = dx * angularSpeed;

				pParticles.emplace_back(
					Vector2{ posX, posY },
					Vector2{
						velocityX + (slingshot.normalizedX * slingshot.length),
						velocityY + (slingshot.normalizedY * slingshot.length)
					},
					85000000000.0f
				);
				rParticles.emplace_back(
					Color{ 128, 128, 128, 100 },
					0.125f,
					false,
					true
				);
				isDragging = false;
			}
		}

		if (IsKeyPressed(KEY_TWO)) {
			for (int i = 0; i < 10000; i++) {
				pParticles.emplace_back(
					Vector2{ static_cast<float>(rand() % screenWidth), static_cast<float>(rand() % screenHeight) },
					Vector2{ 0, 0 },
					500000000000.0f
				);
				rParticles.emplace_back(
					Color{ 128, 128, 128, 100 },
					0.125f,
					false,
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



	if (isBarnesHutEnabled) {
#pragma omp parallel for schedule(dynamic)
		for (size_t i = 0; i < pParticles.size(); i++) {
			ParticlePhysics& pParticle = pParticles[i];

			float accX = 0;
			float accY = 0;
			float prevAccX = accX;
			float prevAccY = accY;

			Vector2 netForce = calculateForceFromGrid(pParticle, grid, pParticles);

			if (isDarkMatterEnabled) {
				Vector2 dmForce = darkMatterForce(pParticle);
				netForce.x += dmForce.x;
				netForce.y += dmForce.y;
			}

			accX = netForce.x / pParticle.mass;
			accY = netForce.y / pParticle.mass;

			pParticle.velocity.x += (timeFactor * ((3.0f / 2.0f) * accX - (1.0f / 2.0f) * prevAccX));
			pParticle.velocity.y += (timeFactor * ((3.0f / 2.0f) * accY - (1.0f / 2.0f) * prevAccY));
		}
	}
	else {
		pairWiseGravity(pParticles);
	}
#pragma omp parallel for schedule(dynamic)
	for (size_t i = 0; i < pParticles.size(); i++) {
		ParticlePhysics& pParticle = pParticles[i];
		pParticle.pos.x += pParticle.velocity.x * timeFactor;
		pParticle.pos.y += pParticle.velocity.y * timeFactor;

		if (isPeriodicBoundaryEnabled) {
			if (pParticle.pos.x < 0)
				pParticle.pos.x += screenWidth;
			else if (pParticle.pos.x >= screenWidth)
				pParticle.pos.x -= screenWidth;

			if (pParticle.pos.y < 0)
				pParticle.pos.y += screenHeight;
			else if (pParticle.pos.y >= screenHeight)
				pParticle.pos.y -= screenHeight;
		}

		if (isCollisionsEnabled) {
			for (size_t j = i + 1; j < pParticles.size(); j++) {
				ParticlePhysics& pParticleOther = pParticles[j];

				float dx = pParticle.pos.x - pParticleOther.pos.x;
				float dy = pParticle.pos.y - pParticleOther.pos.y;

				float distanceSq = dx * dx + dy * dy + softening * softening;

				float radiiSum = rParticles[i].size * 16.0f + rParticles[j].size * 16.0f;
				float radiiSumSq = radiiSum * radiiSum;

				if (distanceSq <= radiiSumSq) {
					float distance = sqrt(distanceSq);
					if (distance == 0.0f) {
						// Avoid division by zero; assign a minimal distance.
						distance = 10.00f;
					}
					float normalX = dx / distance;
					float normalY = dy / distance;

					float relativeVelX = pParticle.velocity.x - pParticleOther.velocity.x;
					float relativeVelY = pParticle.velocity.y - pParticleOther.velocity.y;

					float velocityAlongNormal = relativeVelX * normalX + relativeVelY * normalY;

					if (velocityAlongNormal > 0)
						continue;

					float e = 0.9f;

					float jImpulse = -(1 + e) * velocityAlongNormal;
					jImpulse /= (1 / pParticle.mass + 1 / pParticleOther.mass);

					float impulseX = jImpulse * normalX;
					float impulseY = jImpulse * normalY;

					pParticle.velocity.x += impulseX / pParticle.mass;
					pParticle.velocity.y += impulseY / pParticle.mass;
					pParticleOther.velocity.x -= impulseX / pParticleOther.mass;
					pParticleOther.velocity.y -= impulseY / pParticleOther.mass;

					//TODO: Work on particle overlapping and stability

					float penetration = radiiSum - distance;
					if (penetration > 0) {
						// Correction factor to reduce jitter (commonly 20%).
						float percent = 0.2f;
						// Small tolerance to allow a little overlap (prevents oscillations).
						float slop = 0.01f;
						// Only correct if penetration is above a small threshold.
						float correctionMagnitude = std::max(penetration - slop, 0.0f) /
							(1 / pParticle.mass + 1 / pParticleOther.mass) * percent;
						// Compute the correction vector.
						float correctionX = correctionMagnitude * normalX;
						float correctionY = correctionMagnitude * normalY;
						// Adjust the positions to resolve the overlap.
						pParticle.pos.x += correctionX / pParticle.mass;
						pParticle.pos.y += correctionY / pParticle.mass;
						pParticleOther.pos.x -= correctionX / pParticleOther.mass;
						pParticleOther.pos.y -= correctionY / pParticleOther.mass;
					}
				}
			}
		}


	}
	if (!isPeriodicBoundaryEnabled) {
		for (size_t i = 0; i < pParticles.size(); i++) {
			ParticlePhysics& pParticle = pParticles[i];
			if (pParticle.pos.x > screenWidth || pParticle.pos.x < 0 || pParticle.pos.y > screenHeight || pParticle.pos.y < 0) {
				{
					pParticles.erase(pParticles.begin() + i);
					rParticles.erase(rParticles.begin() + i);
				}
			}

		}
	}

	myCamera.cameraFollowObject(pParticles, rParticles);
}

static void particlesColorVisuals(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles) {

	if (solidColor) {
		for (size_t i = 0; i < pParticles.size(); i++) {
			if (!rParticles[i].uniqueColor) {
				rParticles[i].color.r = static_cast<unsigned char>(densityR);
				rParticles[i].color.g = static_cast<unsigned char>(densityG);
				rParticles[i].color.b = static_cast<unsigned char>(densityB);
				rParticles[i].color.a = densityA;
			}
		}
		blendMode = 1;
	}
	else if (densityColor) {

		float densityRadiusSq = densityRadius * densityRadius;

		std::vector<int> neighborCounts(pParticles.size(), 0);

#pragma omp parallel for schedule(dynamic)
		for (size_t i = 0; i < pParticles.size(); i++) {
			const auto& pParticle = pParticles[i];
			for (size_t j = i + 1; j < pParticles.size(); j++) {
				if (std::abs(pParticles[j].pos.x - pParticle.pos.x) > densityRadius) break;
				float dx = pParticle.pos.x - pParticles[j].pos.x;
				float dy = pParticle.pos.y - pParticles[j].pos.y;
				if (dx * dx + dy * dy < densityRadiusSq) {
					neighborCounts[i]++;
					neighborCounts[j]++;
				}
			}
		}

		for (size_t i = 0; i < pParticles.size(); i++) {
			float normalDensity = std::min(float(neighborCounts[i]) / maxNeighbors, 1.0f);
			float invertedDensity = 1.0f - normalDensity;

			if (!rParticles[i].uniqueColor) {
				rParticles[i].color.r = static_cast<unsigned char>(normalDensity * densityR);
				rParticles[i].color.g = static_cast<unsigned char>(normalDensity * densityG);
				rParticles[i].color.b = static_cast<unsigned char>(invertedDensity * densityB);
				rParticles[i].color.a = densityA;
			}
		}
		blendMode = 1;
	}
	else if (velocityColor) {
#pragma omp parallel for schedule(dynamic)
		for (size_t i = 0; i < pParticles.size(); i++) {
			float maxVel = 11000.0f;
			float minVel = 0.0f;


			float particleVelSq = pParticles[i].velocity.x * pParticles[i].velocity.x +
				pParticles[i].velocity.y * pParticles[i].velocity.y;

			float clampedVel = std::clamp(particleVelSq, minVel, maxVel);
			float normalizedVel = clampedVel / maxVel;

			float hue = (1.0f - normalizedVel) * 240.0f;
			float saturation = 1.0f;
			float value = 1.0f;

			if (!rParticles[i].uniqueColor) {
				rParticles[i].color = ColorFromHSV(hue, saturation, value);
			}
		}

		blendMode = 0;
	}
	else {
		/*for (auto& rParticle : rParticles) {
			if (!rParticle.uniqueColor) {
				rParticle.color = { 128,128,128,128 };
			}
		}
		blendMode = 1;*/
	}
}


int trailDotFrameIndex = 0;
static void mouseTrail(std::vector<MouseTrailDot>& dots, std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles) {
	const size_t NUM_PLANETS = pParticles.size();

	if (IsKeyPressed(KEY_T)) {
		trailsEnabled = !trailsEnabled;
	}
	if (!trailsEnabled) {
		dots.clear();
	}

	if (trailsEnabled) {
		for (size_t i = 0; i < pParticles.size(); i++) {
			dots.push_back(MouseTrailDot({ pParticles[i].pos.x, pParticles[i].pos.y }, 1.0f, { rParticles[i].color }));
		}
	}
	trailDotFrameIndex = 0;

	const size_t MAX_DOTS = 14 * pParticles.size();
	if (dots.size() > MAX_DOTS) {
		size_t excess = dots.size() - MAX_DOTS;
		dots.erase(dots.begin(), dots.begin() + excess);
	}
	if (IsKeyPressed(KEY_C)) {
		pParticles.clear();
		rParticles.clear();
		dots.clear();
	}
}

std::array<Button, 10> settingsButtonsArray = {

Button
	(
		{(float)screenWidth - 220, 100},
		{200, 50},
		"Pixel Drawing",
		true
	),
Button
	(
		{780, 0},
		{200, 50},
		"Particle Trails",
		true
	),
Button
	(
		{780, 0},
		{200, 50},
		"Solid Color",
		true
	),
Button
	(
		{780, 0},
		{200, 50},
		"Density Color",
		true
	),
Button
	(
		{780, 0},
		{200, 50},
		"Velocity Color",
		true
	),
Button
	(
		{780, 0},
		{200, 50},
		"Dark Matter",
		true
	),
Button
	(
		{780, 0},
		{200, 50},
		"Looping Space",
		true
	),
Button
	(
		{780, 0},
		{200, 50},
		"Barnes-Hut",
		true
	),
Button
	(
		{780, 0},
		{200, 50},
		"Multi-Threading",
		true
	),
Button
	(
		{780, 0},
		{200, 50},
		"Collisions (!!!)",
		true
	)
};

Button toggleSettingsButtons
(
	{ (float)screenWidth - 34, 85 },
	{ 14,14 },
	"",
	false
);

std::array<std::string, 14> controlsArray = {
	"1. Hold LMB: Throw heavy particle",
	"2. Hold RMB: Paint particles",
	"3. Space + Hold LMB: Create small galaxy",
	"4. Press 1: Create big galaxy",
	"5. Press 2: Scatter particles",
	"6. T: Toggle trails",
	"7. P: Toggle pixel drawing",
	"8. C: Clear all particles",
	"9. U: Toggle UI"
	"10. RMB on slider to set it to default",
	"11. Move with RMB",
	"12. Zoom with mouse wheel",
	"13. CTRL + Mouse Wheel: Brush size",
	"14. IT IS RECOMMENDED TO TURN GRAVITY OFF",
	"FOR COLLISIONS!"
};

std::array<Slider, 10> slidersArray = {
	Slider
(
	{20, static_cast<float>(screenHeight - 500)}, {250, 10}, {190, 128, 128, 255}, "Red"
),
Slider
(
	{450, 450}, {250, 10}, {128, 190, 128, 255}, "Green"
),
Slider
(
	{450, 450}, {250, 10}, {128, 128, 190, 255}, "Blue"
),
Slider
(
	{450, 450}, {250, 10}, {128, 128, 128, 255}, "Alpha"
),
Slider
(
	{450, 450}, {250, 10}, {128, 128, 128, 255}, "Density Radius"
),
Slider
(
	{450, 450}, {250, 10}, {128, 128, 128, 255}, "Max Neighbors"
),
Slider
(
	{450, 450}, {250, 10}, {128, 128, 128, 255}, "Softening"
),
Slider
(
	{450, 450}, {250, 10}, {128, 128, 128, 255}, "Theta"
),
Slider
(
	{450, 450}, {250, 10}, {128, 128, 128, 255}, "Time Scale"
),
Slider
(
	{450, 450}, {250, 10}, {128, 128, 128, 255}, "Gravity Strength"
)
};


bool showSettings = true;

static void drawScene(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles,
	std::vector<MouseTrailDot>& dots,
	bool& isMouseNotHoveringUI,
	Texture2D particleBlur, SceneCamera myCamera, ScreenCapture& screenCapture) {

	for (int i = 0; i < pParticles.size(); ++i) {

		ParticlePhysics& pParticle = pParticles[i];
		ParticleRendering& rParticle = rParticles[i];

		if (isPixelDrawingEnabled && rParticle.drawPixel) {
			DrawPixelV({ pParticle.pos.x, pParticle.pos.y }, rParticle.color);
		}
		else {
			// I multiply by 32 because that is the particle texture size in pixels
			DrawTextureEx(particleBlur, { static_cast<float>(pParticle.pos.x - rParticle.size * 32 / 2),
				static_cast<float>(pParticle.pos.y - rParticle.size * 32 / 2) }, 0, rParticle.size, rParticle.color);
		}
	}

	particlesColorVisuals(pParticles, rParticles);

	for (const MouseTrailDot& dot : dots) {
		DrawPixel(static_cast<int>(dot.pos.x), static_cast<int>(dot.pos.y), dot.color);
	}

	DrawRectangleLinesEx({ 0,0, (float)screenWidth, (float)screenHeight }, 3, GRAY);

	EndMode2D();

	bool buttonShowSettingsHovering = toggleSettingsButtons.buttonLogic(showSettings);

	if (IsKeyPressed(KEY_U)) {
		showSettings = !showSettings;
	}

	DrawTriangle(
		{ toggleSettingsButtons.pos.x + 3.0f, toggleSettingsButtons.pos.y + 5.0f },
		{ toggleSettingsButtons.pos.x + 7.0f, toggleSettingsButtons.pos.y + 11.0f },
		{ toggleSettingsButtons.pos.x + 11.0f ,toggleSettingsButtons.pos.y + 5.0f }, WHITE);

	if (showSettings) {
		for (size_t i = 1; i < settingsButtonsArray.size(); ++i) {
			settingsButtonsArray[i].pos.x = settingsButtonsArray[i - 1].pos.x;
			settingsButtonsArray[i].pos.y = settingsButtonsArray[i - 1].pos.y + settingsButtonsArray[i].size.y + 20;

		}
		bool buttonPixelDrawingHovering = settingsButtonsArray[0].buttonLogic(isPixelDrawingEnabled);
		bool buttonTrailsHovering = settingsButtonsArray[1].buttonLogic(trailsEnabled);
		bool buttonSolidColorHovering = settingsButtonsArray[2].buttonLogic(solidColor);
		bool buttonDensityColorHovering = settingsButtonsArray[3].buttonLogic(densityColor);
		bool buttonVelocityColorHovering = settingsButtonsArray[4].buttonLogic(velocityColor);
		bool buttonDarkMatterHovering = settingsButtonsArray[5].buttonLogic(isDarkMatterEnabled);
		bool buttonPeriodicBoundaryHovering = settingsButtonsArray[6].buttonLogic(isPeriodicBoundaryEnabled);
		bool buttonBarnesHutHovering = settingsButtonsArray[7].buttonLogic(isBarnesHutEnabled);
		bool buttonMultiThreadingHovering = settingsButtonsArray[8].buttonLogic(isMultiThreadingEnabled);
		bool buttonCollisionsHovering = settingsButtonsArray[9].buttonLogic(isCollisionsEnabled);
		for (size_t i = 0; i < controlsArray.size(); i++) {
			DrawText(TextFormat("%s", controlsArray[i].c_str()), 25, 100 + 20 * i, 15, WHITE);
		}

		for (size_t i = 1; i < slidersArray.size(); ++i) {
			slidersArray[i].sliderPos.x = slidersArray[i - 1].sliderPos.x;
			slidersArray[i].sliderPos.y = slidersArray[i - 1].sliderPos.y + slidersArray[i].sliderSize.y + 40;
		}

		bool sliderRedHovering = slidersArray[0].sliderLogic(0, densityR, 255);
		bool sliderGreenHovering = slidersArray[1].sliderLogic(0, densityG, 255);
		bool sliderBlueHovering = slidersArray[2].sliderLogic(0, densityB, 255);
		bool sliderAlphaHovering = slidersArray[3].sliderLogic(0, densityA, 255);
		bool sliderDensityHovering = slidersArray[4].sliderLogic(0.0f, densityRadius, 30.0f);
		bool sliderMaxNeighborsHovering = slidersArray[5].sliderLogic(0, maxNeighbors, 300);
		bool sliderSofteningHovering = slidersArray[6].sliderLogic(0.1f, softening, 30.0f);
		bool sliderThetaHovering = slidersArray[7].sliderLogic(0.1f, theta, 5.0f);
		bool sliderTimeScaleHovering = slidersArray[8].sliderLogic(0.0f, timeStepMultiplier, 5.0f);
		bool sliderGravityStrengthHovering = slidersArray[9].sliderLogic(0.0f, gravityMultiplier, 3.0f);





		if (buttonPixelDrawingHovering ||
			buttonDarkMatterHovering ||
			buttonPeriodicBoundaryHovering ||
			buttonTrailsHovering ||
			buttonBarnesHutHovering ||
			buttonMultiThreadingHovering ||
			buttonSolidColorHovering ||
			buttonDensityColorHovering ||
			buttonVelocityColorHovering ||
			buttonShowSettingsHovering ||
			sliderRedHovering ||
			sliderGreenHovering ||
			sliderBlueHovering ||
			sliderAlphaHovering ||
			sliderDensityHovering ||
			sliderMaxNeighborsHovering ||
			sliderSofteningHovering ||
			sliderThetaHovering ||
			sliderTimeScaleHovering ||
			buttonCollisionsHovering ||
			sliderGravityStrengthHovering
			) {
			isMouseNotHoveringUI = false;
			isDragging = false;
		}
		else {
			isMouseNotHoveringUI = true;
		}

		if (buttonSolidColorHovering && IsMouseButtonPressed(0)) {
			velocityColor = false;
			densityColor = false;
		}
		if (buttonDensityColorHovering && IsMouseButtonPressed(0)) {
			velocityColor = false;
			solidColor = false;
		}
		if (buttonVelocityColorHovering && IsMouseButtonPressed(0)) {
			densityColor = false;
			solidColor = false;
		}
	}
	else {

		if (buttonShowSettingsHovering) {
			isMouseNotHoveringUI = false;
		}
		else {
			isMouseNotHoveringUI = true;
		}
	}
	if (IsKeyPressed(KEY_P)) {
		isPixelDrawingEnabled = !isPixelDrawingEnabled;
	}


	DrawText(TextFormat("Particles: %i", pParticles.size()), 50, 50, 25, WHITE);

	if (GetFPS() >= 60) {
		DrawText(TextFormat("FPS: %i", GetFPS()), screenWidth - 150, 50, 18, GREEN);

	}
	else if (GetFPS() < 60 && GetFPS() > 30) {
		DrawText(TextFormat("FPS: %i", GetFPS()), screenWidth - 150, 50, 18, YELLOW);
	}
	else {
		DrawText(TextFormat("FPS: %i", GetFPS()), screenWidth - 150, 50, 18, RED);
	}

	bool isRecording = screenCapture.screenGrab();

	if (isRecording) {
		DrawRectangleLines(0, 0, screenWidth, screenHeight, RED);

	}

	/*if (pParticles.size() > 1) {
		for (size_t i = 0; i < pParticles.size() - 1; i++) {
			DrawLineV(pParticles[i].pos, pParticles[i + 1].pos, WHITE);

			DrawText(TextFormat("%i", i), pParticles[i].pos.x, pParticles[i].pos.y - 10, 10, RED);
		}
	}*/
}



static void enableMultiThreading() {
	if (isMultiThreadingEnabled) {
		omp_set_num_threads(16);
	}
	else {
		omp_set_num_threads(1);
	}
}

int main() {

	std::vector<ParticlePhysics> pParticles;
	std::vector<ParticleRendering> rParticles;

	std::vector<MouseTrailDot> trailDots;

	bool isMouseNotHoveringUI = false;

	ScreenCapture screenCapture;

	Morton morton;


	InitWindow(screenWidth, screenHeight, "n-Body");

	Texture2D particleBlurTex = LoadTexture("Textures/ParticleBlur.png");

	SetTargetFPS(targetFPS);

	SceneCamera myCamera;

	Brush brush(myCamera, 25.0f);

	SetConfigFlags(FLAG_MSAA_4X_HINT);

	while (!WindowShouldClose()) {


		BeginDrawing();

		ClearBackground(BLACK);

		BeginBlendMode(blendMode);

		BeginMode2D(myCamera.cameraLogic());

		/*rlPushMatrix();
		rlTranslatef(0, 25 * 50, 0);
		rlRotatef(90, 1, 0, 0);
		DrawGrid(100, 50);
		rlPopMatrix();*/

		mouseTrail(trailDots, pParticles, rParticles);


		updateScene(pParticles, rParticles, isMouseNotHoveringUI, myCamera, brush);

		morton.computeMortonKeys(pParticles, static_cast<float>(screenWidth), static_cast<float>(screenHeight));
		morton.sortParticlesByMortonKey(pParticles, rParticles);

		drawScene(pParticles, rParticles, trailDots, isMouseNotHoveringUI, particleBlurTex, myCamera, screenCapture);

		EndBlendMode();


		enableMultiThreading();

		EndDrawing();
	}

	UnloadTexture(particleBlurTex);

	CloseWindow();



	return 0;
}