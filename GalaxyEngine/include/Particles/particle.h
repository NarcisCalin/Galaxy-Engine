#pragma once

extern uint32_t globalId;

struct ParticlePhysics {
	glm::vec2 pos;
	glm::vec2 predPos;
	glm::vec2 vel;
	glm::vec2 prevVel;
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

	// Other parameters
	float temp;
	float ke;
	float prevKe;
	uint32_t mortonKey;
	uint32_t id;
	std::vector<uint32_t> neighborIds;

	bool isHotPoint;
	bool hasSolidified;

	// Default constructor
	ParticlePhysics()
		: pos(0.0f, 0.0f), predPos{ 0,0 }, vel{ 0,0 }, prevVel{ 0.0f, 0.0f }, predVel{ 0.0f, 0.0f }, acc{ 0,0 },
		mass(8500000000.0f), press(0.0f), pressTmp(0.0f), pressF{ 0.0f,0.0f }, dens(0.0f), predDens(0.0f), sphMass(1.0f),
		restDens(0.0f), stiff(0.0f), visc(0.0f), cohesion(0.0f),
		temp(0.0f), ke(0.0f), prevKe(0.0f), mortonKey(0), id(globalId++), isHotPoint(false), hasSolidified(false)
	{
	}

	// Parameterized constructor
	ParticlePhysics(glm::vec2 pos, glm::vec2 vel, float mass, float restDens, float stiff, float visc, float cohesion) {
		this->pos = pos;
		this->predPos = { 0.0f, 0.0f };
		this->vel = vel;
		this->prevVel = { 0.0f, 0.0f };
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

		// Other parameters
		this->temp = 288.0f;
		this->ke = 0.0f;
		this->prevKe = 0.0f;
		this->mortonKey = 0;
		this->id = globalId++;

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
	uint32_t sphLabel;
	bool isPinned;
	bool isBeingDrawn;
	int spawnCorrectIter;


	// Default constructor
	ParticleRendering()
		: color{ 255,255,255,255 }, pColor{ 255,255,255,255 }, sColor{ 255, 255, 255, 255 }, sphColor{ 128,128,128,128 },
		size(1.0f),
		uniqueColor(false), isSolid(false), canBeSubdivided(false),
		canBeResized(false), isDarkMatter(false), isSPH(false),
		isSelected(false), isGrabbed(false), previousSize(1.0f),
		neighbors(0), totalRadius(0.0f), lifeSpan(-1.0f), sphLabel(0), isPinned(false), isBeingDrawn(true), spawnCorrectIter(100000000)
	{
	}

	// Parameterized constructor
	ParticleRendering(Color color, float size, bool uniqueColor, bool isSelected,
		bool isSolid, bool canBeSubdivided, bool canBeResized, bool isDarkMatter, bool isSPH, float lifeSpan, uint32_t sphLabel) {
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

		this->isPinned = false;
		this->isBeingDrawn = false;
		this->spawnCorrectIter = 100000000;
	}
};