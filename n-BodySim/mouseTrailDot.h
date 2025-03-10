#pragma once
#include "raylib.h"
#include <vector>
#include <iostream>
#include "planet.h"

class MouseTrailDot {
public:
	Vector2 pos;
	int size = 5;
	MouseTrailDot(float x, float y, int z);
};