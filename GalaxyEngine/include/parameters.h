#pragma once

#include "Particles/particle.h"
#include "Particles/particleSubdivision.h"
#include "Particles/densitySize.h"
#include "Particles/particleColorVisuals.h"
#include "Particles/particleTrails.h"
#include "Particles/particleSelection.h"
#include "Particles/particlesSpawning.h"
#include "Particles/neighborSearch.h"
#include "Particles/clusterMouseHelper.h"

#include "Physics/morton.h"

#include "UI/brush.h"
#include "UI/rightClickSettings.h"
#include "UI/controls.h"

#include "UX/screenCapture.h"

struct PlaybackParticle {
	glm::vec3 pos;
	float previousSize;
	float size;
	uint32_t id;
	Color color;
};

struct UpdateParameters {
	std::vector<ParticlePhysics> pParticles;
	std::vector<ParticleRendering> rParticles;

	std::vector<ParticlePhysics> pParticlesSelected;
	std::vector<ParticleRendering> rParticlesSelected;

	std::vector<ParticlePhysics3D> pParticles3D;
	std::vector<ParticleRendering3D> rParticles3D;

	std::vector<ParticlePhysics3D> pParticlesSelected3D;
	std::vector<ParticleRendering3D> rParticlesSelected3D;

	std::vector<ParticlePhysics3D> pParticles3DPlaybackResume;
	std::vector<ParticleRendering3D> rParticles3DPlaybackResume;

	std::vector<std::vector<PlaybackParticle>> playbackFrames;

	std::vector<ParticleTrails> trailDots;

	ScreenCapture screenCapture;

	Morton morton;

	ParticleTrails trails;

	ParticleSelection particleSelection;
	ParticleSelection3D particleSelection3D;

	SceneCamera myCamera;
	SceneCamera3D myCamera3D;

	Brush brush;
	Brush3D brush3D;

	ParticleSubdivision subdivision;

	DensitySize densitySize;

	ColorVisuals colorVisuals;

	RightClickSettings rightClickSettings;

	Controls controls;
	std::unordered_map<uint32_t, size_t> pParticleLookup;
	ParticleDeletion particleDeletion;

	ParticlesSpawning particlesSpawning;
	ParticlesSpawning3D particlesSpawning3D;

	NeighborSearch neighborSearch;
	NeighborSearch3D neighborSearch3D;

	NeighborSearchV2 neighborSearchV2;
	NeighborSearchV2AVX2 neighborSearchV2AVX2;
	NeighborSearch3DV2 neighborSearch3DV2;
	NeighborSearch3DV2AVX2 neighborSearch3DV2AVX2;
};

struct UpdateVariables {
	int screenWidth = 1920;
	int screenHeight = 1080;
	float halfScreenWidth = screenWidth * 0.5f;
	float halfScreenHeight = screenHeight * 0.5f;

	float screenRatioX = 0.0f;
	float screenRatioY = 0.0f;

	glm::vec2 domainSize = { 3840.0f, 2160.0f };

	glm::vec3 domainSize3D = { 1300.0f,  1300.0f,  1300.0f };

	float halfDomainWidth = domainSize.x * 0.5f;
	float halfDomainHeight = domainSize.y * 0.5f;

	float halfDomain3DWidth = domainSize3D.x * 0.5f;
	float halfDomain3DHeight = domainSize3D.y * 0.5f;
	float halfDomain3DDepth = domainSize3D.z * 0.5f;

	bool fullscreenState = false;

	bool exitGame = false;

	int targetFPS = 1000;

	float G = 6.674e-11;
	float gravityMultiplier = 1.0f;
	float softening = 1.5f;
	float theta = 0.8f;
	float timeStepMultiplier = 1.0f;
	float sphMaxVel = 250.0f;
	float globalHeatConductivity = 0.045f;
	float globalAmbientHeatRate = 1.0f;
	float ambientTemp = 274.0f;

	static float particleBaseMass;

	int maxLeafParticles = 1;
	float minLeafSize = 1.0f;

	const float fixedDeltaTime = 0.045f;

	bool isTimePlaying = true;

	float timeFactor = 1.0f;

	bool isGlobalTrailsEnabled = false;
	bool isSelectedTrailsEnabled = false;
	bool isLocalTrailsEnabled = false;
	bool isPeriodicBoundaryEnabled = true;
	bool isMultiThreadingEnabled = true;
	bool isBarnesHutEnabled = true;
	bool isDarkMatterEnabled = true;
	bool isDensitySizeEnabled = false;
	bool isForceSizeEnabled = false;
	bool isShipGasEnabled = true;
	bool isSPHEnabled = false;
	bool sphGround = false;
	bool isTempEnabled = false;
	bool constraintsEnabled = false;
	bool isOpticsEnabled = false;

	bool isGPUEnabled = false;

	bool isMergerEnabled = false;

	bool longExposureFlag = false;
	int longExposureDuration = 200;
	int longExposureCurrent = 0;

	bool isSpawningAllowed = true;

	float particleTextureHalfSize = 16.0f;

	int trailMaxLength = 350;

	static ImVec4 colWindowBg;

	//ImGui style colors
	static ImVec4 colButton;
	static ImVec4 colButtonHover;
	static ImVec4 colButtonPress;

	static ImVec4 colButtonActive;
	static ImVec4 colButtonActiveHover;
	static ImVec4 colButtonActivePress;

	static ImVec4 colButtonRedActive;
	static ImVec4 colButtonRedActiveHover;
	static ImVec4 colButtonRedActivePress;

	// ImGui slider colors
	static ImVec4 colSliderGrab;
	static ImVec4 colSliderGrabActive;
	static ImVec4 colSliderBg;
	static ImVec4 colSliderBgHover;
	static ImVec4 colSliderBgActive;

	// ImPlot style colors
	static ImVec4 colPlotLine;
	static ImVec4 colAxisText;
	static ImVec4 colAxisGrid;
	static ImVec4 colAxisBg;
	static ImVec4 colFrameBg;
	static ImVec4 colPlotBg;
	static ImVec4 colPlotBorder;
	static ImVec4 colLegendBg;

	// Text colors
	static ImVec4 colMenuInformation;

	bool isRecording = false;

	float particleSizeMultiplier = 0.6f;

	bool isDragging = false;
	bool isMouseNotHoveringUI = false;

	bool drawQuadtree = false;
	bool drawZCurves = false;

	bool isGlowEnabled = false;

	glm::vec2 mouseWorldPos = { 0.0f, 0.0f };

	int threadsAmount = 1;
	ImFont* robotoMediumFont = nullptr;

	bool pauseAfterRecording = false;
	bool cleanSceneAfterRecording = false;
	float recordingTimeLimit = 0.0f;

	float globalConstraintStiffnessMult = 1.0f;
	float globalConstraintResistance = 1.0f;

	bool constraintAllSolids = false;
	bool constraintSelected = false;
	bool deleteAllConstraints = false;
	bool deleteSelectedConstraints = false;
	bool drawConstraints = false;
	bool visualizeMesh = false;
	bool unbreakableConstraints = false;
	bool constraintStressColor = false;

	bool constraintAfterDrawingFlag = false;
	bool constraintAfterDrawing = false;

	float constraintMaxStressColor = 0.0f;

	bool pinFlag = false;
	bool unPinFlag = false;

	bool isBrushDrawing = false;

	Font customFont = { 0 };
	int introFontSize = 48;

	bool gridExists = true;
	bool grid3DExists = true;

	bool loadDropDownMenus = false;

	bool exportPlyFlag = false;
	bool exportPlySeqFlag = false;

	int plyFrameNumber = 0;

	bool toolSpawnHeavyParticle = false;
	bool toolDrawParticles = true;
	bool toolSpawnGalaxy = false;
	bool toolSpawnStar = false;
	bool toolSpawnBigBang = false;

	bool toolErase = false;
	bool toolRadialForce = false;
	bool toolSpin = false;
	bool toolMove = false;
	bool toolRaiseTemp = false;
	bool toolLowerTemp = false;

	bool toolPointLight = false;
	bool toolAreaLight = false;
	bool toolConeLight = false;
	bool toolCircle = false;
	bool toolDrawShape = false;
	bool toolLens = false;
	bool toolWall = false;
	bool toolMoveOptics = false;
	bool toolEraseOptics = false;
	bool toolSelectOptics = false;

	bool isGravityFieldEnabled = false;
	bool gravityFieldDMParticles = false;

	int frameCount = 0;

	bool naive = false;

	bool is3DMode = true;

	bool hasAVX2 = false;

	float heavyParticleWeightMultiplier = 1.0f;
	int predictPathLength = 1000;

	float particleAmountMultiplier = 1.0;
	float DMAmountMultiplier = 1.0f;

	float massScatter = 0.75f;

	bool enablePathPrediction = false;

	bool SPHWater = false;
	bool SPHRock = false;
	bool SPHIron = false;
	bool SPHSand = false;
	bool SPHSoil = false;
	bool SPHIce = false;
	bool SPHMud = false;
	bool SPHRubber = false;
	bool SPHGas = false;

	float mass = 0.03f;
	float stiffMultiplier = 1.0f;
	float viscosity = 0.3f;
	float cohesionCoefficient = 1.0f;
	float delta = 19000.0f;
	float verticalGravity = 3.0f;

	bool infiniteDomain = true;

	float brushSpinForceMult = 1.0f;
	float brushAttractForceMult = 1.0f;

	bool clipSelectedX = false;
	bool clipSelectedY = false;
	bool clipSelectedZ = false;

	bool clipSelectedXInv = false;
	bool clipSelectedYInv = false;
	bool clipSelectedZInv = false;


	bool playbackRecord = false;
	int frames = 100;
	int currentFrame = 0;

	int keyframeTickInterval = 5;

	float playbackProgress = 0.0f;
	float playbackSpeed = 0.2f;

	bool runPlayback = false;

	bool deletePlayback = false;

	bool isPlaybackOn = false;

	float playbackParticlesSizeMult = 1.0f;

	bool playBackOnMemory = false;

	bool firstPerson = false;

	std::string playbackPath = "playbackTemp/playback.bin";

	bool lowResRayMarching = false;

	bool isRayMarcherOn = false;

	bool flatParticleTexture3D = true;

	float boundaryFriction = 0.0f;
};