#pragma once

extern uint32_t globalId;

struct ParticlePhysics {

	glm::vec2 pos;
	glm::vec2 vel;
	glm::vec2 acc;
	glm::vec2 predPos;
	glm::vec2 prevVel;
	glm::vec2 predVel;
	glm::vec2 pressF;

	float mass;
	float press;
	float pressTmp;
	float dens;
	float predDens;
	float sphMass;
	float restDens;
	float stiff;
	float visc;
	float cohesion;
	float temp;
	float ke;
	float prevKe;
	uint32_t mortonKey;
	uint32_t id;
	uint32_t neighborOffset;

	bool isHotPoint;
	bool hasSolidified;

	// Default constructor
	ParticlePhysics()
		: pos(0.0f, 0.0f), vel{ 0,0 }, acc{ 0,0 }, predPos{ 0,0 }, prevVel{ 0.0f, 0.0f }, predVel{ 0.0f, 0.0f }, pressF{ 0.0f,0.0f },
		mass(8500000000.0f), press(0.0f), pressTmp(0.0f), dens(0.0f), predDens(0.0f), sphMass(1.0f),
		restDens(0.0f), stiff(0.0f), visc(0.0f), cohesion(0.0f),
		temp(0.0f), ke(0.0f), prevKe(0.0f), mortonKey(0), id(globalId++), neighborOffset(0),
		isHotPoint(false), hasSolidified(false)
	{
	}

	// Parameterized constructor
	ParticlePhysics(glm::vec2 pos, glm::vec2 vel, float mass, float restDens, float stiff, float visc, float cohesion) {
		this->pos = pos;
		this->vel = vel;
		this->acc = { 0.0f, 0.0f };
		this->predPos = { 0.0f, 0.0f };
		this->prevVel = { 0.0f, 0.0f };
		this->predVel = { 0.0f, 0.0f };
		this->pressF = { 0.0f, 0.0f };
		this->mass = mass;
		this->press = 0.0f;
		this->pressTmp = 0.0f;
		this->dens = 0.0f;
		this->predDens = 0.0f;
		this->sphMass = mass / 8500000000.0f;
		this->restDens = restDens;
		this->stiff = stiff;
		this->visc = visc;
		this->cohesion = cohesion;
		this->temp = 288.0f;
		this->ke = 0.0f;
		this->prevKe = 0.0f;
		this->mortonKey = 0;
		this->id = globalId++;
		this->neighborOffset = 0;
		this->isHotPoint = false;
		this->hasSolidified = false;
	}
};

struct ParticleRendering {

	Color color;
	Color pColor;
	Color sColor;
	Color sphColor;

	float size;
	float previousSize;
	float totalRadius;
	float lifeSpan;
	float turbulence;
	uint32_t sphLabel;

	int neighbors;
	int spawnCorrectIter;

	bool uniqueColor;
	bool isSolid;
	bool canBeSubdivided;
	bool canBeResized;
	bool isDarkMatter;
	bool isSPH;
	bool isSelected;
	bool isGrabbed;
	bool isPinned;
	bool isBeingDrawn;

	// Default constructor
	ParticleRendering()
		: color{ 255,255,255,255 }, pColor{ 255,255,255,255 }, sColor{ 255, 255, 255, 255 }, sphColor{ 128,128,128,128 },
		size(1.0f), previousSize(1.0f), totalRadius(0.0f), lifeSpan(-1.0f), turbulence(0.0f), sphLabel(0),
		neighbors(0), spawnCorrectIter(100000000),
		uniqueColor(false), isSolid(false), canBeSubdivided(false),
		canBeResized(false), isDarkMatter(false), isSPH(false),
		isSelected(false), isGrabbed(false), isPinned(false), isBeingDrawn(true)
	{
	}

	// Parameterized constructor
	ParticleRendering(Color color, float size, bool uniqueColor, bool isSelected,
		bool isSolid, bool canBeSubdivided, bool canBeResized, bool isDarkMatter, bool isSPH, float lifeSpan, uint32_t sphLabel) {
		this->color = color;
		this->pColor = { 255, 255, 255, 255 };
		this->sColor = { 255, 255, 255, 255 };
		this->sphColor = { 128, 128, 128, 128 };
		this->size = size;
		this->previousSize = size;
		this->totalRadius = 0.0f;
		this->lifeSpan = lifeSpan;
		this->turbulence = 0.0f;
		this->sphLabel = sphLabel;
		this->neighbors = 0;
		this->spawnCorrectIter = 100000000;
		this->uniqueColor = uniqueColor;
		this->isSolid = isSolid;
		this->canBeSubdivided = canBeSubdivided;
		this->canBeResized = canBeResized;
		this->isDarkMatter = isDarkMatter;
		this->isSPH = isSPH;
		this->isSelected = isSelected;
		this->isGrabbed = false;
		this->isPinned = false;
		this->isBeingDrawn = false;
	}
};

struct ParticlePhysics3D {

	glm::vec3 pos;
	glm::vec3 vel;
	glm::vec3 acc;
	glm::vec3 predPos;
	glm::vec3 prevVel;
	glm::vec3 predVel;
	glm::vec3 pressF;

	float mass;
	float press;
	float pressTmp;
	float dens;
	float predDens;
	float sphMass;
	float restDens;
	float stiff;
	float visc;
	float cohesion;
	float temp;
	float ke;
	float prevKe;
	uint32_t mortonKey;
	uint32_t id;
	uint32_t neighborOffset;

	bool isHotPoint;
	bool hasSolidified;

	// Default constructor
	ParticlePhysics3D()
		: pos(0.0f, 0.0f, 0.0f), vel{ 0.0f,0.0f, 0.0f }, acc{ 0.0f,0.0f, 0.0f }, predPos{ 0.0f,0.0f, 0.0f }, prevVel{ 0.0f, 0.0f, 0.0f }, predVel{ 0.0f, 0.0f, 0.0f }, pressF{ 0.0f,0.0f, 0.0f },
		mass(8500000000.0f), press(0.0f), pressTmp(0.0f), dens(0.0f), predDens(0.0f), sphMass(1.0f),
		restDens(0.0f), stiff(0.0f), visc(0.0f), cohesion(0.0f),
		temp(0.0f), ke(0.0f), prevKe(0.0f), mortonKey(0), id(globalId++), neighborOffset(0),
		isHotPoint(false), hasSolidified(false)
	{
	}

	// Parameterized constructor
	ParticlePhysics3D(glm::vec3 pos, glm::vec3 vel, float mass, float restDens, float stiff, float visc, float cohesion) {
		this->pos = pos;
		this->vel = vel;
		this->acc = { 0.0f, 0.0f, 0.0f };
		this->predPos = { 0.0f, 0.0f, 0.0f };
		this->prevVel = { 0.0f, 0.0f, 0.0f };
		this->predVel = { 0.0f, 0.0f, 0.0f };
		this->pressF = { 0.0f, 0.0f, 0.0f };
		this->mass = mass;
		this->press = 0.0f;
		this->pressTmp = 0.0f;
		this->dens = 0.0f;
		this->predDens = 0.0f;
		this->sphMass = mass / 8500000000.0f;
		this->restDens = restDens;
		this->stiff = stiff;
		this->visc = visc;
		this->cohesion = cohesion;
		this->temp = 288.0f;
		this->ke = 0.0f;
		this->prevKe = 0.0f;
		this->mortonKey = 0;
		this->id = globalId++;
		this->neighborOffset = 0;
		this->isHotPoint = false;
		this->hasSolidified = false;
	}
};

struct ParticleRendering3D {

	Color color;
	Color pColor;
	Color sColor;
	Color sphColor;

	float size;
	float previousSize;
	float totalRadius;
	float lifeSpan;
	float turbulence;
	uint32_t sphLabel;

	int neighbors;
	int spawnCorrectIter;

	bool uniqueColor;
	bool isSolid;
	bool canBeSubdivided;
	bool canBeResized;
	bool isDarkMatter;
	bool isSPH;
	bool isSelected;
	bool isGrabbed;
	bool isPinned;
	bool isBeingDrawn;

	// Default constructor
	ParticleRendering3D()
		: color{ 255,255,255,255 }, pColor{ 255,255,255,255 }, sColor{ 255, 255, 255, 255 }, sphColor{ 128,128,128,128 },
		size(1.0f), previousSize(1.0f), totalRadius(0.0f), lifeSpan(-1.0f), turbulence(0.0f), sphLabel(0),
		neighbors(0), spawnCorrectIter(100000000),
		uniqueColor(false), isSolid(false), canBeSubdivided(false),
		canBeResized(false), isDarkMatter(false), isSPH(false),
		isSelected(false), isGrabbed(false), isPinned(false), isBeingDrawn(true)
	{
	}

	// Parameterized constructor
	ParticleRendering3D(Color color, float size, bool uniqueColor, bool isSelected,
		bool isSolid, bool canBeSubdivided, bool canBeResized, bool isDarkMatter, bool isSPH, float lifeSpan, uint32_t sphLabel) {
		this->color = color;
		this->pColor = { 255, 255, 255, 255 };
		this->sColor = { 255, 255, 255, 255 };
		this->sphColor = { 128, 128, 128, 128 };
		this->size = size;
		this->previousSize = size;
		this->totalRadius = 0.0f;
		this->lifeSpan = lifeSpan;
		this->turbulence = 0.0f;
		this->sphLabel = sphLabel;
		this->neighbors = 0;
		this->spawnCorrectIter = 100000000;
		this->uniqueColor = uniqueColor;
		this->isSolid = isSolid;
		this->canBeSubdivided = canBeSubdivided;
		this->canBeResized = canBeResized;
		this->isDarkMatter = isDarkMatter;
		this->isSPH = isSPH;
		this->isSelected = isSelected;
		this->isGrabbed = false;
		this->isPinned = false;
		this->isBeingDrawn = false;
	}
};