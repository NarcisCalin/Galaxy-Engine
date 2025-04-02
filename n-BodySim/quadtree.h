#pragma once
#include "raylib.h"
#include <vector>
#include <iostream>
#include <memory>
#include "planet.h"

struct Quadtree {
	Vector2 pos;
	float size;
	int startIndex;
	int endIndex;
	float gridMass;
	Vector2 centerOfMass;
	Quadtree* parent;
	std::vector<std::unique_ptr<Quadtree>> subGrids;

	Quadtree(float posX, float posY, float size,
		int startIndex, int endIndex,
		const std::vector<ParticlePhysics>& pParticles, const std::vector<ParticleRendering>& rParticles,
		Quadtree* parent);

	void subGridMaker(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles);

	void calculateMasses(const std::vector<ParticlePhysics>& pParticles);

	static Vector2 boundingBoxPos;
	static float boundingBoxSize;

	static Quadtree* boundingBox(const std::vector<ParticlePhysics>& pParticles,
		const std::vector<ParticleRendering>& rParticles);

	void drawQuadtree();
};