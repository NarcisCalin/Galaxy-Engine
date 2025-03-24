#include "planet.h"

ParticlePhysics::ParticlePhysics(Vector2 pos, Vector2 velocity, float mass) {
	this->pos = pos;
	this->velocity = velocity;
	this->mass = mass;
	mortonKey = 0;
}

ParticleRendering::ParticleRendering(Color color, float size, bool enableBlur, bool uniqueColor, bool drawPixel) {
	this->color = color;
	this->size = size;
	this->enableBlur = enableBlur;
	this->uniqueColor = uniqueColor;
	this->drawPixel = drawPixel;
}
