#include "raylib.h"
#include <vector>
#include <iostream>
#include <algorithm>
#include "quadtree.h"
#include "planet.h"
#include "omp.h"




Quadtree::Quadtree(float posX, float posY, float size,
	int startIndex, int endIndex,
	const std::vector<ParticlePhysics>& pParticles, const std::vector<ParticleRendering>& rParticles,
	Quadtree* parent = nullptr) {

	this->pos.x = posX;
	this->pos.y = posY;
	this->size = size;
	this->startIndex = startIndex;
	this->endIndex = endIndex;
	this->gridMass = 0;
	this->centerOfMass = { 0,0 };
	this->parent = parent;

	if ((endIndex - startIndex) >= 1 && size > 1.0f) {
		subGridMaker(const_cast<std::vector<ParticlePhysics>&>(pParticles), const_cast<std::vector<ParticleRendering>&>(rParticles));
	}
}

void Quadtree::subGridMaker(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles) {
	float midX = pos.x + size / 2.0f;
	float midY = pos.y + size / 2.0f;

	int current = startIndex;
	for (int q = 0; q < 4; ++q) {
		int qStart = current;
		for (int i = current; i < endIndex; ++i) {
			int quadrant = 0;
			if (pParticles[i].pos.x >= midX) quadrant += 1;
			if (pParticles[i].pos.y >= midY) quadrant += 2;

			if (quadrant == q) {
				std::swap(pParticles[i], pParticles[current]);
				std::swap(rParticles[i], rParticles[current]);
				current++;
			}
		}
		int qEnd = current;

		if (qEnd > qStart) {
			switch (q) {
			case 0:
				subGrids.push_back(std::make_unique<Quadtree>(pos.x, pos.y, size / 2, qStart, qEnd, pParticles, rParticles, this));
				break;
			case 1:
				subGrids.push_back(std::make_unique<Quadtree>(pos.x + size / 2, pos.y, size / 2, qStart, qEnd, pParticles, rParticles, this));
				break;
			case 2:
				subGrids.push_back(std::make_unique<Quadtree>(pos.x, pos.y + size / 2, size / 2, qStart, qEnd, pParticles, rParticles, this));
				break;
			case 3:
				subGrids.push_back(std::make_unique<Quadtree>(pos.x + size / 2, pos.y + size / 2, size / 2, qStart, qEnd, pParticles, rParticles, this));
				break;
			}
		}
	}
}

void Quadtree::calculateMasses(const std::vector<ParticlePhysics>& pParticles) {
	for (const auto& child : subGrids) {
		child->calculateMasses(pParticles);
	}
	if (subGrids.empty()) {
		if ((endIndex - startIndex) == 1) {
			gridMass = pParticles[startIndex].mass;
			centerOfMass = pParticles[startIndex].pos;
		}
		else {
			gridMass = 0;
			centerOfMass = { 0, 0 };
			for (int i = startIndex; i < endIndex; i++) {
				gridMass += pParticles[i].mass;
				centerOfMass.x += pParticles[i].pos.x * pParticles[i].mass;
				centerOfMass.y += pParticles[i].pos.y * pParticles[i].mass;
			}
			if (gridMass > 0) {
				centerOfMass.x /= gridMass;
				centerOfMass.y /= gridMass;
			}
		}
	}
	else {
		gridMass = 0;
		centerOfMass = { 0, 0 };
		for (const auto& child : subGrids) {
			gridMass += child->gridMass;
			centerOfMass.x += child->centerOfMass.x * child->gridMass;
			centerOfMass.y += child->centerOfMass.y * child->gridMass;
		}
		if (gridMass > 0) {
			centerOfMass.x /= gridMass;
			centerOfMass.y /= gridMass;
		}
	}
}
