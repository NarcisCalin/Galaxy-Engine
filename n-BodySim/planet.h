#pragma once
#include <cstdint>
#include "raylib.h"


struct ParticlePhysics {

	Vector2 pos;
	Vector2 velocity;
	float mass;
	uint32_t mortonKey;

	ParticlePhysics(Vector2 pos, Vector2 velocity, float mass);
};

struct ParticleRendering {

	Color color;
	float size;
	bool enableBlur;
	bool uniqueColor;
	bool drawPixel;

	ParticleRendering(Color color, float size, bool enableBlur, bool uniqueColor, bool drawPixel);

};
