#include <iostream>
#include <vector>
#include "raylib.h"
#include <cmath>
#include <array>
#include <omp.h>
#include <thread>

#include "slingshot.h"
#include "quadtree.h"
#include "planet.h"
#include "mouseTrailDot.h"
#include "button.h"
#include "screenCapture.h"


#include "morton.h"

int screenWidth = 1024;
int screenHeight = 1024;

int targetFPS = 144;
constexpr double G = 6.674e-11;

bool trailsEnabled = false;
bool enableBlur = false;
bool enablePixelDrawing = true;
bool isFourierSpaceEnabled = true;
bool isMultiThreadingEnabled = true;
bool barnesHutEnabled = true;
bool isDarkMatterEnabled = false;
bool colorVisualsEnabled = false;


const float fixedDeltaTime = 0.03f;


static Quadtree gridFunction(std::vector<ParticlePhysics> pParticles) {
	Quadtree grid(
		0,             // posX
		0,             // posY
		screenWidth,   // size
		pParticles,  // planets
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


static Vector2 calculateForceFromGrid(ParticlePhysics& pParticles, const Quadtree& grid) {

	Vector2 totalForce = { 0.0f, 0.0f };

	if (grid.gridMass <= 0) return totalForce;

	float dx = grid.centerOfMass.x - pParticles.pos.x;

	if (isFourierSpaceEnabled) {
		if (dx > screenWidth / 2)
			dx -= screenWidth;
		else if (dx < -screenWidth / 2)
			dx += screenWidth;
	}

	float dy = grid.centerOfMass.y - pParticles.pos.y;

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

		if (grid.myParticles.size() == 1 &&
			fabs(grid.myParticles[0].pos.x - pParticles.pos.x) < 0.001f &&
			fabs(grid.myParticles[0].pos.y - pParticles.pos.y) < 0.001f) {
			return totalForce;
		}
		double force = G * pParticles.mass * grid.gridMass / distanceSq;
		totalForce.x = (dx / distance) * force;
		totalForce.y = (dy / distance) * force;

	}
	else {
		for (const Quadtree& subGrid : grid.subGrids) {
			Vector2 childForce = calculateForceFromGrid(pParticles, subGrid);
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


static Vector2 darkMatterForce(const ParticlePhysics& pParticles) {
	float centerX = screenWidth / 2.0f;
	float centerY = screenHeight / 2.0f;

	float dx = pParticles.pos.x - centerX;
	float dy = pParticles.pos.y - centerY;
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
	force.x = -(dx / radius) * acceleration * pParticles.mass; // Force = mass * acceleration
	force.y = -(dy / radius) * acceleration * pParticles.mass;

	return force;
}


void pairWiseGravity(std::vector<ParticlePhysics>& pParticles) {

	float timeStepMultiplier = 1;
	float deltaTime = GetFrameTime() * timeStepMultiplier;
#pragma omp parallel for schedule(dynamic)
	for (size_t i = 0; i < pParticles.size(); ++i) {
		for (size_t j = i + 1; j < pParticles.size(); ++j) {
			ParticlePhysics& pParticleA = pParticles[i];
			ParticlePhysics& pParticleB = pParticles[j];

			float accelPlanetAX = 0;
			float accelPlanetAY = 0;

			double prevAccAX = accelPlanetAX;
			double prevAccAY = accelPlanetAY;

			float accelPlanetBX = 0;
			float accelPlanetBY = 0;

			double prevAccBX = accelPlanetBX;
			double prevAccBY = accelPlanetBY;

			// Calculate gravitational force between A and B
			float dx = pParticleB.pos.x - pParticleA.pos.x;
			if (dx > screenWidth / 2)
				dx -= screenWidth;
			else if (dx < -screenWidth / 2)
				dx += screenWidth;

			float dy = pParticleB.pos.y - pParticleA.pos.y;
			if (dy > screenHeight / 2)
				dy -= screenHeight;
			else if (dy < -screenHeight / 2)
				dy += screenHeight;
			float distanceSq = dx * dx + dy * dy;

			// Avoid division by zero
			if (distanceSq < 5.0f) distanceSq = 60.0f;

			float distance = sqrt(distanceSq);
			float force = G * pParticleA.mass * pParticleB.mass / distanceSq; // Total force magnitude


			//planetB.proximity = force;


			// Normalize direction
			float fx = (dx / distance) * force;
			float fy = (dy / distance) * force;

			accelPlanetAX = fx / pParticleA.mass;
			accelPlanetAY = fy / pParticleA.mass;


			accelPlanetBX = fx / pParticleB.mass;
			accelPlanetBY = fy / pParticleB.mass;

			pParticleA.velocity.x += fixedDeltaTime * ((3.0f / 2.0f)) * accelPlanetAX - ((1.0f / 2.0f)) * prevAccAX;
			pParticleA.velocity.y += fixedDeltaTime * ((3.0f / 2.0f)) * accelPlanetAY - ((1.0f / 2.0f)) * prevAccAY;

			pParticleB.velocity.x -= fixedDeltaTime * ((3.0f / 2.0f)) * accelPlanetBX - ((1.0f / 2.0f)) * prevAccBX;
			pParticleB.velocity.y -= fixedDeltaTime * ((3.0f / 2.0f)) * accelPlanetBY - ((1.0f / 2.0f)) * prevAccBY;

		}
	}
}

static void updateScene(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, bool& isMouseHoveringUI) {

	float timeStepMultiplier = 1;
	float deltaTime = GetFrameTime() * timeStepMultiplier;

	if (IsKeyPressed(KEY_F) && isFourierSpaceEnabled) {
		isFourierSpaceEnabled = false;
	}
	else if (IsKeyPressed(KEY_F) && !isFourierSpaceEnabled) {
		isFourierSpaceEnabled = true;
	}

	Quadtree grid = gridFunction(pParticles);


	if (isMouseHoveringUI) {
		Slingshot slingshot = slingshotObject.planetSlingshot(isDragging, isMouse0Pressed, isMouse2SpacePressed);

		if (IsMouseButtonReleased(0) && !IsKeyDown(KEY_SPACE)) {
			pParticles.emplace_back(
				Vector2{ static_cast<float>(GetMouseX()),static_cast<float>(GetMouseY()) },
				Vector2{ slingshot.normalizedX * slingshot.length,
				slingshot.normalizedY * slingshot.length },
				500000000000000
			);
			rParticles.emplace_back(
				Color{ 255, 255, 255, 255 },
				4.0f,
				true,
				true,
				false
			);
			isDragging = false;
		}

		if (IsMouseButtonDown(2)) {
			for (int i = 0; i < 70; i++) {
				pParticles.emplace_back(
					Vector2{ static_cast<float>(GetMouseX() + rand() % 50 - 25),static_cast<float>(GetMouseY() + rand() % 50 - 25) },
					Vector2{ 0,0 },
					200000000000
				);
				rParticles.emplace_back(
					Color{ 128, 128, 128, 100 },
					1,
					true,
					false,
					true
				);

			}
		}

		if (IsMouseButtonPressed(1) && !isDragging) {
			for (int i = 0; i < 40000; i++) {
				float galaxyCenterX = static_cast<float>(screenWidth / 2);
				float galaxyCenterY = static_cast<float>(screenHeight / 2);

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

				pParticles.emplace_back(
					Vector2{ posX, posY },
					Vector2{ velocityX, velocityY },
					50000000000
				);
				rParticles.emplace_back(
					Color{ 128, 128, 128, 100 },
					2.0f,
					true,
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

				pParticles.emplace_back(
					Vector2{ posX, posY },
					Vector2{
						velocityX + (slingshot.normalizedX * slingshot.length),
					velocityY + (slingshot.normalizedY * slingshot.length) },
					85000000000
					);
				rParticles.emplace_back(
					Color{ 128, 128, 128, 100 },
					1,
					true,
					false,
					true
				);
				isDragging = false;
			}
		}

		if (IsKeyPressed(KEY_G)) {
			for (int i = 0; i < 10000; i++) {
				pParticles.emplace_back(
					Vector2{ static_cast<float>(rand() % screenWidth), static_cast<float>(rand() % screenHeight) },
					Vector2{ 0,0 },
					500000000000
				);
				rParticles.emplace_back(
					Color{ 128, 128, 128, 100 },
					1,
					true,
					false,
					true
				);
			}
		}
	}


	if (barnesHutEnabled) {

#pragma omp parallel for schedule(dynamic)
		for (size_t i = 0; i < pParticles.size(); i++) {
			ParticlePhysics& pParticle = pParticles[i];  // Access by index (better for SIMD)

			double accX = 0;
			double accY = 0;

			double prevAccX = accX;
			double prevAccY = accY;

			Vector2 netForce = calculateForceFromGrid(pParticle, grid);

			if (isDarkMatterEnabled) {
				Vector2 dmForce = darkMatterForce(pParticle);
				netForce.x += dmForce.x;
				netForce.y += dmForce.y;
			}

			accX = netForce.x / pParticle.mass;
			accY = netForce.y / pParticle.mass;

			pParticle.velocity.x += fixedDeltaTime * ((3.0f / 2.0f)) * accX - ((1.0f / 2.0f)) * prevAccX;
			pParticle.velocity.y += fixedDeltaTime * ((3.0f / 2.0f)) * accY - ((1.0f / 2.0f)) * prevAccY;
		}

	}
	else {
		pairWiseGravity(pParticles);
	}


	for (ParticlePhysics& pParticle : pParticles) {
		pParticle.pos.x += pParticle.velocity.x * fixedDeltaTime;
		pParticle.pos.y += pParticle.velocity.y * fixedDeltaTime;

		//  THIS IS THE INFINITE SPACE CODE
		if (isFourierSpaceEnabled) {
			if (pParticle.pos.x < 0)
				pParticle.pos.x += screenWidth;
			else if (pParticle.pos.x >= screenWidth)
				pParticle.pos.x -= screenWidth;
			if (pParticle.pos.y < 0)
				pParticle.pos.y += screenHeight;
			else if (pParticle.pos.y >= screenHeight)
				pParticle.pos.y -= screenHeight;

		}
	}


}



static void particlesColorVisuals(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles) {

	const float densityRadius = 7.0f;
	const float densityRadiusSq = densityRadius * densityRadius;  // Precompute squared radius
	const int maxNeighbors = 30;

	std::vector<int> neighborCounts(pParticles.size(), 0);

#pragma omp parallel for schedule(dynamic)
	for (size_t i = 0; i < pParticles.size(); i++) {
		for (size_t j = i + 1; j < pParticles.size(); j++) {
			float dx = pParticles[i].pos.x - pParticles[j].pos.x;
			float dy = pParticles[i].pos.y - pParticles[j].pos.y;
			if (dx * dx + dy * dy < densityRadiusSq) {
				neighborCounts[i]++;
				neighborCounts[j]++;
			}
		}
	}

	for (size_t i = 0; i < pParticles.size(); i++) {
		float normalDensity = std::min(float(neighborCounts[i]) / maxNeighbors, 1.0f);
		if (!rParticles[i].customColor) {
			rParticles[i].color.r = static_cast<unsigned char>(normalDensity * 255);
			rParticles[i].color.g = static_cast<unsigned char>(normalDensity * 140);
			rParticles[i].color.b = static_cast<unsigned char>(60);
			rParticles[i].color.a = static_cast<unsigned char>(100);
		}
	}
}


int trailDotFrameIndex = 0;
static void mouseTrail(std::vector<MouseTrailDot>& dots, std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles) {
	// Add new dots for all planets
	const int NUM_PLANETS = pParticles.size();

	if (IsKeyPressed(KEY_T)) {
		trailsEnabled = !trailsEnabled;
	}
	if (!trailsEnabled) {
		dots.clear();
	}

	if (trailsEnabled) {
		for (const ParticlePhysics& pParticle : pParticles) {
			dots.push_back(MouseTrailDot(pParticle.pos.x, pParticle.pos.y, 1));
		}
	}
	trailDotFrameIndex = 0;

	// Calculate how many dots to remove to stay at MAX_DOTS
	const int MAX_DOTS = 14 * pParticles.size();
	if (dots.size() > MAX_DOTS) {
		int excess = dots.size() - MAX_DOTS;
		dots.erase(dots.begin(), dots.begin() + excess); // Remove oldest `excess` dots
	}
	if (IsKeyPressed(KEY_C)) {
		pParticles.clear();
		rParticles.clear();
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
	)
};

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

static void drawScene(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, std::vector<MouseTrailDot>& dots, bool& isMouseHoveringUI) {

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



	for (int i = 0; i < pParticles.size(); ++i) {

		ParticlePhysics& pParticle = pParticles[i];
		ParticleRendering& rParticle = rParticles[i];

		if (!colorVisualsEnabled) {
			if (!rParticle.customColor) {
				rParticle.color = { 128, 128, 128, 100 };
			}
		}

		if (enablePixelDrawing && rParticle.drawPixel) {
			DrawPixelV({ pParticle.pos.x, pParticle.pos.y }, rParticle.color);
		}
		else if (enableBlur) {
			for (int i = 1; i <= 3; i++) {

				float t = static_cast<float>(i) / 90.0f;

				float falloff = 1.0f - (t * t);

				int blurSize = rParticle.size + static_cast<int>(35 * t);

				float newAlpha = rParticle.color.a * falloff / 5;
				if (newAlpha < 0.0f) newAlpha = 0.0f;
				if (newAlpha > 255.0f) newAlpha = 255.0f;

				Color blurColor = { rParticle.color.r, rParticle.color.g, rParticle.color.b, static_cast<unsigned char>(newAlpha) };

				DrawCircleV({ pParticle.pos.x, pParticle.pos.y }, blurSize, blurColor);
			}
		}
		else {
			DrawCircleV({ pParticle.pos.x, pParticle.pos.y }, rParticle.size, rParticle.color);

		}
	}

	for (const MouseTrailDot& dot : dots) {
		DrawPixel(dot.pos.x, dot.pos.y, YELLOW);
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

	if (colorVisualsEnabled) {
		particlesColorVisuals(pParticles, rParticles);
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

	std::vector<ParticlePhysics> pParticles;
	std::vector<ParticleRendering> rParticles;

	std::vector<MouseTrailDot> trailDots;

	bool isMouseHoveringUI = false;

	ScreenCapture screenCapture;

	Morton morton;

	InitWindow(screenWidth, screenHeight, "n-Body");
	SetTargetFPS(targetFPS);

	while (!WindowShouldClose()) {


		BeginDrawing();

		ClearBackground(BLACK);

		BeginBlendMode(1);

		morton.computeMortonKeys(pParticles);
		morton.sortParticlesByMortonKey(pParticles, rParticles);


		mouseTrail(trailDots, pParticles, rParticles);

		drawScene(pParticles, rParticles, trailDots, isMouseHoveringUI);

		EndBlendMode();

		updateScene(pParticles, rParticles, isMouseHoveringUI);

		enableMultiThreading();

		bool isRecording = screenCapture.screenGrab();

		if (isRecording) {
			DrawRectangleLines(0, 0, screenWidth, screenHeight, RED);

		}

		EndDrawing();
	}

	CloseWindow();



	return 0;
}