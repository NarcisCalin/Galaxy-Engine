#pragma once

#include "Particles/particle.h"
#include "Particles/particleTrails.h"
#include "Particles/particleSelection.h"
#include "Particles/particleSubdivision.h"
#include "Particles/densitySize.h"
#include "Particles/particleColorVisuals.h"
#include "Particles/particleDeletion.h"
#include "Particles/particlesSpawning.h"
#include "Particles/particleSpaceship.h"

#include "Physics/quadtree.h"
#include "Physics/slingshot.h"
#include "Physics/morton.h"
#include "Physics/physics.h"
#include "Physics/SPH.h"
#include "Physics/light.h"

#include "UI/brush.h"
#include "UI/rightClickSettings.h"
#include "UI/controls.h"
#include "UI/UI.h"

#include "Sound/sound.h"

#include "UX/screenCapture.h"
#include "UX/camera.h"
#include "UX/saveSystem.h"
#include "UX/randNum.h"
#include "UX/copyPaste.h"

#include "parameters.h"

extern UpdateParameters myParam;
extern UpdateVariables myVar;
extern UI myUI;
extern Physics physics;
extern ParticleSpaceship ship;
extern SPH sph;
extern SaveSystem save;
extern GESound geSound;
extern Lighting lighting;
extern CopyPaste copyPaste;

Quadtree* gridFunction(std::vector<ParticlePhysics>& pParticles,
	std::vector<ParticleRendering>& rParticles);


void flattenQuadtree(Quadtree* node, std::vector<Quadtree*>& flatList);

struct ParticleBounds {
	float minX, maxX, minY, maxY;
};

// THIS FUNCTION IS MEAN FOR QUICK DEBUGGING WHERE YOU NEED TO CHECK A SPECIFIC PARTICLE'S VARIABLES
void selectedParticleDebug();

void pinParticles();

void updateScene();

void drawScene(Texture2D& particleBlurTex, RenderTexture2D& myRayTracingTexture,
	RenderTexture2D& myUITexture, RenderTexture2D& myMiscTexture, bool& fadeActive, bool& introActive);

void enableMultiThreading();

void fullscreenToggle(int& lastScreenWidth, int& lastScreenHeight,
	bool& wasFullscreen, bool& lastScreenState,
	RenderTexture2D& myParticlesTexture, RenderTexture2D& myUITexture);

void drawConstraints();


void saveConfigIfChanged();

void saveConfig();

void loadConfig();

RenderTexture2D CreateFloatRenderTexture(int w, int h);