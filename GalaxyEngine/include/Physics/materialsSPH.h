#pragma once
#include "../raylib/raylib.h"

struct SPHWater {

	float massMult = 0.6f;

	float restPress = 0.5f;
	float stiff = 0.1f;
	float visc = 0.095f;
	float cohesion = 1.0f;

	Color color = { 30, 65, 230, 150 };
};

struct SPHRock {

	float massMult = 4.0f;

	float restPress = 1.0f;
	float stiff = 2.1f;
	float visc = 3.0f;
	float cohesion = 1750.0f;

	Color color = { 150, 155, 160, 255 };
};

struct SPHSand {

	float massMult = 2.1f;

	float restPress = 1.0f;
	float stiff = 1.255f;
	float visc = 0.74f;
	float cohesion = 1.0f;

	Color color = { 200, 185, 100, 255 };
};

struct SPHMud {

	float massMult = 2.5f;

	float restPress = 1.0f;
	float stiff = 1.0f;
	float visc = 2.23f;
	float cohesion = 3000.0f;

	Color color = { 156, 110, 30, 255 };
};

struct SPHAir {

	float massMult = 0.45f;

	float restPress = 0.20f;
	float stiff = 0.01f;
	float visc = 0.07f;
	float cohesion = 1.0f;

	Color color = { 128, 128, 128, 128 };
};