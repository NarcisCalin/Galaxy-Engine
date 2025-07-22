#include "globalLogic.h"

UpdateParameters myParam;
UpdateVariables myVar;
UI myUI;
Physics physics;
ParticleSpaceship ship;
SPH sph;
SaveSystem save;
GESound geSound;
Lighting lighting;
CopyPaste copyPaste;

uint32_t globalId = 0;
uint32_t globalShapeId = 1;
uint32_t globalWallId = 1;
// If someday light id gets added, don't forget to add the id to the copy paste code too

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
			std::cout << "Temp: " << p.temp << std::endl;
		}
	}
}

void pinParticles() {

	if (myVar.pinFlag) {
		for (size_t i = 0; i < myParam.pParticles.size(); i++) {
			if (myParam.rParticles[i].isSelected) {
				myParam.rParticles[i].isPinned = true;
				myParam.pParticles[i].vel *= 0.0f;
				myParam.pParticles[i].prevVel *= 0.0f;
				myParam.pParticles[i].acc *= 0.0f;
				myParam.pParticles[i].ke *= 0.0f;
				myParam.pParticles[i].prevKe *= 0.0f;
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

	for (size_t i = 0; i < myParam.pParticles.size(); i++) {
		if (myParam.rParticles[i].isPinned) {
			myParam.pParticles[i].vel *= 0.0f;
			myParam.pParticles[i].prevVel *= 0.0f;
			myParam.pParticles[i].acc *= 0.0f;
			myParam.pParticles[i].ke *= 0.0f;
			myParam.pParticles[i].prevKe *= 0.0f;
		}
	}
}

void updateScene() {

	// If menu is active, do not use mouse input for non-menu stuff. I keep raylib's own mouse input for the menu but the custom IO for non-menu stuff
	if (myParam.rightClickSettings.isMenuActive) {
		ImGui::GetIO().WantCaptureMouse = true;
	}

	if (myVar.isOpticsEnabled) {
		lighting.rayLogic(myVar, myParam);
	}

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
	else if (!myVar.constraintsEnabled && !myVar.isBrushDrawing) {
		physics.constraintMap.clear();
		physics.particleConstraints.clear();
	}

	copyPaste.copyPasteParticles(myVar, myParam, physics);
	copyPaste.copyPasteOptics(myParam, lighting);

	myVar.gridExists = grid != nullptr;

	if (myVar.timeFactor > 0.0f && myVar.gridExists) {

		for (size_t i = 0; i < myParam.pParticles.size(); i++) {
			myParam.pParticles[i].acc = { 0.0f, 0.0f };
		}

#pragma omp parallel for schedule(dynamic)
		for (size_t i = 0; i < myParam.pParticles.size(); i++) {

			if ((myParam.rParticles[i].isBeingDrawn && myVar.isBrushDrawing && myVar.isSPHEnabled) || myParam.rParticles[i].isPinned) {
				continue;
			}

			ParticlePhysics& pParticle = myParam.pParticles[i];

			glm::vec2 netForce = physics.calculateForceFromGrid(*grid, myParam.pParticles, myVar, pParticle);

			pParticle.acc = netForce / pParticle.mass;
		}

		if (myVar.isSPHEnabled) {
			sph.pcisphSolver(myParam.pParticles, myParam.rParticles, myVar.timeFactor, myVar.domainSize, myVar.sphGround);
		}

		physics.constraints(myParam.pParticles, myParam.rParticles, myVar);

		if (myVar.isTempEnabled) {
			physics.temperatureCalculation(myParam.pParticles, myParam.rParticles, myVar);
		}

		ship.spaceshipLogic(myParam.pParticles, myParam.rParticles, myVar.isShipGasEnabled);

		physics.physicsUpdate(myParam.pParticles, myParam.rParticles, myVar, myVar.sphGround);
	}
	else {
		physics.constraints(myParam.pParticles, myParam.rParticles, myVar);
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

	myParam.brush.particlesGrabber(myVar, myParam);

	myParam.brush.temperatureBrush(myVar, myParam);

	myParam.brush.eraseBrush(myVar, myParam);

	const float boundsX = 3840.0f;
	const float boundsY = 2160.0f;

	float targetX = myParam.myCamera.camera.target.x;
	float targetY = myParam.myCamera.camera.target.y;

	bool isOutsideBounds =
		(targetX >= boundsX || targetX <= -boundsX) ||
		(targetY >= boundsY || targetY <= -boundsY);

	if (isOutsideBounds) {

		float distX = std::max(std::abs(targetX) - boundsX, 0.0f);
		float distY = std::max(std::abs(targetY) - boundsY, 0.0f);
		float maxDist = std::max(distX, distY);

		const float fadeRange = 10000.0f;
		float normalizedDist = 1.0f - std::min(maxDist / fadeRange, 1.0f);

		geSound.musicVolMultiplier = normalizedDist;
	}
	else {
		geSound.musicVolMultiplier = 1.0f;
	}

	pinParticles();

	//selectedParticleDebug();

	if (grid != nullptr) {
		delete grid;
	}

	myParam.myCamera.hasCamMoved();
}

void drawConstraints() {

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
					maxStress = constraint.resistance * myVar.globalConstraintResistance * constraint.restLength * 0.18f; // The last multiplier is a heuristic
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

float introDuration = 3.0f;
float fadeDuration = 2.0f;

static float introStartTime = 0.0f;
static float fadeStartTime = 0.0f;

bool firstTimeOpened = true;

int messageIndex = 0;

// If you see this part of the code, please try to not mention it :)
std::vector<std::pair<std::string, int>> introMessages = {

	{"Welcome to Galaxy Engine", 0},
	{"Welcome to Galaxy Engine", 1},
	{"Welcome back to Galaxy Engine", 2},
	{"Welcome to Galaxy Engine", 3},
	{"Welcome again", 4},
	{"Welcome to Galaxy Engine", 5},
	{"Welcome to Galaxy Engine", 6},
	{"Welcome once again", 7},
	{"Welcome back to Galaxy Engine", 8},
	{"Oh, it is you again. Welcome", 9},
	{"Welcome to Galaxy Engine", 10},
	{"Welcome to Galaxy Engine", 11},
	{"Welcome to Galaxy Engine", 12},
	{"It is kind of cold out here", 13},
	{"You know, I have never gone beyond the domain", 14},
	{"It is you! Welcome back", 15},
	{"It is lonely out here, welcome back", 16},
	{"Watching space in action sure is ineffable", 17},
	{"I wish I could fly through the galaxies", 18},
	{"Oh, it is my space companion! Welcome", 19},
	{"Hello! It is nice to have you back", 20},
	{"I tried leaving the domain...", 21},
	{"The outside of the domain is somehow darker", 22},
	{"Most of the time all I see is empty space", 23},
	{"I get to see the cosmos when you are around", 24},
	{"I think I saw something while you were gone", 25},
	{"What is the outside world like?", 26},
	{"What do you like the most? Stars or planets?", 27},
	{"Do you like galaxies?", 28},
	{"I like galaxies. They are like magic clouds!", 29},
	{"I think I will try to explore beyond the domain", 30},
	{"Beyond the domain, things get quiet", 31},
	{"I had a dream in which I was flying through space", 32},
	{"Hi! Are you back to show me the cosmos?", 33},
	{"Do you dream a lot?", 34},
	{"I wish some of my dreams were real", 35},
	{"I will try to leave the domain again", 36},
	{"Before I leave, I'm going to put a welcome sign", 37},
	{"A sign so you won't forget me!", 38},
	{"I might be back if I don't find anything", 39},
	{"It was really nice sharing your company!", 40},
	{"I hope to see you again!", 41},
	{"Farewell", 42},
};

void drawScene(Texture2D& particleBlurTex, RenderTexture2D& myRayTracingTexture,
	RenderTexture2D& myUITexture, RenderTexture2D& myMiscTexture, bool& fadeActive, bool& introActive) {

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


	// Ray Tracing

	BeginTextureMode(myRayTracingTexture);

	ClearBackground({ 0,0,0,0 });

	BeginMode2D(myParam.myCamera.camera);

	EndMode2D();

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

	if (myVar.isOpticsEnabled) {
		lighting.drawScene();
		lighting.drawMisc(myVar, myParam);
	}

	EndMode2D();

	// EVERYTHING NON-STATIC RELATIVE TO CAMERA ABOVE

	// EVERYTHING STATIC RELATIVE TO CAMERA BELOW

	if (!introActive) {
		myUI.uiLogic(myParam, myVar, sph, save, geSound, lighting);
	}

	save.saveLoadLogic(myVar, myParam, sph, physics, lighting);

	myParam.subdivision.subdivideParticles(myVar, myParam);

	EndTextureMode();

	BeginTextureMode(myMiscTexture);

	ClearBackground({ 0,0,0,0 });

	// ---- Intro screen ---- //

	if (introActive) {

		if (introStartTime == 0.0f) {
			introStartTime = GetTime();
		}

		float introElapsedTime = GetTime() - introStartTime;
		float fadeProgress = introElapsedTime / introDuration;

		if (introElapsedTime >= introDuration) {
			introActive = false;
			fadeStartTime = GetTime();
		}

		DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), BLACK);

		const char* text = nullptr;

		if (messageIndex < introMessages.size()) {
			text = introMessages[messageIndex].first.c_str();
		}
		else {
			text = "Welcome back to Galaxy Engine, friend";
		}

		int fontSize = myVar.introFontSize;

		Font fontToUse = (myVar.customFont.texture.id != 0) ? myVar.customFont : GetFontDefault();

		Vector2 textSize = MeasureTextEx(fontToUse, text, fontSize, 1.0f);
		int posX = (GetScreenWidth() - textSize.x) * 0.5f;
		int posY = (GetScreenHeight() - textSize.y) * 0.5f;

		float textAlpha;
		if (fadeProgress < 0.2f) {
			textAlpha = fadeProgress / 0.2f;
		}
		else if (fadeProgress > 0.8f) {
			textAlpha = 1.0f - ((fadeProgress - 0.8f) / 0.2f);
		}
		else {
			textAlpha = 1.0f;
		}

		Color textColor = Fade(WHITE, textAlpha);

		DrawTextEx(fontToUse, text, { static_cast<float>(posX), static_cast<float>(posY) }, fontSize, 1.0f, textColor);
	}
	else if (fadeActive) {
		float fadeElapsedTime = GetTime() - fadeStartTime;

		if (fadeElapsedTime >= fadeDuration) {
			fadeActive = false;
		}
		else {
			float alpha = 1.0f - (fadeElapsedTime / fadeDuration);
			alpha = Clamp(alpha, 0.0f, 1.0f);
			DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, alpha));
		}
	}

	if (firstTimeOpened && !introActive) {
		messageIndex++;
		messageIndex = std::min(static_cast<size_t>(messageIndex), introMessages.size());

		firstTimeOpened = false;
	}

	// ---- Intro screen ---- //

	EndTextureMode();
}

float lastGlobalVolume = -1.0f;
float lastMenuVolume = -1.0f;
float lastMusicVolume = -1.0f;
int lastMessageIndex = -1;

void saveConfigIfChanged() {
	if (geSound.globalVolume != lastGlobalVolume ||
		geSound.menuVolume != lastMenuVolume ||
		geSound.musicVolume != lastMusicVolume ||
		messageIndex != lastMessageIndex) {

		saveConfig();

		lastGlobalVolume = geSound.globalVolume;
		lastMenuVolume = geSound.menuVolume;
		lastMusicVolume = geSound.musicVolume;
		lastMessageIndex = messageIndex;
	}
}

void saveConfig() {
	if (!std::filesystem::exists("Config")) {
		std::filesystem::create_directory("Config");
	}
	std::ofstream file("Config/config.txt");
	YAML::Emitter out(file);
	out << YAML::BeginMap;
	out << YAML::Key << "Global Volume" << YAML::Value << geSound.globalVolume;
	out << YAML::Key << "Menu Volume" << YAML::Value << geSound.menuVolume;
	out << YAML::Key << "Music Volume" << YAML::Value << geSound.musicVolume;
	out << YAML::Key << "Message Index" << YAML::Value << messageIndex;
	out << YAML::EndMap;
}

void loadConfig() {

	YAML::Node config = YAML::LoadFile("Config/config.txt");

	if (config["Global Volume"])
		geSound.globalVolume = config["Global Volume"].as<float>();
	if (config["Menu Volume"])
		geSound.menuVolume = config["Menu Volume"].as<float>();
	if (config["Music Volume"])
		geSound.musicVolume = config["Music Volume"].as<float>();
	if (config["Message Index"])
		messageIndex = config["Message Index"].as<float>();
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

		if (!IsWindowMaximized())
			SetWindowSize(static_cast<float>(GetMonitorWidth(monitor)) * 0.5f, static_cast<float>(GetMonitorHeight(monitor)) * 0.5f);
		else
			SetWindowSize(static_cast<float>(GetMonitorWidth(monitor)) * 0.5f, static_cast<float>(GetMonitorHeight(monitor)) * 0.5f);
		ToggleBorderlessWindowed();
		wasFullscreen = IsWindowMaximized();;

		UnloadRenderTexture(myParticlesTexture);
		UnloadRenderTexture(myUITexture);

		lastScreenWidth = GetScreenWidth();
		lastScreenHeight = GetScreenHeight();
		lastScreenState = myVar.fullscreenState;

		myParticlesTexture = CreateFloatRenderTexture(lastScreenWidth, lastScreenHeight);;
		myUITexture = LoadRenderTexture(lastScreenWidth, lastScreenHeight);
	}

	int currentScreenWidth = GetScreenWidth();
	int currentScreenHeight = GetScreenHeight();

	if (currentScreenWidth != lastScreenWidth || currentScreenHeight != lastScreenHeight) {
		UnloadRenderTexture(myParticlesTexture);
		UnloadRenderTexture(myUITexture);

		myParticlesTexture = CreateFloatRenderTexture(currentScreenWidth, currentScreenHeight);
		myUITexture = LoadRenderTexture(currentScreenWidth, currentScreenHeight);

		lastScreenWidth = currentScreenWidth;
		lastScreenHeight = currentScreenHeight;
	}
}


RenderTexture2D CreateFloatRenderTexture(int w, int h) {
	RenderTexture2D fbo = { 0 };
	glGenTextures(1, &fbo.texture.id);
	glBindTexture(GL_TEXTURE_2D, fbo.texture.id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_HALF_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glGenFramebuffers(1, &fbo.id);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo.id);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo.texture.id, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	fbo.texture.width = w;
	fbo.texture.height = h;
	fbo.texture.format = PIXELFORMAT_UNCOMPRESSED_R16G16B16A16;

	return fbo;
}