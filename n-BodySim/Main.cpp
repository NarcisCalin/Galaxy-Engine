

#include <iostream>
#include <vector>
#include "raylib.h"
#include <cmath>
#include <algorithm>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <stdexcept>
#include <array>

#include "slingshot.h"
#include "quadtree.h"
#include "planet.h"
#include "mouseTrailDot.h"
#include "button.h"
#include <omp.h>



int screenWidth = 1000;
int screenHeight = 1000;

int targetFPS = 144;
const double G = 6.674 * pow(10, -11);

bool trailsEnabled = false;
bool enableBlur = false;
bool enablePixelDrawing = true;
bool isFourierSpaceEnabled = true;
bool isMultiThreadingEnabled = true;
bool barnesHutEnabled = true;
bool isDarkMatterEnabled = false;
bool colorVisualsEnabled = false;


static Quadtree gridFunction(std::vector<Planet> vectorPlanet) {
	Quadtree grid(
		0,             // posX
		0,             // posY
		screenWidth,   // size
		12,            // Red
		12,            // Green
		12,            // Blue
		12,            // Alpha
		vectorPlanet,  // planets
		nullptr        // parent (null for root)
	);

	// Calculate masses for the entire tree after construction
	grid.calculateMasses();
	//grid.printGridInfo();
	//grid.drawCenterOfMass();

	return grid;
}

bool isMouse0Pressed;
bool isMouse2SpacePressed;
bool isDragging = false;

Slingshot slingshotObject = Slingshot::planetSlingshot(isDragging, isMouse0Pressed, isMouse2SpacePressed);

int planetIndex = 0;


static Vector2 calculateForceFromGrid(Planet& planet, const Quadtree& grid) {

	Vector2 totalForce = { 0.0f, 0.0f };

	if (grid.gridMass <= 0) return totalForce;

	float dx = grid.centerOfMass.x - planet.pos.x;

	if (isFourierSpaceEnabled) {
		if (dx > screenWidth / 2)
			dx -= screenWidth;
		else if (dx < -screenWidth / 2)
			dx += screenWidth;
	}

	float dy = grid.centerOfMass.y - planet.pos.y;

	if (isFourierSpaceEnabled) {
		if (dy > screenHeight / 2)
			dy -= screenHeight;
		else if (dy < -screenHeight / 2)
			dy += screenHeight;
	}

	float distanceSq = dx * dx + dy * dy;

	// Avoid division by zero
	if (distanceSq < 5.0f) distanceSq = 60.0f;
	float distance = sqrt(distanceSq);

	float s_over_d = grid.size / distance;
	const float theta = 0.5f;

	if (s_over_d < theta || grid.subGrids.empty()) {

		if (grid.myPlanets.size() == 1 &&
			fabs(grid.myPlanets[0].pos.x - planet.pos.x) < 0.001f &&
			fabs(grid.myPlanets[0].pos.y - planet.pos.y) < 0.001f) {
			return totalForce;
		}
		double force = G * planet.mass * grid.gridMass / distanceSq;
		totalForce.x = (dx / distance) * force;
		totalForce.y = (dy / distance) * force;

	}
	else {
		for (const Quadtree& subGrid : grid.subGrids) {
			Vector2 childForce = calculateForceFromGrid(planet, subGrid);
			totalForce.x += childForce.x;
			totalForce.y += childForce.y;
		}
	}
	return totalForce;
}

//THIS FUNCTION BELOW IS USED TO CALCULATE DARK MATTER

struct DarkMatterHalo {
	Vector2 pos;      // Position of the halo center
	double mass;      // Mass of the halo (e.g. 10^19)
	double radius;    // Scale radius parameter of the halo

	// Constructor with default values
	DarkMatterHalo(Vector2 position = { 0.0f, 0.0f },
		double m = 1e19,
		double r = 200.0)
		: pos(position), mass(m), radius(r) {
	}
};

static Vector2 darkMatterForce(const Planet& planet) {
	float centerX = screenWidth / 2.0f;
	float centerY = screenHeight / 2.0f;

	float dx = planet.pos.x - centerX;
	float dy = planet.pos.y - centerY;
	float radius = sqrt(dx * dx + dy * dy);
	if (radius < 1.0f) radius = 1.0f;

	// NFW parameters
	const double haloMass = 7e17;      // Total halo mass
	const double haloRadius = 650.0;    // Scale radius
	const double G = 6.674e-11;         // Gravitational constant

	// NFW enclosed mass formula
	double concentration = 10;        // Typical for galaxies
	double r_ratio = radius / haloRadius;
	double M_enclosed = haloMass * (log(1 + r_ratio) - r_ratio / (1 + r_ratio))
		/ (log(1 + concentration) - concentration / (1 + concentration));

	// Newtonian acceleration
	double acceleration = (G * M_enclosed) / (radius * radius);

	Vector2 force;
	force.x = -(dx / radius) * acceleration * planet.mass; // Force = mass * acceleration
	force.y = -(dy / radius) * acceleration * planet.mass;

	return force;
}


void pairWiseGravity(std::vector<Planet>& planets) {
	const float fixedDeltaTime = 0.03f;

	float timeStepMultiplier = 1;
	float deltaTime = GetFrameTime() * timeStepMultiplier;
#pragma omp parallel for schedule(dynamic)
	for (size_t i = 0; i < planets.size(); ++i) {
		for (size_t j = i + 1; j < planets.size(); ++j) {
			Planet& planetA = planets[i];
			Planet& planetB = planets[j];


			// Calculate gravitational force between A and B
			float dx = planetB.pos.x - planetA.pos.x;
			if (dx > screenWidth / 2)
				dx -= screenWidth;
			else if (dx < -screenWidth / 2)
				dx += screenWidth;

			float dy = planetB.pos.y - planetA.pos.y;
			if (dy > screenHeight / 2)
				dy -= screenHeight;
			else if (dy < -screenHeight / 2)
				dy += screenHeight;
			float distanceSq = dx * dx + dy * dy;

			// Avoid division by zero
			if (distanceSq < 200) continue;

			float distance = sqrt(distanceSq);
			float force = G * planetA.mass * planetB.mass / distanceSq; // Total force magnitude


			//planetB.proximity = force;


			// Normalize direction
			float fx = (dx / distance) * force;
			float fy = (dy / distance) * force;

			float accelPlanetAX = fx / planetA.mass;
			float accelPlanetAY = fy / planetA.mass;


			float accelPlanetBX = fx / planetB.mass;
			float accelPlanetBY = fy / planetB.mass;

			planetA.velocity.x += accelPlanetAX * deltaTime;
			planetA.velocity.y += accelPlanetAY * deltaTime;

			planetB.velocity.x -= accelPlanetBX * deltaTime;
			planetB.velocity.y -= accelPlanetBY * deltaTime;

		}
	}
}

bool isGPU = true;

static void updateScene(std::vector<Planet>& planets, bool& isMouseHoveringUI) {

	const float fixedDeltaTime = 0.03f;

	float timeStepMultiplier = 1;
	float deltaTime = GetFrameTime() * timeStepMultiplier;

	if (IsKeyPressed(KEY_F) && isFourierSpaceEnabled) {
		isFourierSpaceEnabled = false;
	}
	else if (IsKeyPressed(KEY_F) && !isFourierSpaceEnabled) {
		isFourierSpaceEnabled = true;
	}

	Quadtree grid = gridFunction(planets);


	if (isMouseHoveringUI) {
		Slingshot slingshot = slingshotObject.planetSlingshot(isDragging, isMouse0Pressed, isMouse2SpacePressed);

		if (IsMouseButtonReleased(0) && !IsKeyDown(KEY_SPACE)) {
			planets.emplace_back(GetMouseX(),
				GetMouseY(),
				4,
				slingshot.normalizedX * slingshot.length,
				slingshot.normalizedY * slingshot.length,
				500000000000000,
				255,
				255,
				255,
				255,
				0,
				0,
				true,
				false);
			isDragging = false;
		}

		if (IsMouseButtonDown(2)) {
			for (int i = 0; i < 70; i++) {
				planets.emplace_back(
					GetMouseX() + rand() % 50 - 25,
					GetMouseY() + rand() % 50 - 25,
					1,
					0,
					0,
					200000000000,
					128,
					128,
					128,
					100,
					0,
					0,
					false,
					true
				);

			}
		}

		if (IsMouseButtonPressed(1) && !isDragging) {
			for (int i = 0; i < 40000; i++) {
				float galaxyCenterX = 500;
				float galaxyCenterY = 500;

				float angle = static_cast<float>(rand()) / RAND_MAX * 2 * PI;
				float radius = static_cast<float>(rand()) / RAND_MAX * 200.0f + 8;

				float posX = galaxyCenterX + radius * cos(angle);
				float posY = galaxyCenterY + radius * sin(angle);

				float dx = posX - galaxyCenterX;
				float dy = posY - galaxyCenterY;

				float normalRadius = 10 / radius;

				float angularSpeed = 130 / (radius + 60);

				float velocityX = -dy * angularSpeed;
				float velocityY = dx * angularSpeed;

				planets.emplace_back(
					posX, posY,
					2, // Size
					velocityX, velocityY,
					50000000000, // Mass
					128, 128, 128, 100, // Color (RGBA)
					0, 0, // Acceleration
					false,
					true
				);
			}
		}

		if (IsMouseButtonReleased(0) && isDragging || IsKeyReleased(KEY_SPACE) && isDragging) {
			for (int i = 0; i < 12000; i++) {

				float galaxyCenterX = GetMouseX();
				float galaxyCenterY = GetMouseY();

				float angle = static_cast<float>(rand()) / RAND_MAX * 2 * PI;
				float radius = static_cast<float>(rand()) / RAND_MAX * 100.0f + 2;

				float posX = galaxyCenterX + radius * cos(angle);
				float posY = galaxyCenterY + radius * sin(angle);

				float dx = posX - galaxyCenterX;
				float dy = posY - galaxyCenterY;

				float normalRadius = 10 / radius;

				float angularSpeed = 60 / (radius + 60);

				float velocityX = -dy * angularSpeed;
				float velocityY = dx * angularSpeed;

				planets.emplace_back(
					posX,
					posY,
					1,
					velocityX + (slingshot.normalizedX * slingshot.length),
					velocityY + (slingshot.normalizedY * slingshot.length),
					85000000000,
					128,
					128,
					128,
					100,
					0,
					0,
					false,
					true
				);
				isDragging = false;
			}
		}

		if (IsKeyPressed(KEY_G)) {
			for (int i = 0; i < 10000; i++) {
				planets.emplace_back(rand() % screenWidth,
					rand() % screenHeight,
					1,
					0,
					0,
					500000000000,
					128,
					128,
					128,
					100,
					0,
					0,
					false,
					true
				);
			}
		}
	}
//	if (isGPU) {
//		Planet* planets_data = planets.data();
//		int numPlanets = planets.size();
//
//#pragma omp target teams distribute parallel for \
//    map(to: grid, isDarkMatterEnabled, fixedDeltaTime) \
//    map(tofrom: planets_data[0:numPlanets])
//		for (int i = 0; i < numPlanets; i++) {
//			Planet& planet = planets_data[i];
//
//			// Reset acceleration
//			planet.acceleration.x = 0.0f;
//			planet.acceleration.y = 0.0f;
//
//			// Calculate the net force from the grid
//			Vector2 netForce = calculateForceFromGrid(planet, grid);
//
//			// Add dark matter force if enabled
//			if (isDarkMatterEnabled) {
//				Vector2 dmForce = darkMatterForce(planet);
//				netForce.x += dmForce.x;
//				netForce.y += dmForce.y;
//			}
//
//			// Update acceleration
//			planet.acceleration.x = netForce.x / planet.mass;
//			planet.acceleration.y = netForce.y / planet.mass;
//
//			// Update velocity (predictor-corrector update)
//			planet.velocity.x += fixedDeltaTime * (1.5f * planet.acceleration.x) - (0.5f * planet.prevAcceleration.x);
//			planet.velocity.y += fixedDeltaTime * (1.5f * planet.acceleration.y) - (0.5f * planet.prevAcceleration.y);
//		}
//	}
//	else {

	if (barnesHutEnabled) {

#pragma omp parallel for schedule(dynamic)
		for (Planet& planet : planets) {
			planet.acceleration = { 0.0f, 0.0f };

			Vector2 netForce = calculateForceFromGrid(planet, grid);

			if (isDarkMatterEnabled) {
				Vector2 dmForce = darkMatterForce(planet);
				netForce.x += dmForce.x;
				netForce.y += dmForce.y;
			}

			planet.acceleration.x = netForce.x / planet.mass;
			planet.acceleration.y = netForce.y / planet.mass;

			planet.velocity.x += fixedDeltaTime * ((3.0f / 2.0f)) * planet.acceleration.x - ((1.0f / 2.0f)) * planet.prevAcceleration.x;
			planet.velocity.y += fixedDeltaTime * ((3.0f / 2.0f)) * planet.acceleration.y - ((1.0f / 2.0f)) * planet.prevAcceleration.y;
		}

	}
	else {
		pairWiseGravity(planets);
	}
	//}



	for (Planet& planet : planets) {
		planet.pos.x += planet.velocity.x * fixedDeltaTime;
		planet.pos.y += planet.velocity.y * fixedDeltaTime;

		//  THIS IS THE INFINITE SPACE CODE
		if (isFourierSpaceEnabled) {
			if (planet.pos.x < 0)
				planet.pos.x += screenWidth;
			else if (planet.pos.x >= screenWidth)
				planet.pos.x -= screenWidth;
			if (planet.pos.y < 0)
				planet.pos.y += screenHeight;
			else if (planet.pos.y >= screenHeight)
				planet.pos.y -= screenHeight;

		}
	}
}

static void particlesColorVisuals(std::vector<Planet>& planets) {

	const float densityRadius = 7.0f;
	const float densityRadiusSq = densityRadius * densityRadius;  // Precompute squared radius
	const int maxNeighbors = 30;

	std::vector<int> neighborCounts(planets.size(), 0);

#pragma omp parallel for schedule(dynamic)
	for (size_t i = 0; i < planets.size(); i++) {
		for (size_t j = i + 1; j < planets.size(); j++) {
			float dx = planets[i].pos.x - planets[j].pos.x;
			float dy = planets[i].pos.y - planets[j].pos.y;
			if (dx * dx + dy * dy < densityRadiusSq) {
				neighborCounts[i]++;
				neighborCounts[j]++;
			}
		}
	}

	for (size_t i = 0; i < planets.size(); i++) {
		float normalDensity = std::min(float(neighborCounts[i]) / maxNeighbors, 1.0f);
		if (!planets[i].customColor) {
			planets[i].color.r = static_cast<unsigned char>(normalDensity * 255);
			planets[i].color.g = static_cast<unsigned char>(normalDensity * 140);
			planets[i].color.b = static_cast<unsigned char>(60);
			planets[i].color.a = static_cast<unsigned char>(100);
		}
	}
}


int trailDotFrameIndex = 0;
static void mouseTrail(std::vector<MouseTrailDot>& dots, std::vector<Planet>& planets) {
	// Add new dots for all planets
	const int NUM_PLANETS = planets.size();

	if (IsKeyPressed(KEY_T)) {
		trailsEnabled = !trailsEnabled;
	}
	if (!trailsEnabled) {
		dots.clear();
	}

	if (trailsEnabled) {
		for (const Planet& planet : planets) {
			dots.push_back(MouseTrailDot(planet.pos.x, planet.pos.y, 1));
		}
	}
	trailDotFrameIndex = 0;

	// Calculate how many dots to remove to stay at MAX_DOTS
	const int MAX_DOTS = 14 * planets.size();
	if (dots.size() > MAX_DOTS) {
		int excess = dots.size() - MAX_DOTS;
		dots.erase(dots.begin(), dots.begin() + excess); // Remove oldest `excess` dots
	}
	if (IsKeyPressed(KEY_C)) {
		planets.clear();
		dots.clear();
	}
}

std::array<Button, 8> settingsButtonsArray = {

Button
	(
		{780, 100},
		{200, 50},
		"Pixel Drawing",
		true
	),
Button
	(
		{780, 0},
		{200, 50},
		"Blur Drawing",
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
		{780, 100},
		{200, 50},
		"Color Visuals",
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
		"Fourier Space",
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
	) };

Button toggleSettingsButtons
(
	{ 966, 85 },
	{ 14,14 },
	"",
	false
);

std::array<std::string, 9> controlsArray = {
	"Hold LMB: Throw heavy particle",
	"RMB: Create big galaxy",
	"Hold MMB: Paint particles",
	"Space + Hold LMB: Create small galaxy",
	"G: Scatter particles",
	"T: Toggle trails",
	"P: Toggle pixel drawing",
	"B: Toggle blur drawing",
	"C: Clear all particles"
};

bool showSettings = true;

static void drawScene(std::vector<Planet>& planets, std::vector<MouseTrailDot>& dots, bool& isMouseHoveringUI) {

	bool buttonShowSettingsHovering = toggleSettingsButtons.buttonLogic(showSettings);
	DrawTriangle({ 969, 90 }, { 973, 96 }, { 977,90 }, WHITE);

	if (showSettings) {
		for (int i = 1; i < settingsButtonsArray.size(); i++) {
			settingsButtonsArray[i].pos.x = settingsButtonsArray[i - 1].pos.x;
			settingsButtonsArray[i].pos.y = settingsButtonsArray[i - 1].pos.y + settingsButtonsArray[i].size.y + 20;

		}
		bool buttonPixelDrawingHovering = settingsButtonsArray[0].buttonLogic(enablePixelDrawing);
		bool buttonBlurHovering = settingsButtonsArray[1].buttonLogic(enableBlur);
		bool buttonTrailsHovering = settingsButtonsArray[2].buttonLogic(trailsEnabled);
		bool buttoncolorVisualsHovering = settingsButtonsArray[3].buttonLogic(colorVisualsEnabled);
		bool buttonDarkMatterHovering = settingsButtonsArray[4].buttonLogic(isDarkMatterEnabled);
		bool buttonFourierSpaceHovering = settingsButtonsArray[5].buttonLogic(isFourierSpaceEnabled);
		bool buttonBarnesHutHovering = settingsButtonsArray[6].buttonLogic(barnesHutEnabled);
		bool buttonMultiThreadingHovering = settingsButtonsArray[7].buttonLogic(isMultiThreadingEnabled);

		for (int i = 0; i < controlsArray.size(); i++) {
			DrawText(TextFormat("%s", controlsArray[i].c_str()), 25, 100 + 20 * i, 15, WHITE);
		}

		if (buttonPixelDrawingHovering ||
			buttonDarkMatterHovering ||
			buttonFourierSpaceHovering ||
			buttonTrailsHovering ||
			buttonBarnesHutHovering ||
			buttonMultiThreadingHovering ||
			buttonBlurHovering ||
			buttoncolorVisualsHovering ||
			buttonShowSettingsHovering
			) {
			isMouseHoveringUI = false;
		}
		else {
			isMouseHoveringUI = true;
		}
		if (buttonPixelDrawingHovering && IsMouseButtonPressed(0)) {
			enableBlur = false;
		}
		if (buttonBlurHovering && IsMouseButtonPressed(0)) {
			enablePixelDrawing = false;
		}
	}
	else {
		if (buttonShowSettingsHovering) {
			isMouseHoveringUI = false;
		}
		else {
			isMouseHoveringUI = true;
		}
	}


	if (IsKeyPressed(KEY_B)) {
		enableBlur = !enableBlur;
		enablePixelDrawing = false;
	}

	if (IsKeyPressed(KEY_P)) {
		enablePixelDrawing = !enablePixelDrawing;
		enableBlur = false;
	}



	for (Planet& planet : planets) {

		if (!colorVisualsEnabled) {
			if (!planet.customColor) {
				planet.color = { 128, 128, 128, 100 };
			}
		}
		if (enableBlur) {

			for (int i = 1; i <= 3; i++) {

				float t = static_cast<float>(i) / 90.0f;

				float falloff = 1.0f - (t * t);

				int blurSize = planet.size + static_cast<int>(35 * t);

				float newAlpha = planet.color.a * falloff / 5;
				if (newAlpha < 0.0f) newAlpha = 0.0f;
				if (newAlpha > 255.0f) newAlpha = 255.0f;

				Color blurColor = { planet.color.r, planet.color.g, planet.color.b, static_cast<unsigned char>(newAlpha) };

				DrawCircle(planet.pos.x, planet.pos.y, blurSize, blurColor);
			}
		}
		else if (enablePixelDrawing && planet.drawPixel) {

			DrawPixel(planet.pos.x, planet.pos.y, planet.color);


		}
		else {
			DrawCircle(planet.pos.x, planet.pos.y, planet.size, planet.color);

		}
	}

	for (const MouseTrailDot& dot : dots) {
		DrawPixel(dot.pos.x, dot.pos.y, YELLOW);
	}
	DrawText(TextFormat("Particles: %i", planets.size()), 50, 50, 25, WHITE);

	if (GetFPS() >= 60) {
		DrawText(TextFormat("FPS: %i", GetFPS()), screenWidth - 150, 50, 18, GREEN);

	}
	else if (GetFPS() < 60 && GetFPS() > 30) {
		DrawText(TextFormat("FPS: %i", GetFPS()), screenWidth - 150, 50, 18, YELLOW);
	}
	else {
		DrawText(TextFormat("FPS: %i", GetFPS()), screenWidth - 150, 50, 18, RED);
	}

	if (colorVisualsEnabled) {
		particlesColorVisuals(planets);
	}

}

static void enableMultiThreading() {
	if (isMultiThreadingEnabled) {
		omp_set_num_threads(14);
	}
	else {
		omp_set_num_threads(1);
	}
}

int main() {



	int planetPosX = 0;
	int planetPosY = 0;
	int planetSize = 0;

	std::vector<Planet> vectorPlanet;

	std::vector<MouseTrailDot> trailDots;

	bool isMouseHoveringUI = false;


	InitWindow(screenWidth, screenHeight, "n-Body");
	SetTargetFPS(targetFPS);

	while (!WindowShouldClose()) {


		BeginDrawing();

		ClearBackground(BLACK);

		BeginBlendMode(1);

		mouseTrail(trailDots, vectorPlanet);

		drawScene(vectorPlanet, trailDots, isMouseHoveringUI);

		EndBlendMode();

		updateScene(vectorPlanet, isMouseHoveringUI);

		enableMultiThreading();

		EndDrawing();
	}

	CloseWindow();



	return 0;
}