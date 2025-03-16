#include "raylib.h"
#include <vector>
#include <iostream>
#include "quadtree.h"
#include "planet.h"
#include "omp.h"




Quadtree::Quadtree(float posX, float posY, float size,
	const std::vector<Planet>& planets,
	Quadtree* parent = nullptr) {

	this->pos.x = posX;
	this->pos.y = posY;
	this->size = size;
	this->gridMass = 0;
	this->centerOfMass = { 0,0 };
	this->parent = parent;
	depth = (parent == nullptr) ? 0 : parent->depth + 1; // Set depth here

	for (auto& planet : planets) {
		if (planet.pos.x >= pos.x && planet.pos.x < pos.x + size &&
			planet.pos.y >= pos.y && planet.pos.y < pos.y + size) {
			myPlanets.push_back(planet);
		}
	}

	if (myPlanets.size() > 1) {
		subGridMaker();
	}
}
void Quadtree::subGridMaker() {

	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 2; j++) {
			subGrids.emplace_back(
				this->pos.x + i * (this->size / 2),
				this->pos.y + j * (this->size / 2),
				this->size / 2,
				this->myPlanets,
				this

			);
		}
	}

	//Draw grids
	/*for (auto& subGrid : subGrids) {
		if (size < 10) {
			DrawRectangleLines(this->pos.x, subGrid.pos.y, subGrid.size, subGrid.size, RED);
		}
		else {
			DrawRectangleLines(subGrid.pos.x, subGrid.pos.y, subGrid.size, subGrid.size, WHITE);
		}
	}*/
}

void Quadtree::calculateMasses() {

	for (auto& subGrid : subGrids) {
		subGrid.calculateMasses();
	}

	if (myPlanets.size() == 1 && subGrids.empty()) {
		gridMass = myPlanets[0].mass;
		centerOfMass = myPlanets[0].pos;
	}
	else if (!subGrids.empty()) {
		gridMass = 0;
		float totalMass = 0;
		centerOfMass = { 0,0 };
		for (auto& subGrid : subGrids) {
			if (subGrid.gridMass > 0) {
				gridMass += subGrid.gridMass;
				centerOfMass.x += subGrid.centerOfMass.x * subGrid.gridMass;
				centerOfMass.y += subGrid.centerOfMass.y * subGrid.gridMass;
				totalMass += subGrid.gridMass;
			}
		}

		if (totalMass > 0) {
			centerOfMass.x /= totalMass;
			centerOfMass.y /= totalMass;
		}
	}
	else {
		gridMass = 0;
		centerOfMass = { 0,0 };
	}
}

void Quadtree::printGridInfo() {
	std::cout << "Grid at (" << pos.x << ", " << pos.y << ") with size " << size << ":\n";
	std::cout << "  Mass: " << gridMass << "\n";
	std::cout << "  Center of Mass: (" << centerOfMass.x << ", " << centerOfMass.y << ")\n";
	std::cout << "  Number of planets: " << myPlanets.size() << "\n";
	std::cout << "  Number of subgrids: " << subGrids.size() << "\n";
}

void Quadtree::drawCenterOfMass() const{

		/*if (gridMass > 0 && (myPlanets.size() > 1 || subGrids.size() > 0)) {

			float circleSize = std::max(4.0f - (depth * 0.5f), 1.0f);


			Color depthColor = { 255,20 + (depth * 30),20,255 };

			DrawCircle(centerOfMass.x, centerOfMass.y, circleSize, depthColor);
		}


		for (const auto& subGrid : subGrids) {
			subGrid.drawCenterOfMass();
		}*/
}