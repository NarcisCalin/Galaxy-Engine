#include "raylib.h"
#include <vector>
#include <iostream>
#include "planet.h"


Planet::Planet(float x, float y, float z, float vx, float vy, double w, int r, int g, int b, int a, float accX, float accY, bool customColor, bool drawPixel) {
	pos.x = x;
	pos.y = y;
	size = z;
	velocity.x = vx;
	velocity.y = vy;
	mass = w;
	color.r = r;
	color.g = g;
	color.b = b;
	color.a = a;
	acceleration.x = accX;
	acceleration.y = accY;
	prevAcceleration = acceleration;
	this->customColor = customColor;
	this->drawPixel = drawPixel;

}
