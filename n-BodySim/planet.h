#pragma once
#include "raylib.h"
#include <vector>
#include <iostream>


class Planet {
public:
	Vector2 pos;
	float size;
	Vector2 velocity;
	double mass;
	Color color;
	bool enableBlur = true;
	Vector2 acceleration;
	Vector2 prevAcceleration;
	bool customColor;
	bool drawPixel;




	Planet(float x, float y, float z, float vx, float vy, double w, int r, int g, int b, int a, float accX, float accY, bool customColor, bool drawPixel);

};
