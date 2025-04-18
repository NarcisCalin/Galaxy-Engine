#include <iostream>
#include <vector>
#include <cmath>
#include <array>
#include <omp.h>
#include <thread>
#include <algorithm>
#include <bitset>

#include "../include/Particles/particle.h"
#include "../include/Physics/quadtree.h"
#include "../include/Physics/slingshot.h"
#include "../include/Particles/particleTrails.h"
#include "../include/UI/button.h"
#include "../include/UX/screenCapture.h"
#include "../include/Physics/morton.h"
#include "../include/UI/slider.h"
#include "../include/UX/camera.h"
#include "../include/raylib/raylib.h"
#include "../include/raylib/rlgl.h"
#include "../include/raylib/raymath.h"
#include "../include/UI/brush.h"
#include "../include/Particles/particleSelection.h"
#include "../include/Particles/particleSubdivision.h"
#include "../include/Particles/densitySize.h"
#include "../include/Particles/particleColorVisuals.h"
#include "../include/UI/rightClickSettings.h"
#include "../include/UI/controls.h"
#include "../include/Particles/particleDeletion.h"
#include "../include/Particles/particlesSpawning.h"
#include "../include/UI/UI.h"
#include "../include/Physics/physics.h"
#include "../include/parameters.h"


UpdateParameters myParam;
UpdateVariables myVar;
UI myUI;
Physics physics;


static Quadtree* gridFunction(std::vector<ParticlePhysics>& pParticles,
	std::vector<ParticleRendering>& rParticles) {
	Quadtree* grid = Quadtree::boundingBox(pParticles, rParticles);
	return grid;
}


void flattenQuadtree(Quadtree* node, std::vector<Quadtree*>& flatList) {
	if (!node) return;

	flatList.push_back(node);

	for (const auto& child : node->subGrids) {
		flattenQuadtree(child.get(), flatList);
	}
}

static void updateScene() {
	Quadtree* grid = nullptr;

	myVar.G = 6.674e-11 * myVar.gravityMultiplier;

	if (IsKeyPressed(KEY_SPACE)) {
		myVar.isTimeStopped = !myVar.isTimeStopped;
	}

	if (myVar.isTimeStopped) {
		myVar.timeStepMultiplier = 0.0f;
	}
	else {
		myVar.timeStepMultiplier = myVar.timeStepMultiplierSlider;
	}

	myVar.timeFactor = myVar.fixedDeltaTime * myVar.timeStepMultiplier;

	//if (myVar.timeFactor == 0) {
	//	myParam.morton.computeMortonKeys(myParam.pParticles, grid->boundingBoxPos, grid->boundingBoxSize);
	//	myParam.morton.sortParticlesByMortonKey(myParam.pParticles, myParam.rParticles);
	//}

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

	myParam.particlesSpawning.particlesInitialConditions(grid, physics, myVar, myParam);


	if (myVar.timeFactor > 0.0f && grid != nullptr) {
		if (myVar.isBarnesHutEnabled) {
#pragma omp parallel for schedule(dynamic)
			for (size_t i = 0; i < myParam.pParticles.size(); i++) {
				ParticlePhysics& pParticle = myParam.pParticles[i];

				Vector2 netForce = physics.calculateForceFromGrid(*grid, myParam.pParticles, myVar, pParticle);

				pParticle.acc.x = netForce.x / pParticle.mass;
				pParticle.acc.y = netForce.y / pParticle.mass;
			}
		}
		else {
			physics.pairWiseGravity(myParam.pParticles, myVar);
		}


		if (myVar.isCollisionsEnabled) {
			physics.collisions(myParam.pParticles, myParam.rParticles, myVar.softening, myVar.particleTextureHalfSize);
		}

		physics.physicsUpdate(myParam.pParticles, myParam.rParticles, myVar);
	}



	myParam.trails.trailLogic(myVar, myParam);

	myParam.myCamera.cameraFollowObject(myVar, myParam);

	myParam.particleSelection.clusterSelection(myVar, myParam);

	myParam.particleSelection.particleSelection(myVar, myParam);

	myParam.particleSelection.manyClustersSelection(myVar, myParam);

	myParam.particleSelection.boxSelection(myParam);

	myParam.particleSelection.invertSelection(myParam.rParticles);

	myParam.particleSelection.deselection(myParam.rParticles);

	myParam.particleSelection.selectedParticlesStoring(myParam);

	myParam.densitySize.sizeByDensity(myParam.pParticles, myParam.rParticles, myVar.isDensitySizeEnabled, myVar.isForceSizeEnabled,
		myVar.particleSizeMultiplier);

	myParam.particleDeletion.deleteSelected(myParam.pParticles, myParam.rParticles);

	myParam.particleDeletion.deleteNonImportanParticles(myParam.pParticles, myParam.rParticles);

	myParam.brush.particlesAttractor(myVar, myParam);

	myParam.brush.particlesSpinner(myVar, myParam);

	myParam.brush.particlesGrabber(myVar, myParam);

	myParam.brush.eraseBrush(myVar, myParam);

	if (grid != nullptr) {
		delete grid;
	}
}


static void drawScene(Texture2D& particleBlurTex, RenderTexture2D& myUITexture) {

	for (int i = 0; i < myParam.pParticles.size(); ++i) {

		ParticlePhysics& pParticle = myParam.pParticles[i];
		ParticleRendering& rParticle = myParam.rParticles[i];


		// Texture size is set to 16 because that is the particle texture half size in pixels
		DrawTextureEx(particleBlurTex, { static_cast<float>(pParticle.pos.x - rParticle.size * myVar.particleTextureHalfSize),
			static_cast<float>(pParticle.pos.y - rParticle.size * myVar.particleTextureHalfSize) }, 0, rParticle.size, rParticle.color);


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

	EndTextureMode();
	//EVERYTHING INTENDED TO APPEAR WHILE RECORDING ABOVE


	//END OF PARTICLES RENDER PASS
	//-------------------------------------------------\\
	//BEGINNNG OF UI RENDER PASS


	//EVERYTHING NOT INTENDED TO APPEAR WHILE RECORDING BELOW
	BeginTextureMode(myUITexture);

	ClearBackground({ 0,0,0,0 });

	Vector2 mouseScreenPos = GetMousePosition();

	BeginMode2D(myParam.myCamera.camera);

	myVar.mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), myParam.myCamera.camera);
	myParam.brush.drawBrush(myVar.mouseWorldPos);
	DrawRectangleLinesEx({ 0,0, static_cast<float>(myVar.domainWidth), static_cast<float>(myVar.domainHeight) }, 3, GRAY);

	// Z-Curves debug toggle
	if (myParam.pParticles.size() > 1 && myVar.drawZCurves) {
		for (size_t i = 0; i < myParam.pParticles.size() - 1; i++) {
			DrawLineV(myParam.pParticles[i].pos, myParam.pParticles[i + 1].pos, WHITE);

			DrawText(TextFormat("%i", i), static_cast<int>(myParam.pParticles[i].pos.x), static_cast<int>(myParam.pParticles[i].pos.y) - 10, 10, { 128,128,128,128 });
		}
	}

	EndMode2D();

	// EVERYTHING NON-STATIC RELATIVE TO CAMERA ABOVE

	// EVERYTHING STATIC RELATIVE TO CAMERA BELOW

	myUI.uiLogic(myParam, myVar);

	myParam.subdivision.subdivideParticles(myVar, myParam);

	EndTextureMode();
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

	SetConfigFlags(FLAG_MSAA_4X_HINT);

	InitWindow(myVar.screenWidth, myVar.screenHeight, "n-Body");

	Texture2D particleBlurTex = LoadTexture("Textures/ParticleBlur.png");

	Shader myBloom = LoadShader(nullptr, "Shaders/bloom.fs");

	RenderTexture2D myParticlesTexture = LoadRenderTexture(myVar.screenWidth, myVar.screenHeight);
	RenderTexture2D myUITexture = LoadRenderTexture(myVar.screenWidth, myVar.screenHeight);

	SetTargetFPS(myVar.targetFPS);

	while (!WindowShouldClose()) {


		BeginTextureMode(myParticlesTexture);

		ClearBackground(BLACK);

		BeginBlendMode(myParam.colorVisuals.blendMode);

		BeginMode2D(myParam.myCamera.cameraLogic());

		updateScene();
		drawScene(particleBlurTex, myUITexture);

		EndMode2D();

		EndBlendMode();

		//------------------------ RENDER TEXTURES BELOW ------------------------\\
		
		if (myVar.isGlowEnabled) {
			BeginShaderMode(myBloom);
		}

		DrawTextureRec(
			myParticlesTexture.texture,
			Rectangle{ 0, 0, static_cast<float>(myParticlesTexture.texture.width), -static_cast<float>(myParticlesTexture.texture.height) },
			Vector2{ 0, 0 },
			WHITE
		);

		if (myVar.isGlowEnabled) {
			EndShaderMode();
		}

		DrawTextureRec(
			myUITexture.texture,
			Rectangle{ 0, 0, static_cast<float>(myUITexture.texture.width), -static_cast<float>(myUITexture.texture.height) },
			Vector2{ 0, 0 },
			WHITE
		);

		myVar.isRecording = myParam.screenCapture.screenGrab(myParticlesTexture, myVar);

		if (myVar.isRecording) {
			DrawRectangleLinesEx({ 0,0, static_cast<float>(myVar.screenWidth), static_cast<float>(myVar.screenHeight) }, 3, RED);
		}

		EndDrawing();


		enableMultiThreading();
	}

	UnloadShader(myBloom);
	UnloadTexture(particleBlurTex);
	UnloadRenderTexture(myParticlesTexture);
	UnloadRenderTexture(myUITexture);

	CloseWindow();



	return 0;
}