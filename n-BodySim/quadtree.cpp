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

	if ((endIndex - startIndex) <= 1 || size <= 1.0f) {
		computeLeafMass(pParticles);
	}
	else {
		subGridMaker(const_cast<std::vector<ParticlePhysics>&>(pParticles), const_cast<std::vector<ParticleRendering>&>(rParticles));
		computeInternalMass();
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

Vector2 Quadtree::boundingBoxPos = { 0.0f, 0.0f };
float Quadtree::boundingBoxSize = 0.0f;
Quadtree* Quadtree::boundingBox(const std::vector<ParticlePhysics>& pParticles,
	const std::vector<ParticleRendering>& rParticles) {


	float min_x = std::numeric_limits<float>::max();
	float min_y = std::numeric_limits<float>::max();
	float max_x = std::numeric_limits<float>::lowest();
	float max_y = std::numeric_limits<float>::lowest();

	for (const auto& particle : pParticles) {
		min_x = std::min(min_x, particle.pos.x);
		min_y = std::min(min_y, particle.pos.y);
		max_x = std::max(max_x, particle.pos.x);
		max_y = std::max(max_y, particle.pos.y);
	}

	boundingBoxSize = std::max(max_x - min_x, max_y - min_y);

	float centerX = (min_x + max_x) * 0.5f;
	float centerY = (min_y + max_y) * 0.5f;

	boundingBoxPos.x = centerX - boundingBoxSize / 2.0f;
	boundingBoxPos.y = centerY - boundingBoxSize / 2.0f;

	//DrawRectangleLines(boundingBoxPos.x, boundingBoxPos.y, boundingBoxSize, boundingBoxSize, WHITE);
	return new Quadtree(boundingBoxPos.x, boundingBoxPos.y, boundingBoxSize, 0, static_cast<int>(pParticles.size()),
		pParticles, rParticles, nullptr);
}

void Quadtree::drawQuadtree() {
	DrawRectangleLines(pos.x, pos.y, size, size, WHITE);

	if (gridMass > 0) {
		DrawCircle(centerOfMass.x, centerOfMass.y, 2.0f, YELLOW);
	}

	for (auto& child : subGrids) {
		child->drawQuadtree();
	}
}