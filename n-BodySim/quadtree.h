#pragma once
#include "raylib.h"
#include <vector>
#include <iostream>
#include "planet.h"

class Quadtree {
public:
	static constexpr int MAX_DEPTH = 30.0f;
	static constexpr float MIN_SIZE = 1.0f;

	Vector2 pos;
	float size;
	std::vector<Quadtree> subGrids;
	std::vector<Planet> myPlanets;
	float gridMass;
	Vector2 centerOfMass;
	Quadtree* parent;
	int depth;

	Quadtree(float posX, float posY, float size,
		const std::vector<Planet>& planets,
		Quadtree* parent);

	void subGridMaker();

	void calculateMasses();

	void printGridInfo();

	void drawCenterOfMass() const;
};