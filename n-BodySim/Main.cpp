#include <iostream>
#include <vector>
#include "raylib.h"
#include <cmath>
#include <array>
#include <omp.h>
#include <thread>
#include <algorithm>
#include <bitset>

#include "slingshot.h"
#include "quadtree.h"
#include "planet.h"
#include "particleTrails.h"
#include "button.h"
#include "screenCapture.h"
#include "morton.h"
#include "slider.h"
#include "camera.h"
#include "rlgl.h"
#include "raymath.h"
#include "brush.h"
#include "particleSelection.h"
#include "particleSubdivision.h"
#include "densitySize.h"

struct UpdateParameters {
	std::vector<ParticlePhysics> pParticles;
	std::vector<ParticleRendering> rParticles;

	std::vector<ParticlePhysics> pParticlesSelected;
	std::vector<ParticleRendering> rParticlesSelected;

	std::vector<ParticleTrails> trailDots;

	bool isMouseNotHoveringUI = false;

	ScreenCapture screenCapture;

	Morton morton;

	ParticleTrails trails;

	ParticleSelection particleSelection;

	Texture2D particleBlurTex = LoadTexture("Textures/ParticleBlur.png");
	SceneCamera myCamera;

	Brush brush;

	UpdateParameters() : brush(myCamera, 25.0f) {}

	ParticleSubdivision subdivision;

	DensitySize densitySize;
};

int screenWidth = 1920;
int screenHeight = 1024;
float halfScreenWidth = screenWidth * 0.5f;
float halfScreenHeight = screenHeight * 0.5f;

float screenRatioX;
float screenRatioY;

int targetFPS = 144;

double G = 6.674e-11;
float gravityMultiplier = 1.0f;
float softening = 1.0f;
float theta = 0.6f;
float timeStepMultiplier = 1.0f;
const float fixedDeltaTime = 0.03f;

float timeFactor;

bool isGlobalTrailsEnabled = false;
bool isSelectedTrailsEnabled = false;
bool isLocalTrailsEnabled = false;
bool isPixelDrawingEnabled = false;
bool isPeriodicBoundaryEnabled = true;
bool isMultiThreadingEnabled = true;
bool isBarnesHutEnabled = true;
bool isDarkMatterEnabled = false;
bool isCollisionsEnabled = false;
bool isShowControlsEnabled = true;
bool isDensitySizeEnabled = false;


bool solidColor = false;
bool densityColor = true;
bool velocityColor = false;

bool isSpawningAllowed = true;

int blendMode = 1;

int densityR = 225;
int densityG = 120;
int densityB = 125;
int densityA = 200;

float particleTextureSize = 32.0f;

float densityRadius = 2.4f;
int maxNeighbors = 60;

int trailMaxLength = 14;

bool isRecording = false;

float particleSizeMultiplier = 1.0f;


bool subdivideAll = false;
bool subdivideSelected = false;

static Quadtree* gridFunction(std::vector<ParticlePhysics>& pParticles,
	std::vector<ParticleRendering>& rParticles) {
	Quadtree* grid = Quadtree::boundingBox(pParticles, rParticles);
	//grid->calculateMasses(pParticles);
	return grid;
}

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
		dx -= screenWidth * ((dx > halfScreenWidth) - (dx < -halfScreenWidth));
		dy -= screenHeight * ((dy > halfScreenHeight) - (dy < -halfScreenHeight));
	}

	float distanceSq = dx * dx + dy * dy + softening * softening;

	if ((grid.size * grid.size < theta * theta * distanceSq) || grid.subGrids.empty()) {
		if ((grid.endIndex - grid.startIndex) == 1 &&
			fabs(pParticles[grid.startIndex].pos.x - pParticle.pos.x) < 0.001f &&
			fabs(pParticles[grid.startIndex].pos.y - pParticle.pos.y) < 0.001f) {
			return totalForce;
		}

		float invDistance = 1.0f / sqrt(distanceSq);
		float forceMagnitude = static_cast<float>(G) * pParticle.mass * grid.gridMass * invDistance * invDistance * invDistance;
		totalForce.x = dx * forceMagnitude;
		totalForce.y = dy * forceMagnitude;
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

static void physicsUpdate(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles) {
	if (isPeriodicBoundaryEnabled) {
		for (size_t i = 0; i < pParticles.size(); i++) {
			ParticlePhysics& pParticle = pParticles[i];
			pParticle.pos.x += pParticle.velocity.x * timeFactor;
			pParticle.pos.y += pParticle.velocity.y * timeFactor;
			if (pParticle.pos.x < 0) pParticle.pos.x += screenWidth;
			else if (pParticle.pos.x >= screenWidth) pParticle.pos.x -= screenWidth;

			if (pParticle.pos.y < 0) pParticle.pos.y += screenHeight;
			else if (pParticle.pos.y >= screenHeight) pParticle.pos.y -= screenHeight;
		}
	}
	else {
		for (size_t i = 0; i < pParticles.size(); i++) {
			ParticlePhysics& pParticle = pParticles[i];
			pParticle.pos.x += pParticle.velocity.x * timeFactor;
			pParticle.pos.y += pParticle.velocity.y * timeFactor;
			if (pParticles[i].pos.x < 0 || pParticles[i].pos.x >= screenWidth || pParticles[i].pos.y < 0 || pParticles[i].pos.y >= screenHeight) {
				pParticles.erase(pParticles.begin() + i);
				rParticles.erase(rParticles.begin() + i);

			}
		}
	}

}

static void collisions(std::vector<ParticlePhysics>& particles,
	std::vector<ParticleRendering>& renderings,
	float softening) {
	size_t n = particles.size();
#pragma omp parallel for schedule(dynamic)
	for (size_t i = 0; i < n; i++) {
		ParticlePhysics& a = particles[i];
		for (size_t j = i + 1; j < n; j++) {
			ParticlePhysics& b = particles[j];
			float dx = a.pos.x - b.pos.x;
			float dy = a.pos.y - b.pos.y;
			float distSq = dx * dx + dy * dy + softening * softening;
			// TODO: MAYBE PRECOMPUTE radiiSum
			float radiiSum = renderings[i].size * 16.0f + renderings[j].size * 16.0f;
			float rsq = radiiSum * radiiSum;
			if (distSq > rsq) continue;

			float distance = std::sqrt(distSq);
			if (distance == 0.0f) distance = 10.0f;
			float normalX = dx / distance;
			float normalY = dy / distance;
			float relVel = (a.velocity.x - b.velocity.x) * normalX +
				(a.velocity.y - b.velocity.y) * normalY;
			if (relVel > 0) continue;
			float e = 0.9f;
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

void flattenQuadtree(Quadtree* node, std::vector<Quadtree*>& flatList) {
	if (!node) return;

	flatList.push_back(node);

	for (const auto& child : node->subGrids) {
		flattenQuadtree(child.get(), flatList);
	}
}

Vector2 mouseWorldPos;
static void updateScene(UpdateParameters& myParameters) {

	Quadtree* grid = nullptr;

	G = 6.674e-11 * gravityMultiplier;

	timeFactor = fixedDeltaTime * timeStepMultiplier;

	if (timeFactor == 0) {
		myParameters.morton.computeMortonKeys(myParameters.pParticles, grid->boundingBoxPos, grid->boundingBoxSize);
		myParameters.morton.sortParticlesByMortonKey(myParameters.pParticles, myParameters.rParticles);
	}

	if (timeFactor > 0) {
		grid = gridFunction(myParameters.pParticles, myParameters.rParticles);
	}

	/*std::vector<Quadtree*> flatNodes;

	flattenQuadtree(grid, flatNodes);*/

	//int index = 0;
	/*for (Quadtree* node : flatNodes) {
		DrawRectangleLines(node->pos.x, node->pos.y, node->size, node->size, WHITE);

		const char* textDisplay = TextFormat("%i", node->depth);

		DrawText(textDisplay, node->pos.x + node->size / 2, node->pos.y + node->size / 2, 10, {128,128,128,140});

		index++;
	}*/

	// DRAW QUADREE DEBUGGING
	/*if (grid != nullptr) {
		grid->drawQuadtree();
	}*/

	mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), myParameters.myCamera.camera);

	myParameters.brush.brushSize(mouseWorldPos);

	if (myParameters.isMouseNotHoveringUI && isSpawningAllowed) {
		Slingshot slingshot = slingshot.planetSlingshot(isDragging, myParameters.myCamera);
		if (IsMouseButtonReleased(0) && !IsKeyDown(KEY_LEFT_CONTROL) && !IsKeyDown(KEY_LEFT_ALT) && isDragging) {
			myParameters.pParticles.emplace_back(
				Vector2{ static_cast<float>(mouseWorldPos.x), static_cast<float>(mouseWorldPos.y) },
				Vector2{ slingshot.normalizedX * slingshot.length, slingshot.normalizedY * slingshot.length },
				300000000000000.0f
			);
			myParameters.rParticles.emplace_back(
				Color{ 255, 255, 255, 255 },
				0.3f,
				true,
				true,
				false,
				true,
				false
			);
			isDragging = false;
		}
		if (IsMouseButtonDown(2)) {
			myParameters.brush.brushLogic(myParameters.pParticles, myParameters.rParticles, mouseWorldPos);
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

				myParameters.pParticles.emplace_back(
					Vector2{ posX, posY },
					Vector2{ velocityX, velocityY },
					50000000000.0f
				);
				myParameters.rParticles.emplace_back(
					Color{ 128, 128, 128, 100 },
					0.125f,
					false,
					true,
					false,
					false,
					true
				);

			}
		}

		if (IsKeyReleased(KEY_THREE) && isDragging) {
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

				myParameters.pParticles.emplace_back(
					Vector2{ posX, posY },
					Vector2{
						velocityX + (slingshot.normalizedX * slingshot.length * 0.3f),
						velocityY + (slingshot.normalizedY * slingshot.length * 0.3f)
					},
					85000000000.0f
				);
				myParameters.rParticles.emplace_back(
					Color{ 128, 128, 128, 100 },
					0.125f,
					false,
					true,
					false,
					false,
					true
				);
				isDragging = false;
			}
		}

		if (IsKeyPressed(KEY_TWO)) {
			for (int i = 0; i < 10000; i++) {
				myParameters.pParticles.emplace_back(
					Vector2{ static_cast<float>(rand() % screenWidth), static_cast<float>(rand() % screenHeight) },
					Vector2{ 0, 0 },
					500000000000.0f
				);
				myParameters.rParticles.emplace_back(
					Color{ 128, 128, 128, 100 },
					0.125f,
					false,
					true,
					false,
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


	if (timeFactor > 0.0f) {
		if (isBarnesHutEnabled) {
#pragma omp parallel for schedule(dynamic)
			for (size_t i = 0; i < myParameters.pParticles.size(); i++) {
				ParticlePhysics& pParticle = myParameters.pParticles[i];

				float accX = 0;
				float accY = 0;
				float prevAccX = accX;
				float prevAccY = accY;

				Vector2 netForce = calculateForceFromGrid(pParticle, *grid, myParameters.pParticles);

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
			pairWiseGravity(myParameters.pParticles);
		}

		if (isCollisionsEnabled) {
			collisions(myParameters.pParticles, myParameters.rParticles, softening);
		}

		physicsUpdate(myParameters.pParticles, myParameters.rParticles);

	}
	myParameters.trails.trailLogic(myParameters.pParticles, myParameters.rParticles, myParameters.pParticlesSelected, myParameters.rParticlesSelected,
		isGlobalTrailsEnabled, isSelectedTrailsEnabled, trailMaxLength, timeFactor, isLocalTrailsEnabled);

	myParameters.myCamera.cameraFollowObject(myParameters.pParticles, myParameters.rParticles, myParameters.isMouseNotHoveringUI, isSelectedTrailsEnabled,
		myParameters.trails);

	myParameters.particleSelection.clusterSelection(myParameters.pParticles, myParameters.rParticles,
		myParameters.myCamera, myParameters.isMouseNotHoveringUI, myParameters.trails, isGlobalTrailsEnabled);

	myParameters.particleSelection.particleSelection(myParameters.pParticles, myParameters.rParticles, myParameters.myCamera,
		myParameters.isMouseNotHoveringUI, myParameters.trails, isGlobalTrailsEnabled);

	myParameters.particleSelection.manyClustersSelection(myParameters.pParticles, myParameters.rParticles, myParameters.trails, isGlobalTrailsEnabled);

	myParameters.particleSelection.selectedParticlesStoring(myParameters.pParticles, myParameters.rParticles, myParameters.rParticlesSelected,
		myParameters.pParticlesSelected);

	myParameters.densitySize.sizeByDensity(myParameters.pParticles, myParameters.rParticles, isDensitySizeEnabled);


	if (grid != nullptr) {
		delete grid;
	}
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
}

std::array<Button, 16> settingsButtonsArray = {

Button({(float)screenWidth - 195, 100}, {175, 35}, "Pixel Drawing", true),

Button({780, 0}, {200, 50}, "Global Trails", true),

Button({780, 0}, {200, 50}, "Selected Trails", true),

Button({780, 0}, {200, 50}, "Local Trails", true),

Button({780, 0}, {200, 50}, "Solid Color", true),

Button({780, 0}, {200, 50}, "Density Color", true),

Button({780, 0}, {200, 50}, "Velocity Color", true),

Button({780, 0}, {200, 50}, "Dark Matter", true),

Button({780, 0}, {200, 50}, "Looping Space", true),

Button({780, 0}, {200, 50}, "Barnes-Hut", true),

Button({780, 0}, {200, 50}, "Multi-Threading", true),

Button({780, 0}, {200, 50}, "Collisions (!!!)", true),

Button({780, 0}, {200, 50}, "Controls", true),

Button({780, 0}, {200, 50}, "Subdivide All", true),

Button({780, 0}, {200, 50}, "Subdivide Selec.", true),

Button({780, 0}, {200, 50}, "Density Size", true)

};
std::array<Button, 1> toggleSettingsButtons = {
Button
(
	{ (float)screenWidth - 34, 85 },
	{ 14,14 },
	"",
	false
)
};

std::array<std::string, 19> controlsArray = {
	"1. Hold LMB and Drag: Throw heavy particle",
	"2. Hold MMB: Paint particles",
	"3. Hold Key 3 and Drag: Create small galaxy",
	"4. Press 1: Create big galaxy",
	"5. Press 2: Create scattered particles",
	"6. T: Toggle global trails",
	"7. LCTRL + T: Toggle local trails",
	"8. P: Toggle pixel drawing",
	"9. C: Clear all particles",
	"10. U: Toggle UI",
	"11. RMB on slider to set it to default",
	"12. Move with RMB",
	"13. Zoom with mouse wheel",
	"14. LCTRL + Scroll wheel: Brush size",
	"15. RMB on particle cluster to follow it",
	"16.  LCTRL + RMB on particle to follow it",
	"17. Z: Center camera on selected particles",
	"18. F: Reset camera ",
	"19. COLLISIONS ARE STILL EXPERIMENTAL!"
};

std::array<Slider, 12> slidersArray = {
	Slider
(
	{20, static_cast<float>(screenHeight - 500)}, {230, 7}, {190, 128, 128, 255}, "Red"
),
Slider({450, 450}, {250, 10}, {128, 190, 128, 255}, "Green"),

Slider({450, 450}, {250, 10}, {128, 128, 190, 255}, "Blue"),

Slider({450, 450}, {250, 10}, {128, 128, 128, 255}, "Alpha"),

Slider({450, 450}, {250, 10}, {128, 128, 128, 255}, "Density Radius"),

Slider({450, 450}, {250, 10}, {128, 128, 128, 255}, "Max Neighbors"),

Slider({450, 450}, {250, 10}, {128, 128, 128, 255}, "Softening"),

Slider({450, 450}, {250, 10}, {128, 128, 128, 255}, "Theta"),

Slider({450, 450}, {250, 10}, {128, 128, 128, 255}, "Time Scale"),

Slider({450, 450}, {250, 10}, {128, 128, 128, 255}, "Gravity Strength"),

Slider({450, 450}, {250, 10}, {128, 128, 128, 255}, "Trails Length"),

Slider({450, 450}, {250, 10}, {128, 128, 128, 255}, "Particles Size")
};


bool showSettings = true;
static void drawScene(UpdateParameters& myParameters) {

	for (int i = 0; i < myParameters.pParticles.size(); ++i) {

		ParticlePhysics& pParticle = myParameters.pParticles[i];
		ParticleRendering& rParticle = myParameters.rParticles[i];

		if (isPixelDrawingEnabled && rParticle.drawPixel) {
			DrawPixelV({ pParticle.pos.x, pParticle.pos.y }, rParticle.color);
		}
		else {
			// Texture size is set to 32 because that is the particle texture size in pixels
			DrawTextureEx(myParameters.particleBlurTex, { static_cast<float>(pParticle.pos.x - rParticle.size * particleTextureSize / 2),
				static_cast<float>(pParticle.pos.y - rParticle.size * particleTextureSize / 2) }, 0, rParticle.size, rParticle.color);
		}

		if (!isDensitySizeEnabled) {

			rParticle.size = rParticle.previousSize * particleSizeMultiplier;

		}
	}

	particlesColorVisuals(myParameters.pParticles, myParameters.rParticles);

	myParameters.trails.drawTrail(myParameters.rParticles, myParameters.particleBlurTex);


	//EVERYTHING INTENDED TO APPEAR WHILE RECORDING ABOVE
	isRecording = myParameters.screenCapture.screenGrab();
	if (isRecording) {
		DrawRectangleLinesEx({ 0,0, (float)screenWidth, (float)screenHeight }, 3, RED);
	}
	//EVERYTHING NOT INTENDED TO APPEAR WHILE RECORDING BELOW

	mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), myParameters.myCamera.camera);
	myParameters.brush.drawBrush(mouseWorldPos);

	DrawRectangleLinesEx({ 0,0, (float)screenWidth, (float)screenHeight }, 3, GRAY);

	// MORTON DEBUGGING

	/*if (myParameters.pParticles.size() > 1) {
		for (size_t i = 0; i < myParameters.pParticles.size() - 1; i++) {
			DrawLineV(myParameters.pParticles[i].pos, myParameters.pParticles[i + 1].pos, WHITE);

			DrawText(TextFormat("%i", i), myParameters.pParticles[i].pos.x, myParameters.pParticles[i].pos.y - 10, 10, { 128,128,128,128 });
		}
	}*/

	//for (size_t i = 0; i < myParameters.pParticles.size(); i++) {
	//	if (myParameters.rParticles[i].isSelected) {
	//		// Bitset 22 because morton is computing 11 bits per axis
	//		std::bitset<22> mortonBinary(myParameters.pParticles[i].mortonKey);

	//		std::cout
	//			<< "Pos X: " << myParameters.pParticles[i].pos.x << std::endl
	//			<< "Pos Y:" << myParameters.pParticles[i].pos.y << std::endl
	//			<< "Morton Key:" << myParameters.pParticles[i].mortonKey << std::endl
	//			<< "Morton Binary: " << mortonBinary << std::endl
	//			<< "----------------------" << std::endl;

	//	}
	//}

	// MORTON DEBUGGING


	// EVERYTHING NON-STATIC RELATIVE TO CAMERA ABOVE
	EndMode2D();
	// EVERYTHING STATIC RELATIVE TO CAMERA BELOW

	bool buttonShowSettingsHovering = toggleSettingsButtons[0].buttonLogic(showSettings);

	if (IsKeyPressed(KEY_U)) {
		showSettings = !showSettings;
	}

	DrawTriangle(
		{ toggleSettingsButtons[0].pos.x + 3.0f, toggleSettingsButtons[0].pos.y + 5.0f },
		{ toggleSettingsButtons[0].pos.x + 7.0f, toggleSettingsButtons[0].pos.y + 11.0f },
		{ toggleSettingsButtons[0].pos.x + 11.0f ,toggleSettingsButtons[0].pos.y + 5.0f }, WHITE);

	if (showSettings) {
		for (size_t i = 1; i < settingsButtonsArray.size(); ++i) {
			settingsButtonsArray[i].pos.x = settingsButtonsArray[i - 1].pos.x;
			settingsButtonsArray[i].pos.y = settingsButtonsArray[i - 1].pos.y + settingsButtonsArray[i].size.y + 20;
			settingsButtonsArray[i].size = settingsButtonsArray[i - 1].size;

		}
		bool buttonPixelDrawingHovering = settingsButtonsArray[0].buttonLogic(isPixelDrawingEnabled);
		bool buttonGlobalTrailsHovering = settingsButtonsArray[1].buttonLogic(isGlobalTrailsEnabled);
		bool buttonSelectedTrailsHovering = settingsButtonsArray[2].buttonLogic(isSelectedTrailsEnabled);
		bool buttonLocalTrailsHovering = settingsButtonsArray[3].buttonLogic(isLocalTrailsEnabled);
		bool buttonSolidColorHovering = settingsButtonsArray[4].buttonLogic(solidColor);
		bool buttonDensityColorHovering = settingsButtonsArray[5].buttonLogic(densityColor);
		bool buttonVelocityColorHovering = settingsButtonsArray[6].buttonLogic(velocityColor);
		bool buttonDarkMatterHovering = settingsButtonsArray[7].buttonLogic(isDarkMatterEnabled);
		bool buttonPeriodicBoundaryHovering = settingsButtonsArray[8].buttonLogic(isPeriodicBoundaryEnabled);
		bool buttonBarnesHutHovering = settingsButtonsArray[9].buttonLogic(isBarnesHutEnabled);
		bool buttonMultiThreadingHovering = settingsButtonsArray[10].buttonLogic(isMultiThreadingEnabled);
		bool buttonCollisionsHovering = settingsButtonsArray[11].buttonLogic(isCollisionsEnabled);
		bool buttonControlsHovering = settingsButtonsArray[12].buttonLogic(isShowControlsEnabled);
		bool buttonSubdivideAllHovering = settingsButtonsArray[13].buttonLogic(subdivideAll);
		bool buttonSubdivideSelectedHovering = settingsButtonsArray[14].buttonLogic(subdivideSelected);
		bool buttonDensitySizeHovering = settingsButtonsArray[15].buttonLogic(isDensitySizeEnabled);

		if (isShowControlsEnabled) {
			for (size_t i = 0; i < controlsArray.size(); i++) {
				DrawText(TextFormat("%s", controlsArray[i].c_str()), 25, 100 + 20 * static_cast<int>(i), 15, WHITE);
			}
		}

		for (size_t i = 1; i < slidersArray.size(); ++i) {
			slidersArray[i].sliderPos.x = slidersArray[i - 1].sliderPos.x;
			slidersArray[i].sliderPos.y = slidersArray[i - 1].sliderPos.y + slidersArray[i].sliderSize.y + 35;
			slidersArray[i].sliderSize = slidersArray[i - 1].sliderSize;
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
		bool sliderTrailsLengthHovering = slidersArray[10].sliderLogic(0, trailMaxLength, 400);
		bool sliderParticlesSizeHovering = slidersArray[11].sliderLogic(0.1f, particleSizeMultiplier, 5.0f);


		if (buttonPixelDrawingHovering ||
			buttonDarkMatterHovering ||
			buttonPeriodicBoundaryHovering ||
			buttonGlobalTrailsHovering ||
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
			sliderGravityStrengthHovering ||
			buttonControlsHovering ||
			buttonSelectedTrailsHovering ||
			sliderTrailsLengthHovering ||
			buttonLocalTrailsHovering ||
			buttonSubdivideAllHovering ||
			buttonSubdivideSelectedHovering ||
			buttonDensitySizeHovering ||
			sliderParticlesSizeHovering
			) {
			myParameters.isMouseNotHoveringUI = false;
			isDragging = false;
		}
		else {
			myParameters.isMouseNotHoveringUI = true;
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

		if (buttonGlobalTrailsHovering && IsMouseButtonPressed(0)) {
			isSelectedTrailsEnabled = false;
			myParameters.trails.trailDots.clear();
		}
		if (buttonSelectedTrailsHovering && IsMouseButtonPressed(0)) {
			isGlobalTrailsEnabled = false;
			myParameters.trails.trailDots.clear();
		}
	}
	else {

		if (buttonShowSettingsHovering) {
			myParameters.isMouseNotHoveringUI = false;
		}
		else {
			myParameters.isMouseNotHoveringUI = true;
		}
	}
	if (IsKeyPressed(KEY_P)) {
		isPixelDrawingEnabled = !isPixelDrawingEnabled;
	}

	myParameters.subdivision.subdivideParticles(myParameters.pParticles, myParameters.rParticles, particleTextureSize, subdivideAll, subdivideSelected,
		myParameters.isMouseNotHoveringUI, isDragging);


	DrawText(TextFormat("Particles: %i", myParameters.pParticles.size()), 50, 50, 25, WHITE);
	if (myParameters.pParticlesSelected.size() > 0) {
		DrawText(TextFormat("Selected Particles: %i", myParameters.pParticlesSelected.size()), 400, 50, 25, WHITE);
	}

	if (GetFPS() >= 60) {
		DrawText(TextFormat("FPS: %i", GetFPS()), screenWidth - 150, 50, 18, GREEN);

	}
	else if (GetFPS() < 60 && GetFPS() > 30) {
		DrawText(TextFormat("FPS: %i", GetFPS()), screenWidth - 150, 50, 18, YELLOW);
	}
	else {
		DrawText(TextFormat("FPS: %i", GetFPS()), screenWidth - 150, 50, 18, RED);
	}
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

	InitWindow(screenWidth, screenHeight, "n-Body");


	SetTargetFPS(targetFPS);


	UpdateParameters updateParameters;

	SetConfigFlags(FLAG_MSAA_4X_HINT);

	while (!WindowShouldClose()) {


		BeginDrawing();

		ClearBackground(BLACK);

		BeginBlendMode(blendMode);

		BeginMode2D(updateParameters.myCamera.cameraLogic());

		/*rlPushMatrix();
		rlTranslatef(0, 25 * 50, 0);
		rlRotatef(90, 1, 0, 0);
		DrawGrid(100, 50);
		rlPopMatrix();*/

		updateScene(updateParameters);

		drawScene(updateParameters);

		EndBlendMode();


		enableMultiThreading();

		EndDrawing();
	}

	UnloadTexture(updateParameters.particleBlurTex);

	CloseWindow();



	return 0;
}