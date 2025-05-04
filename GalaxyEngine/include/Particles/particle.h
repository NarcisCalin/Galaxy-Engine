#pragma once

#include <cstdint>
#include "../raylib/raylib.h"


struct ParticlePhysics {

	Vector2 pos;
	Vector2 predPos;

	Vector2 vel;
	Vector2 prevVel;
	Vector2 acc;

	float mass;

	// SPH Parameters
	float pressF;
	float dens;

	// SPH Input parameters
	float restPress;
	float stiff;
	float visc;
	float cohesion;

	uint32_t mortonKey;

	ParticlePhysics(Vector2 pos, Vector2 vel, float mass, float restPress, float stiff, float visc, float cohesion) {
		this->pos = pos;
		this->predPos = { 0.0f, 0.0f };
		this->vel = vel;
		this->prevVel = vel;
		this->acc = { 0.0f, 0.0f };
		this->mass = mass;

		// SPH Parameters
		this->pressF = 0.0f;
		this->dens = 0.0f;

		// SPH Input parameters
		this->restPress = restPress;
		this->stiff = stiff;
		this->visc = visc;
		this->cohesion = cohesion;


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
	bool isSPH;

	// Temporal states (these values can change over time)
	bool isSelected;
	bool isGrabbed;
	float previousSize;
	int neighbors;
	float totalRadius;
	float lifeSpan;

	ParticleRendering(Color color, float size, bool uniqueColor, bool isSelected,
		bool isSolid, bool canBeSubdivided, bool canBeResized, bool isDarkMatter, bool isSPH, float lifeSpan) {
		// Initial
		this->color = color;
		this->size = size;
		this->uniqueColor = uniqueColor;
		this->isSolid = isSolid;
		this->canBeSubdivided = canBeSubdivided;
		this->canBeResized = canBeResized;
		this->isDarkMatter = isDarkMatter;
		this->isSPH = isSPH;

		// Temp
		this->isSelected = isSelected;
		this->isGrabbed = false;
		this->previousSize = size;
		this->neighbors = 0;
		this->totalRadius = 0.0f;
		this->lifeSpan = lifeSpan;
	
	}

};
