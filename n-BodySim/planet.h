#pragma once
#include "raylib.h"


struct ParticlePhysics {
public:
	Vector2 pos;
	Vector2 velocity;
	double mass;

	ParticlePhysics(Vector2 pos, Vector2 velocity, double mass);
};

struct ParticleRendering {

	Color color;
	float size;
	bool enableBlur;
	bool customColor;
	bool drawPixel;

	ParticleRendering(Color color, float size, bool enableBlur, bool customColor, bool drawPixel);

};
