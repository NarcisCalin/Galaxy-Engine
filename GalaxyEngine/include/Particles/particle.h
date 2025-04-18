#pragma once

#include <cstdint>
#include "../raylib/raylib.h"


struct ParticlePhysics {

	Vector2 pos;
	Vector2 velocity;
	Vector2 acc;
	Vector2 prevAcc;
	float mass;
	uint32_t mortonKey;

	ParticlePhysics(Vector2 pos, Vector2 velocity, float mass) {
		this->pos = pos;
		this->velocity = velocity;
		this->acc = { 0.0f, 0.0f };
		this->prevAcc = { 0.0f, 0.0f };
		this->mass = mass;
		mortonKey = 0;
	}
};

struct ParticleRendering {

	Color color;
	float size;
	bool uniqueColor;
	bool isSelected;
	bool isSolid;
	bool canBeSubdivided;
	bool canBeResized;
	bool isDarkMatter;
	float previousSize;
	ParticleRendering(Color color, float size, bool uniqueColor, bool isSelected, 
		bool isSolid, bool canBeSubdivided, bool canBeResized, bool isDarkMatter) {
		this->color = color;
		this->size = size;
		this->uniqueColor = uniqueColor;
		this->isSelected = isSelected;
		this->isSolid = isSolid;
		this->canBeSubdivided = canBeSubdivided;
		this->canBeResized = canBeResized;
		this->isDarkMatter = isDarkMatter;
		this->previousSize = size;
	
	}

};
