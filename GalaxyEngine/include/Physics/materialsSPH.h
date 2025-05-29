#pragma once
#include "../raylib/raylib.h"
#include <string>

struct SPHWater {

	float massMult = 0.6f;

	float restDens = 0.095f;
	float stiff = 1.0f;
	float visc = 0.075f;
	float cohesion = 0.05f;

	Color color = { 30, 65, 230, 150 };

	std::string sphLabel = "water";
};

struct SPHRock {

	float massMult = 4.0f;

	float restDens = 0.008f;
	float stiff = 1.4f;
	float visc = 3.0f;
	float cohesion = 1750.0f;

	Color color = { 150, 155, 160, 255 };

	std::string sphLabel = "rock";
};

struct SPHSand {

	float massMult = 2.1f;

	float restDens = 0.008f;
	float stiff = 1.255f;
	float visc = 0.74f;
	float cohesion = 1.0f;

	Color color = { 200, 185, 100, 255 };

	std::string sphLabel = "sand";
};

struct SPHSoil {

	float massMult = 1.9f;

	float restDens = 0.008f;
	float stiff = 1.0f;
	float visc = 2.23f;
	float cohesion = 3000.0f;

	Color color = { 156, 110, 30, 255 };

	std::string sphLabel = "soil";
};

struct SPHIce {

	float massMult = 0.45f;

	float restDens = 0.012f;
	float stiff = 1.0f;
	float visc = 2.2f;
	float cohesion = 2500.0f;

	Color color = { 230, 230, 250, 240 };

	std::string sphLabel = "ice";
};

struct SPHMud {

	float massMult = 2.3f;

	float restDens = 0.0095f;
	float stiff = 1.0f;
	float visc = 5.6f;
	float cohesion = 1000.0f;

	Color color = { 106, 60, 3, 255 };

	std::string sphLabel = "mud";
};