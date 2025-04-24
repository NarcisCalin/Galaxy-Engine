#pragma once

#include <cstdint>
#include "../raylib/raylib.h"


struct ParticlePhysics {

	Vector2 pos;
	Vector2 vel;
	Vector2 prevVel;
	Vector2 acc;
	float mass;
	uint32_t mortonKey;

	ParticlePhysics(Vector2 pos, Vector2 vel, float mass) {
		this->pos = pos;
		this->vel = vel;
		this->prevVel = vel;
		this->acc = { 0.0f, 0.0f };
		this->mass = mass;
		mortonKey = 0;
	}
};

struct ParticleRendering {

	// Initial states (these values will likely not change over time)
	Color color;
	float size;
	bool uniqueColor;
	bool isSolid;
	bool canBeSubdivided;
	bool canBeResized;
	bool isDarkMatter;

	// Temporal states (these values can change over time)
	bool isSelected;
	bool isGrabbed;
	float previousSize;

	ParticleRendering(Color color, float size, bool uniqueColor, bool isSelected,
		bool isSolid, bool canBeSubdivided, bool canBeResized, bool isDarkMatter) {
		// Initial
		this->color = color;
		this->size = size;
		this->uniqueColor = uniqueColor;
		this->isSolid = isSolid;
		this->canBeSubdivided = canBeSubdivided;
		this->canBeResized = canBeResized;
		this->isDarkMatter = isDarkMatter;

		// Temp
		this->isSelected = isSelected;
		this->isGrabbed = false;
		this->previousSize = size;
	
	}

};
