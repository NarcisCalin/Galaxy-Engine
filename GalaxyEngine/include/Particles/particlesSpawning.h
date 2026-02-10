#pragma once

#include "Particles/particle.h"

#include "Physics/slingshot.h"

#include "Physics/constraint.h"

#include "UI/brush.h"

#include "UX/camera.h"

struct UpdateVariables;
struct UpdateParameters;

struct Physics;
struct Physics3D;
struct Quadtree;

class ParticlesSpawning {
public:

	const int correctionSubsteps = 32;
	bool particlesIterating = false;

	bool massMultiplierEnabled = true;

	float outerRadius = 200.0f;
	float scaleLength = 90.0f;

	float outerRadiusDM = 2000.0f;
	float radiusCoreDM = 3.5f;

	void particlesInitialConditions(Physics& physics, UpdateVariables& myVar, UpdateParameters& myParam);

	void predictTrajectory(const std::vector<ParticlePhysics>& actualParticles, 
		SceneCamera& myCamera, Physics physics, UpdateVariables& myVar, Slingshot& slingshot);

	void drawGalaxyDisplay(UpdateParameters& myParam);

private:

	float heavyParticleInitMass = 300000000000000.0f;
};

class ParticlesSpawning3D {
public:

	const int correctionSubsteps = 24;
	bool particlesIterating = false;

	bool massMultiplierEnabled = true;

	float diskAxisX = 90.0f;
	float diskAxisY = 0.0f;
	float diskAxisZ = 0.0f;

	float outerRadius = 120.0f;
	float radiusCore = 2.5f;

	float diskThickness = 0.5f;
	float bulgeThickness = 3.0f;
	float bulgeSize = 1200.0f;

	float outerRadiusDM = 2000.0f;
	float radiusCoreDM = 3.5f;

	void particlesInitialConditions(Physics3D& physics3D, UpdateVariables& myVar, UpdateParameters& myParam);

	void predictTrajectory(const std::vector<ParticlePhysics3D>& pParticles,
		SceneCamera3D& myCamera, Physics3D physics3D,
		UpdateVariables& myVar, UpdateParameters& myParam, Slingshot3D& slingshot);

	void drawGalaxyDisplay(UpdateParameters& myParam);

private:

	float heavyParticleInitMass = 300000000000000.0f;
};