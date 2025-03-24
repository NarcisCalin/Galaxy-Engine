#pragma once
#include "raylib.h"
#include <vector>
#include <iostream>
#include "planet.h"

class MouseTrailDot {
public:
	Vector2 pos;
	float size = 5;
	Color color;
	MouseTrailDot(Vector2 pos, float size, Color color);
};