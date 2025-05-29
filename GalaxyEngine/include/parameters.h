#pragma once
#include "raylib/raylib.h"
#include <vector>
#include "Particles/particle.h"
#include "UX/screenCapture.h"
#include "Physics/morton.h"
#include "Particles/particleTrails.h"
#include "Particles/particleSelection.h"
#include "UI/brush.h"
#include "Particles/particleSubdivision.h"
#include "Particles/densitySize.h"
#include "Particles/particleColorVisuals.h"
#include "UI/rightClickSettings.h"
#include "UI/controls.h"
#include "Particles/particlesSpawning.h"
#include "Particles/neighborSearch.h"
#include "../external/imgui/imgui.h"
#include "../external/imgui/rlImGui.h"
#include "../external/imgui/rlImGuiColors.h"


struct UpdateParameters {
	std::vector<ParticlePhysics> pParticles;
	std::vector<ParticleRendering> rParticles;

	std::vector<ParticlePhysics> pParticlesSelected;
	std::vector<ParticleRendering> rParticlesSelected;

	std::vector<ParticleTrails> trailDots;

	ScreenCapture screenCapture;

	Morton morton;

	ParticleTrails trails;

	ParticleSelection particleSelection;

	SceneCamera myCamera;

	Brush brush;

	UpdateParameters() : brush(myCamera, 25.0f) {}

	ParticleSubdivision subdivision;

	DensitySize densitySize;

	ColorVisuals colorVisuals;

	RightClickSettings rightClickSettings;

	Controls controls;

	ParticleDeletion particleDeletion;

	ParticlesSpawning particlesSpawning;

	NeighborSearch neighborSearch;
};

struct UpdateVariables{
	int screenWidth = 1920;
	int screenHeight = 1080;
	float halfScreenWidth = screenWidth * 0.5f;
	float halfScreenHeight = screenHeight * 0.5f;

	float screenRatioX = 0.0f;
	float screenRatioY = 0.0f;

	Vector2 domainSize = { 3840.0f, 2160.0f };

	float halfDomainWidth = domainSize.x * 0.5f;
	float halfDomainHeight = domainSize.y * 0.5f;

	bool fullscreenState = false;

	int targetFPS = 144;

	double G = 6.674e-11;
	float gravityMultiplier = 1.0f;
	float softening = 2.5f;
	float theta = 0.8f;
	float timeStepMultiplier = 1.0f;
	float sphMaxVel = 250.0f;

	const float particleBaseMass = 8500000000.0f;

	int maxLeafParticles = 1;
	float minLeafSize = 1.0f;

	int substeps = 1;

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
	bool isCollisionsEnabled = false;
	bool isDensitySizeEnabled = false;
	bool isForceSizeEnabled = false;
	bool isShipGasEnabled = true;
	bool isSPHEnabled = false;
	bool sphGround = false;

	bool isSpawningAllowed = true;

	float particleTextureHalfSize = 16.0f;

	int trailMaxLength = 48;
	ImVec4 buttonEnabledColor = { 0.2f,0.5f, 0.2f, 1.0f };
	ImVec4 buttonDisabledColor = { 0.3f,0.3f, 0.3f, 1.0f };
	ImVec4 buttonCancelColor = { 0.8f, 0.2f, 0.2f, 1.0f };

	bool isRecording = false;

	float particleSizeMultiplier = 1.0f;

	bool isDragging = false;
	bool isMouseNotHoveringUI = false;

	bool drawQuadtree = false;
	bool drawZCurves = false;

	bool isGlowEnabled = false;

	Vector2 mouseWorldPos = { 0.0f, 0.0f };

	float particleBounciness = 0.75f;

	int threadsAmount = 16;
	bool longExposure = false;
	ImFont* robotoMediumFont = nullptr;

	bool pauseAfterRecording = false;
	bool cleanSceneAfterRecording = false;
	float recordingTimeLimit = 0.0f; 
};