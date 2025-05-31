#pragma once

// Streamable Vec2
inline std::ostream& operator<<(std::ostream& os, glm::vec2 const& v) {
	return os << v.x << ' ' << v.y;
}
inline std::istream& operator>>(std::istream& is, glm::vec2& v) {
	return is >> v.x >> v.y;
}

struct ParticlePhysics {
	glm::vec2 pos;
	glm::vec2 predPos;
	glm::vec2 vel;
	glm::vec2 predVel;
	glm::vec2 acc;
	float mass;

	// SPH Parameters
	float press;
	float pressTmp;
	glm::vec2 pressF;
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
		: pos(0.0f, 0.0f) , predPos{ 0,0 }, vel{ 0,0 }, predVel{0.0f, 0.0f}, acc{ 0,0 },
		mass(1.0f), press(0.0f), pressTmp(0.0f), pressF{ 0.0f,0.0f }, dens(0.0f), predDens(0.0f), sphMass(0.0f),
		restDens(0.0f), stiff(0.0f), visc(0.0f), cohesion(0.0f),
		mortonKey(0)
	{}

	// Parameterized constructor
	ParticlePhysics(glm::vec2 pos, glm::vec2 vel, float mass, float restDens, float stiff, float visc, float cohesion) {
		this->pos = pos;
		this->predPos = { 0.0f, 0.0f };
		this->vel = vel;
		this->predVel = { 0.0f, 0.0f };
		this->acc = { 0.0f, 0.0f };
		this->mass = mass;

		// SPH Parameters
		this->press = 0.0f;
		this->pressTmp = 0.0f;
		this->pressF = { 0.0f, 0.0f };
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
		<< p.predVel << ' '
		<< p.acc << ' '
		<< p.mass << ' '
		<< p.press << ' '
		<< p.pressTmp << ' '
		<< p.pressF << ' '
		<< p.dens << ' '
		<< p.predDens << ' '
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
		>> p.predVel.x >> p.predVel.y
		>> p.acc.x >> p.acc.y
		>> p.mass
		>> p.press
		>> p.pressTmp
		>> p.pressF.x >> p.pressF.y
		>> p.dens
		>> p.predDens
		>> p.sphMass
		>> p.restDens
		>> p.stiff
		>> p.visc
		>> p.cohesion
		>> p.mortonKey;
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
	Color pColor;
	Color sColor;
	Color sphColor;
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
	std::string sphLabel;

	// Default constructor
	ParticleRendering()
		: color{ 255,255,255,255 }, pColor{ 255,255,255,255 }, sColor{ 255, 255, 255, 255 }, sphColor{ 128,128,128,128 },
		size(1.0f),
		uniqueColor(false), isSolid(false), canBeSubdivided(false),
		canBeResized(false), isDarkMatter(false), isSPH(false),
		isSelected(false), isGrabbed(false), previousSize(1.0f),
		neighbors(0), totalRadius(0.0f), lifeSpan(0.0f), sphLabel("nonSPH")
	{}

	// Parameterized constructor
	ParticleRendering(Color color, float size, bool uniqueColor, bool isSelected,
		bool isSolid, bool canBeSubdivided, bool canBeResized, bool isDarkMatter, bool isSPH, float lifeSpan, std::string sphLabel) {
		// Initial states
		this->color = color;
		this->pColor = { 255, 255, 255, 255 };
		this->sColor = { 255, 255, 255, 255 };
		this->sphColor = { 128, 128, 128, 128 };
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
		this->sphLabel = sphLabel;
	}
};

// ParticleRendering stream operators
inline std::ostream& operator<<(std::ostream& os, ParticleRendering const& r) {
	return os
		<< r.color << ' '
		<< r.pColor << ' '
		<< r.sColor << ' '
		<< r.sphColor << ' '
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
		<< r.sphLabel;
}

inline std::istream& operator>>(std::istream& is, ParticleRendering& r) {
	int cr, cg, cb, ca;
	int pcr, pcg, pcb, pca;
	int scr, scg, scb, sca;
	int sphcr, sphcg, sphcb, sphca;
	is >> cr >> cg >> cb >> ca;
	r.color = { static_cast<unsigned char>(cr),
			   static_cast<unsigned char>(cg),
			   static_cast<unsigned char>(cb),
			   static_cast<unsigned char>(ca) };
	is >> pcr >> pcg >> pcb >> pca;
	r.pColor = { static_cast<unsigned char>(pcr),
			   static_cast<unsigned char>(pcg),
			   static_cast<unsigned char>(pcb),
			   static_cast<unsigned char>(pca) };
	is >> scr >> scg >> scb >> sca;
	r.sColor = { static_cast<unsigned char>(scr),
			   static_cast<unsigned char>(scg),
			   static_cast<unsigned char>(scb),
			   static_cast<unsigned char>(sca) };
	is >> sphcr >> sphcg >> sphcb >> sphca;
	r.sphColor = { static_cast<unsigned char>(sphcr),
			   static_cast<unsigned char>(sphcg),
			   static_cast<unsigned char>(sphcb),
			   static_cast<unsigned char>(sphca) };
	is >> r.size;
	int uniq, solid, subdiv, resize, dark, sph, selected, grabbed;
	is >> uniq >> solid >> subdiv >> resize >> dark >> sph >> selected >> grabbed;
	is >> r.previousSize >> r.neighbors >> r.totalRadius >> r.lifeSpan;

	r.uniqueColor = static_cast<bool>(uniq);
	r.isSolid = static_cast<bool>(solid);
	r.canBeSubdivided = static_cast<bool>(subdiv);
	r.canBeResized = static_cast<bool>(resize);
	r.isDarkMatter = static_cast<bool>(dark);
	r.isSPH = static_cast<bool>(sph);
	r.isSelected = static_cast<bool>(selected);
	r.isGrabbed = static_cast<bool>(grabbed);
	is >> std::ws;
	std::getline(is, r.sphLabel);
	return is;
}
