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
	int screenHeight = 1080;
	float halfScreenWidth = screenWidth * 0.5f;
	float halfScreenHeight = screenHeight * 0.5f;

	float screenRatioX = 0.0f;
	float screenRatioY = 0.0f;

	Vector2 domainSize = { 3840.0f, 2160.0f };

	float halfDomainWidth = domainSize.x * 0.5f;
	float halfDomainHeight = domainSize.y * 0.5f;

	int targetFPS = 144;

	double G = 6.674e-11;
	float gravityMultiplier = 1.0f;
	float softening = 2.5f;
	float theta = 0.8f;
	float timeStepMultiplierSlider = 1.0f;
	float timeStepMultiplier = 1.0f;

	int substeps = 1;

	const float fixedDeltaTime = 0.045f;

	bool isTimeStopped = false;

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

	bool isSpawningAllowed = true;

	float particleTextureHalfSize = 16.0f;

	int trailMaxLength = 48;

	bool isRecording = false;

	float particleSizeMultiplier = 1.0f;

	bool subdivideAll = false;
	bool subdivideSelected = false;

	bool isDragging = false;
	bool isMouseNotHoveringUI = false;

	bool drawQuadtree = false;
	bool drawZCurves = false;

	bool isGlowEnabled = false;

	bool isInformationEnabled = false;

	Vector2 mouseWorldPos = { 0.0f, 0.0f };

	float particleBounciness = 0.15f;

	int threadsAmount = 16;
};