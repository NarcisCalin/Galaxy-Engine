#pragma once

#include <cstdint>
#include <ostream>
#include <istream>
#include "../raylib/raylib.h"

// Streamable Vector2
inline std::ostream& operator<<(std::ostream& os, Vector2 const& v) {
	return os << v.x << ' ' << v.y;
}
inline std::istream& operator>>(std::istream& is, Vector2& v) {
	return is >> v.x >> v.y;
}

struct ParticlePhysics {
	Vector2 pos;
	Vector2 predPos;
	Vector2 vel;
	Vector2 predVel;
	Vector2 prevVel;
	Vector2 acc;
	float mass;

	// SPH Parameters
	float press;
	float pressTmp;
	float delta;
	Vector2 pressV;
	Vector2 force;
	float dens;
	float predDens;
	float sphMass;

	// SPH Input parameters
	float restDens;
	float stiff;
	float visc;
	float cohesion;
	uint32_t mortonKey;

	// Default constructor
	ParticlePhysics()
		: pos{ 0,0 }, predPos{ 0,0 }, vel{ 0,0 }, prevVel{ 0,0 }, acc{ 0,0 },
		mass(1.0f), press(0.0f), dens(0.0f), sphMass(0.0f),
		restDens(0.0f), stiff(0.0f), visc(0.0f), cohesion(0.0f),
		mortonKey(0)
	{
	}

	// Parameterized constructor
	ParticlePhysics(Vector2 pos, Vector2 vel, float mass, float restDens, float stiff, float visc, float cohesion) {
		this->pos = pos;
		this->predPos = { 0.0f, 0.0f };
		this->vel = vel;
		this->predVel = { 0.0f, 0.0f };
		this->prevVel = vel;
		this->acc = { 0.0f, 0.0f };
		this->mass = mass;

		// SPH Parameters
		this->press = 0.0f;
		this->pressTmp = 0.0f;
		this->delta = 0.0f;
		this->pressV = { 0.0f, 0.0f };
		this->force = { 0.0f, 0.0f };
		this->dens = 0.0f;
		this->predDens = 0.0f;

		this->sphMass = mass / 8500000000.0f; // I divide by the base standard mass

		// SPH Input parameters
		this->restDens = restDens;
		this->stiff = stiff;
		this->visc = visc;
		this->cohesion = cohesion;


		mortonKey = 0;
	}
};

// ParticlePhysics stream operators
inline std::ostream& operator<<(std::ostream& os, ParticlePhysics const& p) {
	return os
		<< p.pos << ' '
		<< p.predPos << ' '
		<< p.vel << ' '
		<< p.prevVel << ' '
		<< p.acc << ' '
		<< p.mass << ' '
		<< p.press << ' '
		<< p.dens << ' '
		<< p.sphMass << ' '
		<< p.restDens << ' '
		<< p.stiff << ' '
		<< p.visc << ' '
		<< p.cohesion << ' '
		<< p.mortonKey;
}

inline std::istream& operator>>(std::istream& is, ParticlePhysics& p) {
	return is
		>> p.pos.x >> p.pos.y
		>> p.predPos.x >> p.predPos.y
		>> p.vel.x >> p.vel.y
		>> p.prevVel.x >> p.prevVel.y
		>> p.acc.x >> p.acc.y
		>> p.mass
		>> p.press
		>> p.dens
		>> p.sphMass
		>> p.restDens
		>> p.stiff
		>> p.visc
		>> p.cohesion
		>> p.mortonKey;
}

// RGBA multiplier
struct RGBAMultiplier {
	float r;
	float g;
	float b;
	float a;
};
inline std::ostream& operator<<(std::ostream& os, RGBAMultiplier const& m) {
	return os << m.r << ' ' << m.g << ' ' << m.b << ' ' << m.a;
}
inline std::istream& operator>>(std::istream& is, RGBAMultiplier& m) {
	return is >> m.r >> m.g >> m.b >> m.a;
}

// Streamable Color
inline std::ostream& operator<<(std::ostream& os, Color const& c) {
	return os
		<< int(c.r) << ' '
		<< int(c.g) << ' '
		<< int(c.b) << ' '
		<< int(c.a);
}
inline std::istream& operator>>(std::istream& is, Color& c) {
	int r, g, b, a;
	is >> r >> g >> b >> a;
	c = { static_cast<unsigned char>(r),
		  static_cast<unsigned char>(g),
		  static_cast<unsigned char>(b),
		  static_cast<unsigned char>(a) };
	return is;
}

struct ParticleRendering {
	Color color;
	Color originalColor;
	float size;
	bool uniqueColor;
	bool isSolid;
	bool canBeSubdivided;
	bool canBeResized;
	bool isDarkMatter;
	bool isSPH;
	bool isSelected;
	bool isGrabbed;
	float previousSize;
	int neighbors;
	float totalRadius;
	float lifeSpan;
	RGBAMultiplier PRGBA;
	RGBAMultiplier SRGBA;

	// Default constructor
	ParticleRendering()
		: color{ 255,255,255,255 }, originalColor{ 255,255,255,255 }, size(1.0f),
		uniqueColor(false), isSolid(false), canBeSubdivided(false),
		canBeResized(false), isDarkMatter(false), isSPH(false),
		isSelected(false), isGrabbed(false), previousSize(1.0f),
		neighbors(0), totalRadius(0.0f), lifeSpan(0.0f),
		PRGBA{ 1,1,1,1 }, SRGBA{ 1,1,1,1 }
	{
	}

	// Parameterized constructor
	ParticleRendering(Color color, float size, bool uniqueColor, bool isSelected,
		bool isSolid, bool canBeSubdivided, bool canBeResized, bool isDarkMatter, bool isSPH, float lifeSpan) {
		// Initial states
		this->color = color;
		this->originalColor = color;
		this->size = size;
		this->uniqueColor = uniqueColor;
		this->isSolid = isSolid;
		this->canBeSubdivided = canBeSubdivided;
		this->canBeResized = canBeResized;
		this->isDarkMatter = isDarkMatter;
		this->isSPH = isSPH;

		// Temporal states
		this->isSelected = isSelected;
		this->isGrabbed = false;
		this->previousSize = size;
		this->neighbors = 0;
		this->totalRadius = 0.0f;
		this->lifeSpan = lifeSpan;

		this->PRGBA = { 1.0f, 1.0f, 1.0f, 1.0f };
		this->SRGBA = { 1.0f, 1.0f, 1.0f, 1.0f };

	}
};

// ParticleRendering stream operators
inline std::ostream& operator<<(std::ostream& os, ParticleRendering const& r) {
	return os
		<< r.color << ' '
		<< r.originalColor << ' '
		<< r.size << ' '
		<< int(r.uniqueColor) << ' '
		<< int(r.isSolid) << ' '
		<< int(r.canBeSubdivided) << ' '
		<< int(r.canBeResized) << ' '
		<< int(r.isDarkMatter) << ' '
		<< int(r.isSPH) << ' '
		<< int(r.isSelected) << ' '
		<< int(r.isGrabbed) << ' '
		<< r.previousSize << ' '
		<< r.neighbors << ' '
		<< r.totalRadius << ' '
		<< r.lifeSpan << ' '
		<< r.PRGBA << ' '
		<< r.SRGBA;
}

inline std::istream& operator>>(std::istream& is, ParticleRendering& r) {
	int cr, cg, cb, ca;
	is >> cr >> cg >> cb >> ca;
	r.color = { static_cast<unsigned char>(cr),
			   static_cast<unsigned char>(cg),
			   static_cast<unsigned char>(cb),
			   static_cast<unsigned char>(ca) };
	is >> r.originalColor;
	is >> r.size;
	int uniq, solid, subdiv, resize, dark, sph, selected, grabbed;
	is >> uniq >> solid >> subdiv >> resize >> dark >> sph >> selected >> grabbed;
	is >> r.previousSize >> r.neighbors >> r.totalRadius >> r.lifeSpan;
	is >> r.PRGBA >> r.SRGBA;

	r.uniqueColor = static_cast<bool>(uniq);
	r.isSolid = static_cast<bool>(solid);
	r.canBeSubdivided = static_cast<bool>(subdiv);
	r.canBeResized = static_cast<bool>(resize);
	r.isDarkMatter = static_cast<bool>(dark);
	r.isSPH = static_cast<bool>(sph);
	r.isSelected = static_cast<bool>(selected);
	r.isGrabbed = static_cast<bool>(grabbed);
	return is;
}
