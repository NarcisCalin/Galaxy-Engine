#include "globalLogic.h"

UpdateParameters myParam;
UpdateVariables myVar;
UI myUI;
Physics physics;
ParticleSpaceship ship;
SPH sph;
SaveSystem save;

uint32_t globalId = 0;

std::unordered_map<unsigned int, uint64_t> NeighborSearch::idToIndex;

Quadtree* gridFunction(std::vector<ParticlePhysics>& pParticles,
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


// THIS FUNCTION IS MEANT FOR QUICK DEBUGGING WHERE YOU NEED TO CHECK A SPECIFIC PARTICLE'S VARIABLES
void selectedParticleDebug() {

	for (size_t i = 0; i < myParam.pParticles.size(); i++) {
		ParticlePhysics& p = myParam.pParticles[i];
		ParticleRendering& r = myParam.rParticles[i];
		if (r.isSelected && myVar.timeFactor != 0.0f) {
			std::cout << "Drawn: " << r.isBeingDrawn << std::endl;
		}
	}
}

void pinParticles() {

	if (myVar.pinFlag) {
		for (size_t i = 0; i < myParam.pParticles.size(); i++) {
			if (myParam.rParticles[i].isSelected) {
				myParam.rParticles[i].isPinned = true;
			}
		}
		myVar.pinFlag = false;
	}

	if (myVar.unPinFlag) {
		for (size_t i = 0; i < myParam.pParticles.size(); i++) {
			if (myParam.rParticles[i].isSelected) {
				myParam.rParticles[i].isPinned = false;
			}
		}
		myVar.unPinFlag = false;
	}
}

void updateScene() {

	Quadtree* grid = nullptr;

	myVar.G = 6.674e-11 * myVar.gravityMultiplier;

	if (IO::shortcutPress(KEY_SPACE)) {
		myVar.isTimePlaying = !myVar.isTimePlaying;
	}

	myVar.halfDomainWidth = myVar.domainSize.x * 0.5f;
	myVar.halfDomainHeight = myVar.domainSize.y * 0.5f;

	myVar.timeFactor = myVar.fixedDeltaTime * myVar.timeStepMultiplier * static_cast<float>(myVar.isTimePlaying);

	//if (myVar.timeFactor == 0) {
	//	myParam.morton.computeMortonKeys(myParam.pParticles, grid->boundingBoxPos, grid->boundingBoxSize);
	//	myParam.morton.sortParticlesByMortonKey(myParam.pParticles, myParam.rParticles);
	//}

	if (myVar.timeFactor > 0) {
		grid = gridFunction(myParam.pParticles, myParam.rParticles);
	}

	/*std::vector<Quadtree*> flatNodes;

	flattenQuadtree(grid, flatNodes);

	size_t index = 0;

	for (size_t i = 0; i < flatNodes.size(); i++) {
		Quadtree* node = flatNodes[i];
	}*/

	if (grid != nullptr && myVar.drawQuadtree) {
		grid->drawQuadtree();
	}


	for (ParticleRendering& rParticle : myParam.rParticles) {
		rParticle.totalRadius = rParticle.size * myVar.particleTextureHalfSize * myVar.particleSizeMultiplier;
	}

	for (size_t i = 0; i < myParam.pParticles.size(); i++) {
		myParam.pParticles[i].neighborIds.clear();
	}

	myParam.brush.brushSize();

	if (myVar.constraintsEnabled || myVar.drawConstraints || myVar.visualizeMesh || myVar.isBrushDrawing) {
		NeighborSearch::idToI(myParam.pParticles);
		myParam.neighborSearch.neighborSearchHash(myParam.pParticles, myParam.rParticles);
	}

	myParam.particlesSpawning.particlesInitialConditions(grid, physics, myVar, myParam);

	if (myVar.constraintsEnabled && !myVar.isBrushDrawing) {
		physics.createConstraints(myParam.pParticles, myParam.rParticles, myVar.constraintAfterDrawingFlag, myVar);
	}
	else if(!myVar.constraintsEnabled && !myVar.isBrushDrawing) {
		physics.constraintMap.clear();
		physics.particleConstraints.clear();
	}

	myParam.particlesSpawning.copyPaste(myParam.pParticles, myParam.rParticles, myVar.isDragging, myParam.myCamera, myParam.pParticlesSelected, 
		physics, myVar, myParam);

	if (myVar.timeFactor > 0.0f && grid != nullptr) {

		for (size_t i = 0; i < myParam.pParticles.size(); i++) {
			myParam.pParticles[i].acc = { 0.0f, 0.0f };
		}

#pragma omp parallel for schedule(dynamic)
		for (size_t i = 0; i < myParam.pParticles.size(); i++) {

			if (myParam.rParticles[i].isBeingDrawn && myVar.isBrushDrawing && myVar.isSPHEnabled) {
				continue;
			}

			ParticlePhysics& pParticle = myParam.pParticles[i];

			glm::vec2 netForce = physics.calculateForceFromGrid(*grid, myParam.pParticles, myVar, pParticle);

			pParticle.acc = netForce / pParticle.mass;
		}

		if (myVar.isSPHEnabled) {
			sph.pcisphSolver(myParam.pParticles, myParam.rParticles, myVar.timeFactor, myVar.domainSize, myVar.sphGround);
		}

		if (myVar.isTempEnabled) {
			physics.temperatureCalculation(myParam.pParticles, myParam.rParticles, myVar);
		}

		ship.spaceshipLogic(myParam.pParticles, myParam.rParticles, myVar.isShipGasEnabled);

		physics.constraints(myParam.pParticles, myParam.rParticles, myVar);

		physics.physicsUpdate(myParam.pParticles, myParam.rParticles, myVar, myVar.sphGround);
	}

	if (myVar.isDensitySizeEnabled || myParam.colorVisuals.densityColor) {
		myParam.neighborSearch.neighborSearch(myParam.pParticles, myParam.rParticles, myVar.particleSizeMultiplier, myVar.particleTextureHalfSize);
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

	myParam.particleDeletion.deleteStrays(myParam.pParticles, myParam.rParticles, myVar.isSPHEnabled);

	myParam.brush.particlesAttractor(myVar, myParam);

	myParam.brush.particlesSpinner(myVar, myParam);

	myParam.brush.particlesGrabber(myParam);

	myParam.brush.temperatureBrush(myParam);

	myParam.brush.eraseBrush(myParam);

	pinParticles();

	//selectedParticleDebug();

	if (grid != nullptr) {
		delete grid;
	}
}

void drawConstraints() {

	// TODO: ADD A COLOR LERP FOR CONSTRAINTS

	if (myVar.visualizeMesh) {
		rlBegin(RL_LINES);
		for (size_t i = 0; i < myParam.pParticles.size(); i++) {
			ParticlePhysics& pi = myParam.pParticles[i];
			for (uint32_t& id : myParam.pParticles[i].neighborIds) {
				auto it = NeighborSearch::idToIndex.find(id);
				if (it != NeighborSearch::idToIndex.end()) {

					size_t neighborIndex = it->second;

					if (neighborIndex == i) continue;

					auto& pj = myParam.pParticles[neighborIndex];

					if (pi.id < pj.id) {
						glm::vec2 delta = pj.pos - pi.pos;
						glm::vec2 periodicDelta = delta;

						if (abs(delta.x) > myVar.domainSize.x * 0.5f) {
							periodicDelta.x += (delta.x > 0) ? -myVar.domainSize.x : myVar.domainSize.x;
						}
						if (abs(delta.y) > myVar.domainSize.y * 0.5f) {
							periodicDelta.y += (delta.y > 0) ? -myVar.domainSize.y : myVar.domainSize.y;
						}

						glm::vec2 pjCorrectedPos = pi.pos + periodicDelta;

						Color lineColor = ColorLerp(myParam.rParticles[i].color, myParam.rParticles[neighborIndex].color, 0.5f);
						rlColor4ub(lineColor.r, lineColor.g, lineColor.b, lineColor.a);
						rlVertex2f(pi.pos.x, pi.pos.y);
						rlVertex2f(pjCorrectedPos.x, pjCorrectedPos.y);
					}
				}
			}
		}
		rlEnd();
	}

	if (myVar.drawConstraints && !physics.particleConstraints.empty()) {
		rlBegin(RL_LINES);
		for (size_t i = 0; i < physics.particleConstraints.size(); i++) {
			auto& constraint = physics.particleConstraints[i];
			auto it1 = NeighborSearch::idToIndex.find(constraint.id1);
			auto it2 = NeighborSearch::idToIndex.find(constraint.id2);

			if (it1 == NeighborSearch::idToIndex.end() ||
				it2 == NeighborSearch::idToIndex.end()) {
				continue;
			}

			ParticlePhysics& pi = myParam.pParticles[it1->second];
			ParticlePhysics& pj = myParam.pParticles[it2->second];

			glm::vec2 delta = pj.pos - pi.pos;
			glm::vec2 periodicDelta = delta;

			if (abs(delta.x) > myVar.domainSize.x * 0.5f) {
				periodicDelta.x += (delta.x > 0) ? -myVar.domainSize.x : myVar.domainSize.x;
			}
			if (abs(delta.y) > myVar.domainSize.y * 0.5f) {
				periodicDelta.y += (delta.y > 0) ? -myVar.domainSize.y : myVar.domainSize.y;
			}

			glm::vec2 pjCorrectedPos = pi.pos + periodicDelta;

			Color lineColor;
			if (myVar.constraintStressColor) {

				float maxStress = 0.0f;

				if (myVar.constraintMaxStressColor > 0.0f) {
					maxStress = myVar.constraintMaxStressColor;
				}
				else {
					maxStress = constraint.resistance * myVar.globalConstraintResistence * constraint.restLength * 0.18f; // The last multiplier is a heuristic
				}

				float clampedStress = std::clamp(constraint.displacement, 0.0f, maxStress);
				float normalizedStress = clampedStress / maxStress;

				float hue = (1.0f - normalizedStress) * 240.0f;
				float saturation = 1.0f;
				float value = 1.0f;

				lineColor = ColorFromHSV(hue, saturation, value);
			}
			else {
				lineColor = ColorLerp(myParam.rParticles[it1->second].color, myParam.rParticles[it2->second].color, 0.5f);
			}

			rlColor4ub(lineColor.r, lineColor.g, lineColor.b, lineColor.a);
			rlVertex2f(pi.pos.x, pi.pos.y);
			rlVertex2f(pjCorrectedPos.x, pjCorrectedPos.y);
		}
		rlEnd();
	}
}

void drawScene(Texture2D& particleBlurTex, RenderTexture2D& myUITexture) {

	for (int i = 0; i < myParam.pParticles.size(); ++i) {

		ParticlePhysics& pParticle = myParam.pParticles[i];
		ParticleRendering& rParticle = myParam.rParticles[i];


		// Texture size is set to 16 because that is the particle texture half size in pixels
		DrawTextureEx(particleBlurTex, { static_cast<float>(pParticle.pos.x - rParticle.size * myVar.particleTextureHalfSize),
			static_cast<float>(pParticle.pos.y - rParticle.size * myVar.particleTextureHalfSize) }, 0.0f, rParticle.size, rParticle.color);


		if (!myVar.isDensitySizeEnabled) {

			if (rParticle.canBeResized) {
				rParticle.size = rParticle.previousSize * myVar.particleSizeMultiplier;
			}
			else {
				rParticle.size = rParticle.previousSize;
			}
		}
	}


	myParam.colorVisuals.particlesColorVisuals(myParam.pParticles, myParam.rParticles, myVar.isTempEnabled);

	myParam.trails.drawTrail(myParam.rParticles, particleBlurTex);

	EndTextureMode();
	//EVERYTHING INTENDED TO APPEAR WHILE RECORDING ABOVE


	//END OF PARTICLES RENDER PASS
	//-------------------------------------------------//
	//BEGINNNG OF UI RENDER PASS


	//EVERYTHING NOT INTENDED TO APPEAR WHILE RECORDING BELOW
	BeginTextureMode(myUITexture);

	ClearBackground({ 0,0,0,0 });

	BeginMode2D(myParam.myCamera.camera);

	myVar.mouseWorldPos = glm::vec2(GetScreenToWorld2D(GetMousePosition(), myParam.myCamera.camera).x,
		GetScreenToWorld2D(GetMousePosition(), myParam.myCamera.camera).y);
	myParam.brush.drawBrush(myVar.mouseWorldPos);
	DrawRectangleLinesEx({ 0,0, static_cast<float>(myVar.domainSize.x), static_cast<float>(myVar.domainSize.y) }, 3, GRAY);

	drawConstraints();

	// Z-Curves debug toggle
	if (myParam.pParticles.size() > 1 && myVar.drawZCurves) {
		for (size_t i = 0; i < myParam.pParticles.size() - 1; i++) {
			DrawLineV({ myParam.pParticles[i].pos.x, myParam.pParticles[i].pos.y }, { myParam.pParticles[i + 1].pos.x,myParam.pParticles[i + 1].pos.y }, WHITE);

			DrawText(TextFormat("%i", i), static_cast<int>(myParam.pParticles[i].pos.x), static_cast<int>(myParam.pParticles[i].pos.y) - 10, 10, { 128,128,128,128 });
		}
	}

	EndMode2D();

	// EVERYTHING NON-STATIC RELATIVE TO CAMERA ABOVE

	// EVERYTHING STATIC RELATIVE TO CAMERA BELOW

	myUI.uiLogic(myParam, myVar, sph, save);

	save.saveLoadLogic(myVar, myParam, sph, physics);

	myParam.subdivision.subdivideParticles(myVar, myParam);

	EndTextureMode();
}


void enableMultiThreading() {
	if (myVar.isMultiThreadingEnabled) {
		omp_set_num_threads(myVar.threadsAmount);
	}
	else {
		omp_set_num_threads(1);
	}
}

void fullscreenToggle(int& lastScreenWidth, int& lastScreenHeight,
	bool& wasFullscreen, bool& lastScreenState,
	RenderTexture2D& myParticlesTexture, RenderTexture2D& myUITexture) {

	if (myVar.fullscreenState != lastScreenState) {
		int monitor = GetCurrentMonitor();

		if (!IsWindowFullscreen())
			SetWindowSize(GetMonitorWidth(monitor), GetMonitorHeight(monitor));
		else
			SetWindowSize(myVar.screenWidth, myVar.screenHeight);

		ToggleFullscreen();
		wasFullscreen = IsWindowFullscreen();

		UnloadRenderTexture(myParticlesTexture);
		UnloadRenderTexture(myUITexture);

		lastScreenWidth = GetScreenWidth();
		lastScreenHeight = GetScreenHeight();
		lastScreenState = myVar.fullscreenState;

		myParticlesTexture = LoadRenderTexture(lastScreenWidth, lastScreenHeight);
		myUITexture = LoadRenderTexture(lastScreenWidth, lastScreenHeight);
	}

	int currentScreenWidth = GetScreenWidth();
	int currentScreenHeight = GetScreenHeight();

	if (currentScreenWidth != lastScreenWidth || currentScreenHeight != lastScreenHeight) {
		UnloadRenderTexture(myParticlesTexture);
		UnloadRenderTexture(myUITexture);

		myParticlesTexture = LoadRenderTexture(currentScreenWidth, currentScreenHeight);
		myUITexture = LoadRenderTexture(currentScreenWidth, currentScreenHeight);

		lastScreenWidth = currentScreenWidth;
		lastScreenHeight = currentScreenHeight;
	}
}