#include <iostream>
#include <vector>
#include "raylib.h"
#include <cmath>
#include <array>
#include <omp.h>
#include <thread>
#include <algorithm>
#include <bitset>

#include "particle.h"
#include "quadtree.h"
#include "slingshot.h"
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
#include "particleColorVisuals.h"
#include "rightClickSettings.h"
#include "controls.h"
#include "particleDeletion.h"
#include "particlesSpawning.h"
#include "UI.h"
#include "parameters.h"

UpdateParameters myParam;
UpdateVariables myVar;
UI myUI;

static Quadtree* gridFunction(std::vector<ParticlePhysics>& pParticles,
	std::vector<ParticleRendering>& rParticles) {
	Quadtree* grid = Quadtree::boundingBox(pParticles, rParticles);
	//grid->calculateMasses(pParticles);
	return grid;
}



static Vector2 calculateForceFromGrid(ParticlePhysics& pParticle,
	const Quadtree& grid,
	const std::vector<ParticlePhysics>& pParticles) {
	Vector2 totalForce = { 0.0f, 0.0f };

	if (grid.gridMass <= 0)
		return totalForce;

	float dx = grid.centerOfMass.x - pParticle.pos.x;
	float dy = grid.centerOfMass.y - pParticle.pos.y;

	if (myVar.isPeriodicBoundaryEnabled) {
		dx -= myVar.screenWidth * ((dx > myVar.halfScreenWidth) - (dx < -myVar.halfScreenWidth));
		dy -= myVar.screenHeight * ((dy > myVar.halfScreenHeight) - (dy < -myVar.halfScreenHeight));
	}

	float distanceSq = dx * dx + dy * dy + myVar.softening * myVar.softening;

	if ((grid.size * grid.size < myVar.theta * myVar.theta * distanceSq) || grid.subGrids.empty()) {
		if ((grid.endIndex - grid.startIndex) == 1 &&
			fabs(pParticles[grid.startIndex].pos.x - pParticle.pos.x) < 0.001f &&
			fabs(pParticles[grid.startIndex].pos.y - pParticle.pos.y) < 0.001f) {
			return totalForce;
		}

		float invDistance = 1.0f / sqrt(distanceSq);
		float forceMagnitude = static_cast<float>(myVar.G) * pParticle.mass * grid.gridMass * invDistance * invDistance * invDistance;
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
	float centerX = myVar.screenWidth / 2.0f;
	float centerY = myVar.screenHeight / 2.0f;

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

	float acceleration = static_cast<float>(myVar.G * M_enclosed) / (radius * radius);

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

			if (myVar.isPeriodicBoundaryEnabled) {
				if (myVar.isPeriodicBoundaryEnabled) {
					if (dx > myVar.screenWidth / 2)
						dx -= myVar.screenWidth;
					else if (dx < -myVar.screenWidth / 2)
						dx += myVar.screenWidth;

					if (dy > myVar.screenHeight / 2)
						dy -= myVar.screenHeight;
					else if (dy < -myVar.screenHeight / 2)
						dy += myVar.screenHeight;
				}
			}

			float distanceSq = dx * dx + dy * dy + myVar.softening * myVar.softening;

			float distance = sqrt(distanceSq);
			float force = static_cast<float>(myVar.G * pParticleA.mass * pParticleB.mass / distanceSq);

			float fx = (dx / distance) * force;
			float fy = (dy / distance) * force;

			accelPlanetAX = fx / pParticleA.mass;
			accelPlanetAY = fy / pParticleA.mass;


			accelPlanetBX = fx / pParticleB.mass;
			accelPlanetBY = fy / pParticleB.mass;

			pParticleA.velocity.x += (myVar.fixedDeltaTime * ((3.0f / 2.0f)) * accelPlanetAX - ((1.0f / 2.0f)) * prevAccAX) * myVar.timeStepMultiplier;
			pParticleA.velocity.y += (myVar.fixedDeltaTime * ((3.0f / 2.0f)) * accelPlanetAY - ((1.0f / 2.0f)) * prevAccAY) * myVar.timeStepMultiplier;

			pParticleB.velocity.x -= (myVar.fixedDeltaTime * ((3.0f / 2.0f)) * accelPlanetBX - ((1.0f / 2.0f)) * prevAccBX) * myVar.timeStepMultiplier;
			pParticleB.velocity.y -= (myVar.fixedDeltaTime * ((3.0f / 2.0f)) * accelPlanetBY - ((1.0f / 2.0f)) * prevAccBY) * myVar.timeStepMultiplier;

		}
	}
}

static void physicsUpdate(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles) {
	if (myVar.isPeriodicBoundaryEnabled) {
		for (size_t i = 0; i < pParticles.size(); i++) {
			ParticlePhysics& pParticle = pParticles[i];
			pParticle.pos.x += pParticle.velocity.x * myVar.timeFactor;
			pParticle.pos.y += pParticle.velocity.y * myVar.timeFactor;
			if (pParticle.pos.x < 0) pParticle.pos.x += myVar.screenWidth;
			else if (pParticle.pos.x >= myVar.screenWidth) pParticle.pos.x -= myVar.screenWidth;

			if (pParticle.pos.y < 0) pParticle.pos.y += myVar.screenHeight;
			else if (pParticle.pos.y >= myVar.screenHeight) pParticle.pos.y -= myVar.screenHeight;
		}
	}
	else {
		for (size_t i = 0; i < pParticles.size(); i++) {
			ParticlePhysics& pParticle = pParticles[i];
			pParticle.pos.x += pParticle.velocity.x * myVar.timeFactor;
			pParticle.pos.y += pParticle.velocity.y * myVar.timeFactor;
			if (pParticles[i].pos.x < 0 || pParticles[i].pos.x >= myVar.screenWidth || pParticles[i].pos.y < 0 || pParticles[i].pos.y >= myVar.screenHeight) {
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
static void updateScene() {

	if (IsKeyPressed(KEY_SPACE)) {
		myVar.isTimeStopped = !myVar.isTimeStopped;
	}
	if (myVar.isTimeStopped) {
		myVar.timeStepMultiplier = 0.0f;
	}

	Quadtree* grid = nullptr;

	myVar.G = 6.674e-11 * myVar.gravityMultiplier;

	myVar.timeFactor = myVar.fixedDeltaTime * myVar.timeStepMultiplier;

	if (myVar.timeFactor == 0) {
		myParam.morton.computeMortonKeys(myParam.pParticles, grid->boundingBoxPos, grid->boundingBoxSize);
		myParam.morton.sortParticlesByMortonKey(myParam.pParticles, myParam.rParticles);
	}

	if (myVar.timeFactor > 0) {

		grid = gridFunction(myParam.pParticles, myParam.rParticles);
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

	if (grid != nullptr && myVar.drawQuadtree) {
		grid->drawQuadtree();
	}

	myParam.brush.brushSize(myParam.myCamera.mouseWorldPos);

	myParam.particlesSpawning.particlesInitialConditions(myParam.pParticles, myParam.rParticles, myVar.isDragging,
		myVar.isMouseNotHoveringUI, myParam.myCamera, myVar.screenHeight, myVar.screenWidth, myParam.brush);

	if (myVar.timeFactor > 0.0f) {
		if (myVar.isBarnesHutEnabled) {
#pragma omp parallel for schedule(dynamic)
			for (size_t i = 0; i < myParam.pParticles.size(); i++) {
				ParticlePhysics& pParticle = myParam.pParticles[i];

				float accX = 0;
				float accY = 0;
				float prevAccX = accX;
				float prevAccY = accY;

				Vector2 netForce = calculateForceFromGrid(pParticle, *grid, myParam.pParticles);

				if (myVar.isDarkMatterEnabled) {
					Vector2 dmForce = darkMatterForce(pParticle);
					netForce.x += dmForce.x;
					netForce.y += dmForce.y;
				}

				accX = netForce.x / pParticle.mass;
				accY = netForce.y / pParticle.mass;

				pParticle.velocity.x += (myVar.timeFactor * ((3.0f / 2.0f) * accX - (1.0f / 2.0f) * prevAccX));
				pParticle.velocity.y += (myVar.timeFactor * ((3.0f / 2.0f) * accY - (1.0f / 2.0f) * prevAccY));
			}
		}
		else {
			pairWiseGravity(myParam.pParticles);
		}

		if (myVar.isCollisionsEnabled) {
			collisions(myParam.pParticles, myParam.rParticles, myVar.softening);
		}

		physicsUpdate(myParam.pParticles, myParam.rParticles);

	}
	myParam.trails.trailLogic(myParam.pParticles, myParam.rParticles, myParam.pParticlesSelected, myParam.rParticlesSelected,
		myVar.isGlobalTrailsEnabled, myVar.isSelectedTrailsEnabled, myVar.trailMaxLength, myVar.timeFactor, myVar.isLocalTrailsEnabled);

	myParam.myCamera.cameraFollowObject(myParam.pParticles, myParam.rParticles, myVar.isMouseNotHoveringUI, myVar.isSelectedTrailsEnabled,
		myParam.trails);

	myParam.particleSelection.clusterSelection(myParam.pParticles, myParam.rParticles,
		myParam.myCamera, myVar.isMouseNotHoveringUI, myParam.trails, myVar.isGlobalTrailsEnabled);

	myParam.particleSelection.particleSelection(myParam.pParticles, myParam.rParticles, myParam.myCamera,
		myVar.isMouseNotHoveringUI, myParam.trails, myVar.isGlobalTrailsEnabled);

	myParam.particleSelection.manyClustersSelection(myParam.pParticles, myParam.rParticles, myParam.trails, myVar.isGlobalTrailsEnabled);

	myParam.particleSelection.boxSelection(myParam.pParticles, myParam.rParticles, myParam.myCamera);

	myParam.particleSelection.invertSelection(myParam.rParticles);

	myParam.particleSelection.deselection(myParam.rParticles);

	myParam.particleSelection.selectedParticlesStoring(myParam.pParticles, myParam.rParticles, myParam.rParticlesSelected,
		myParam.pParticlesSelected);

	myParam.densitySize.sizeByDensity(myParam.pParticles, myParam.rParticles, myVar.isDensitySizeEnabled, myVar.particleSizeMultiplier);

	myParam.particleDeletion.deleteSelected(myParam.pParticles, myParam.rParticles);

	myParam.particleDeletion.deleteNonImportanParticles(myParam.pParticles, myParam.rParticles);

	myParam.subdivision.subdivideParticles(myParam.pParticles, myParam.rParticles, myVar.particleTextureSize,
		myVar.isMouseNotHoveringUI, myVar.isDragging);


	if (grid != nullptr) {
		delete grid;
	}
}


static void drawScene(Texture2D& particleBlurTex) {

	for (int i = 0; i < myParam.pParticles.size(); ++i) {

		ParticlePhysics& pParticle = myParam.pParticles[i];
		ParticleRendering& rParticle = myParam.rParticles[i];

		if (myVar.isPixelDrawingEnabled && rParticle.drawPixel) {
			DrawPixelV({ pParticle.pos.x, pParticle.pos.y }, rParticle.color);
		}
		else {
			// Texture size is set to 32 because that is the particle texture size in pixels
			DrawTextureEx(particleBlurTex, { static_cast<float>(pParticle.pos.x - rParticle.size * myVar.particleTextureSize / 2),
				static_cast<float>(pParticle.pos.y - rParticle.size * myVar.particleTextureSize / 2) }, 0, rParticle.size, rParticle.color);
		}

		if (!myVar.isDensitySizeEnabled) {

			if (rParticle.canBeResized) {
				rParticle.size = rParticle.previousSize * myVar.particleSizeMultiplier;
			}
			else {
				rParticle.size = rParticle.previousSize;
			}

		}
	}

	myParam.colorVisuals.particlesColorVisuals(myParam.pParticles, myParam.rParticles);

	myParam.trails.drawTrail(myParam.rParticles, particleBlurTex);


	//EVERYTHING INTENDED TO APPEAR WHILE RECORDING ABOVE
	myVar.isRecording = myParam.screenCapture.screenGrab();
	//EVERYTHING NOT INTENDED TO APPEAR WHILE RECORDING BELOW

	mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), myParam.myCamera.camera);

	myParam.brush.drawBrush(mouseWorldPos);

	DrawRectangleLinesEx({ 0,0, static_cast<float>(myVar.screenWidth), static_cast<float>(myVar.screenHeight) }, 3, GRAY);

	// Z-Curves debug toggle
	if (myParam.pParticles.size() > 1 && myVar.drawZCurves) {
		for (size_t i = 0; i < myParam.pParticles.size() - 1; i++) {
			DrawLineV(myParam.pParticles[i].pos, myParam.pParticles[i + 1].pos, WHITE);

			DrawText(TextFormat("%i", i), myParam.pParticles[i].pos.x, myParam.pParticles[i].pos.y - 10, 10, { 128,128,128,128 });
		}
	}

	// EVERYTHING NON-STATIC RELATIVE TO CAMERA ABOVE
	EndMode2D();
	// EVERYTHING STATIC RELATIVE TO CAMERA BELOW

	myUI.uiLogic(myParam, myVar);

	if (IsKeyPressed(KEY_P)) {
		myVar.isPixelDrawingEnabled = !myVar.isPixelDrawingEnabled;
	}
}



static void enableMultiThreading() {
	if (myVar.isMultiThreadingEnabled) {
		omp_set_num_threads(16);
	}
	else {
		omp_set_num_threads(1);
	}
}
int main() {

	InitWindow(myVar.screenWidth, myVar.screenHeight, "n-Body");

	Texture2D particleBlurTex = LoadTexture("Textures/ParticleBlur.png");

	SetTargetFPS(myVar.targetFPS);

	SetConfigFlags(FLAG_MSAA_4X_HINT);

	while (!WindowShouldClose()) {


		BeginDrawing();

		ClearBackground(BLACK);

		BeginBlendMode(myParam.colorVisuals.blendMode);

		BeginMode2D(myParam.myCamera.cameraLogic());

		updateScene();

		drawScene(particleBlurTex);

		EndBlendMode();


		enableMultiThreading();

		EndDrawing();
	}

	UnloadTexture(particleBlurTex);

	CloseWindow();



	return 0;
}