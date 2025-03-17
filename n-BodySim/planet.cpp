#include "planet.h"


ParticlePhysics::ParticlePhysics(Vector2 pos, Vector2 velocity, double mass) {
	this->pos = pos;
	this->velocity = velocity;
	this->mass = mass;
	mortonKey = 0;
}

ParticleRendering::ParticleRendering(Color color, float size, bool enableBlur, bool customColor, bool drawPixel) {
	this->color = color;
	this->size = size;
	this->enableBlur = enableBlur;
	this->customColor = customColor;
	this->drawPixel = drawPixel;
}
