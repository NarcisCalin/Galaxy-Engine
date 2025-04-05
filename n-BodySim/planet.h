#pragma once
#include <cstdint>
#include "raylib.h"


struct ParticlePhysics {

	Vector2 pos;
	Vector2 velocity;
	float mass;
	uint32_t mortonKey;

	ParticlePhysics(Vector2 pos, Vector2 velocity, float mass) {
		this->pos = pos;
		this->velocity = velocity;
		this->mass = mass;
		mortonKey = 0;
	}
};

struct ParticleRendering {

	Color color;
	float size;
	bool uniqueColor;
	bool drawPixel;
	bool isSelected;
	bool isSolid;
	bool canBeSubdivided;
	bool canBeResized;
	float previousSize;
	ParticleRendering(Color color, float size, bool uniqueColor, bool drawPixel, bool isSelected, bool isSolid, bool canBeSubdivided, bool canBeResized) {
		this->color = color;
		this->size = size;
		this->uniqueColor = uniqueColor;
		this->drawPixel = drawPixel;
		this->isSelected = isSelected;
		this->isSolid = isSolid;
		this->canBeSubdivided = canBeSubdivided;
		this->canBeResized = canBeResized;
		this->previousSize = size;
	}

};
