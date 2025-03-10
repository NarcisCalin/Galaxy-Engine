#pragma once
#include "raylib.h"
#include <vector>
#include <iostream>
#include "planet.h"

class Quadtree {
public:
	Vector2 pos;
	float size;
	Color color;
	std::vector<Quadtree> subGrids;
	std::vector<Planet> myPlanets;
	float gridMass;
	Vector2 centerOfMass;
	Quadtree* parent;
	int depth;

	Quadtree(float posX, float posY, float size,
		int r, int g, int b, int a,
		std::vector<Planet> planets,
		Quadtree* parent);

	void subGridMaker();

	void calculateMasses();

	void printGridInfo();

	void drawCenterOfMass() const;
};