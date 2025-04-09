#pragma once
#include "raylib.h"
#include <vector>
#include "particle.h"
#include "screenCapture.h"
#include "morton.h"
#include "particleTrails.h"
#include "particleSelection.h"
#include "brush.h"
#include "particleSubdivision.h"
#include "densitySize.h"
#include "particleColorVisuals.h"
#include "rightClickSettings.h"
#include "controls.h"
#include "particlesSpawning.h"


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
};

struct UpdateVariables{
	int screenWidth = 1920;
	int screenHeight = 1024;
	float halfScreenWidth = screenWidth * 0.5f;
	float halfScreenHeight = screenHeight * 0.5f;

	float screenRatioX = 0.0f;
	float screenRatioY = 0.0f;

	int targetFPS = 144;

	double G = 6.674e-11;
	float gravityMultiplier = 1.0f;
	float softening = 1.8f;
	float theta = 0.8f;
	float timeStepMultiplierSlider = 1.0f;
	float timeStepMultiplier = 1.0f;

	const float fixedDeltaTime = 0.03f;

	bool isTimeStopped = false;

	float timeFactor = 1.0f;

	bool isGlobalTrailsEnabled = false;
	bool isSelectedTrailsEnabled = false;
	bool isLocalTrailsEnabled = false;
	bool isPixelDrawingEnabled = false;
	bool isPeriodicBoundaryEnabled = true;
	bool isMultiThreadingEnabled = true;
	bool isBarnesHutEnabled = true;
	bool isDarkMatterEnabled = false;
	bool isCollisionsEnabled = false;
	bool isDensitySizeEnabled = false;

	bool isSpawningAllowed = true;

	float particleTextureSize = 32.0f;

	int trailMaxLength = 14;

	bool isRecording = false;

	float particleSizeMultiplier = 1.0f;

	bool subdivideAll = false;
	bool subdivideSelected = false;

	bool isDragging = false;
	bool isMouseNotHoveringUI = false;

	bool drawQuadtree = false;
	bool drawZCurves = false;

	bool isGlowEnabled = false;

	Vector2 mouseWorldPos = { 0.0f };
};