#pragma once

#include "UX/randNum.h"

struct Wall {

	glm::vec2 vA;
	glm::vec2 vB;

	glm::vec2 normal;
	glm::vec2 normalVA;
	glm::vec2 normalVB;

	bool isBeingSpawned;
	bool vAisBeingMoved;
	bool vBisBeingMoved;

	Color baseColor;
	Color specularColor;
	Color refractionColor;

	float baseColorVal;
	float specularColorVal;
	float refractionColorVal;

	float specularRoughness;
	float refractionRoughness;
	float refractionAmount;
	float IOR;

	bool isShapeWall;
	uint32_t shapeId;

	Wall() = default;
	Wall(glm::vec2 vA, glm::vec2 vB, bool isShapeWall, Color baseColor, Color specularColor, Color refractionColor, 
		float specularRoughness, float refractionRoughness, float refractionAmount, float IOR) {

		this->vA = vA;
		this->vB = vB;
		this->isShapeWall = isShapeWall;
		this->shapeId = 0;
		this->isBeingSpawned = true;
		this->vAisBeingMoved = false;
		this->vBisBeingMoved = false;
		this->baseColor = baseColor;
		this->specularColor = specularColor;
		this->refractionColor = refractionColor;

		this->baseColorVal = std::max({ baseColor.r, baseColor.g, baseColor.b }) * (baseColor.a / 255.0f) / 255.0f;
		this->specularColorVal = std::max({ specularColor.r, specularColor.g, specularColor.b }) * (specularColor.a / 255.0f) / 255.0f;
		this->refractionColorVal = std::max({refractionColor.r, refractionColor.g, refractionColor.b}) * (refractionColor.a / 255.0f) / 255.0f;

		this->specularRoughness = specularRoughness;
		this->refractionRoughness = refractionRoughness;
		this->refractionAmount = refractionAmount;
		this->IOR = IOR;
	}

	void drawWall() {
		DrawLineV({ vA.x, vA.y }, { vB.x, vB.y }, baseColor);
	}
};

extern uint32_t globalShapeId;

struct Shape {

	glm::vec2 center;
	std::vector<Wall>& walls;
	std::vector<size_t> myWallIndices;

	std::vector<glm::vec2> polygonVerts;

	/*enum class ShapeType {
		Circle1,
		Circle2,
	};*/

	Color baseColor;
	Color specularColor;
	Color refractionColor;

	float specularRoughness;
	float refractionRoughness;

	float refractionAmount;

	float IOR;

	uint32_t id;

	Shape(glm::vec2 center, std::vector<Wall>& walls, Color baseColor, Color specularColor, Color refractionColor, 
		float specularRoughness, float refractionRoughness, float refractionAmount, float IOR) :

		center(center),
		walls(walls),
		baseColor(baseColor),
		specularColor(specularColor),
		refractionColor(refractionColor),
		specularRoughness(specularRoughness),
		refractionRoughness(refractionRoughness),
		refractionAmount(refractionAmount),
		IOR(IOR),
	    id(globalShapeId++) {

		makeCircle();
	}

	float getSignedArea(const std::vector<glm::vec2>& vertices) {
		float area = 0.0f;
		int n = vertices.size();

		for (int i = 0; i < n; ++i) {
			const glm::vec2& current = vertices[i];
			const glm::vec2& next = vertices[(i + 1) % n];
			area += (current.x * next.y - next.x * current.y);
		}

		return 0.5f * area;
	}

	void calculateWallsNormals() {

		int n = myWallIndices.size();
		if (n == 0) return;

		for (int i = 0; i < myWallIndices.size(); ++i) {
			Wall& w = walls[myWallIndices[i]];
			polygonVerts.push_back(w.vA);
		}

		bool flipNormals = getSignedArea(polygonVerts) > 0.0f;

		for (int i = 0; i < n; ++i) {
			Wall& w = walls[myWallIndices[i]];

			if (!w.isShapeWall) {
				continue;
			}

			glm::vec2 tangent = glm::normalize(w.vB - w.vA);
			glm::vec2 normal = flipNormals ? glm::vec2(tangent.y, -tangent.x) : glm::vec2(-tangent.y, tangent.x);

			w.normal = normal;
		}

		for (int i = 0; i < n; ++i) {
			int prev = (i - 1 + n) % n;
			int next = (i + 1) % n;

			Wall& wPrev = walls[myWallIndices[prev]];
			Wall& w = walls[myWallIndices[i]];
			Wall& wNext = walls[myWallIndices[next]];

			if (!wPrev.isShapeWall || !w.isShapeWall || !wNext.isShapeWall) {
				continue;
			}

			w.normalVA = glm::normalize(wPrev.normal + w.normal);

			w.normalVB = glm::normalize(w.normal + wNext.normal);
		}
	}

	

	void makeCircle() {

		float radius = 30.0f;
		int segments = 100;

		std::vector<size_t> newWallIndices;

		for (int i = 0; i < segments; ++i) {
			float theta1 = (2.0f * PI * i) / segments;
			float theta2 = (2.0f * PI * (i + 1)) / segments;

			glm::vec2 vA = {
				center.x + cos(theta1) * radius,
				center.y + sin(theta1) * radius
			};
			glm::vec2 vB = {
				center.x + cos(theta2) * radius,
				center.y + sin(theta2) * radius
			};

			walls.emplace_back(vA, vB, true, baseColor, specularColor, refractionColor, specularRoughness, refractionRoughness, refractionAmount, IOR);

			walls.back().shapeId = id;

			newWallIndices.push_back(walls.size() - 1);
		}

		myWallIndices = std::move(newWallIndices);

		calculateWallsNormals();
	}
};

struct LightRay {

	glm::vec2 source;
	glm::vec2 dir;

	float maxLength = 100000.0f;
	float length = 10000.0f;

	glm::vec2 hitPoint;

	bool hasHit;
	int bounceLevel;

	Wall wall;

	Color color;

	bool reflectSpecular;
	bool refracted;

	float prevMediumIOR;

	int enterMediumCount;

	LightRay() = default;
	LightRay(glm::vec2 source, glm::vec2 dir, int bounceLevel, Color color) {
		this->source = source;
		this->dir = dir;
		this->hitPoint = { 0.0f, 0.0f };
		this->hasHit = false;
		this->bounceLevel = bounceLevel;
		this->color = color;
		this->reflectSpecular = false;
		this->refracted = false;
		this->prevMediumIOR = 1.0f;
		this->enterMediumCount = 0;
	}

	void drawRay() {
		DrawLineV({ source.x, source.y }, { source.x + (dir.x * length), source.y + (dir.y * length) }, color);
	}
};

struct PointLight {

	glm::vec2 pos;
	bool isBeingMoved;

	Color color;

	PointLight(glm::vec2 pos, Color color) {
		this->pos = pos;
		this->isBeingMoved = false;
		this->color = color;
	}

	void pointLightLogic(int& sampleRaysAmount, std::vector<LightRay>& rays) {
		float radius = 100.0f;

		for (int i = 0; i < sampleRaysAmount; i++) {
			float angle = 2.0f * PI * i / sampleRaysAmount;
			float x = pos.x + radius * std::cos(angle);
			float y = pos.y + radius * std::sin(angle);

			glm::vec2 dA = pos;

			glm::vec2 dB(x, y);

			glm::vec2 d = dB - dA;

			float length = glm::length(d);

			glm::vec2 dNormalized = d / length;

			rays.emplace_back(
				pos,
				dNormalized,
				1,
				color
			);
		}
	}

};

struct AreaLight {

	glm::vec2 vA;
	glm::vec2 vB;

	bool isBeingSpawned;
	bool vAisBeingMoved;
	bool vBisBeingMoved;

	Color color;

	AreaLight() = default;
	AreaLight(glm::vec2 vA, glm::vec2 vB, Color color) {
		this->vA = vA;
		this->vB = vB;
		this->isBeingSpawned = true;
		this->vAisBeingMoved = false;
		this->vBisBeingMoved = false;
		this->color = color;
	}

	glm::vec2 rotateVec2(glm::vec2 v, float angle) {
		float c = cos(angle);
		float s = sin(angle);
		return glm::vec2(
			v.x * c - v.y * s,
			v.x * s + v.y * c
		);
	}

	void areaLightLogic(UpdateParameters& myParam, int& sampleRaysAmount, std::vector<LightRay>& rays) {

		glm::vec2 mouseWorldPos = myParam.myCamera.mouseWorldPos;

		DrawLineV({ vA.x, vA.y }, { vB.x, vB.y }, RED);

		for (int i = 0; i < sampleRaysAmount; i++) {
			float spreadMultiplier = 0.7f;
			float maxSpreadAngle = glm::radians(90.0f * spreadMultiplier);

			float randAngle = getRandomFloat() * 2.0f * maxSpreadAngle - maxSpreadAngle;

			glm::vec2 d = vB - vA;
			float length = glm::length(d);
			glm::vec2 dNormal = d / length;
			float separation = length / static_cast<float>(sampleRaysAmount);

			glm::vec2 source = vA + dNormal * (separation * i);

			glm::vec2 rayDirection = rotateVec2(dNormal, randAngle);
			rayDirection = glm::vec2(rayDirection.y, -rayDirection.x);

			rays.emplace_back(
				source,
				rayDirection,
				1,
				color
			);
		}
	}
};

struct Lighting {

	std::vector<Wall> walls;

	std::vector<Shape> shapes;

	std::vector<LightRay> rays;

	std::vector<PointLight> pointLights;

	std::vector<AreaLight> areaLights;

	size_t firstPassTotalRays = 0;

	int sampleRaysAmount = 60000;
	int maxBounces = 3;
	int maxSamples = 1;
	int currentSamples = 1;

	bool isDiffuseEnabled = true;
	bool isSpecularEnabled = true;
	bool isRefractionEnabled = true;

	bool shouldPileRays = true;

	bool drawNormals = false;

	const float lightBias = 0.1f;

	Color lightColor = { 30, 30, 30, 10 };

	Color wallBaseColor = { 200, 200, 200, 255 };
	Color wallSpecularColor = { 255, 255, 255, 255 };
	Color wallRefractionColor = { 255, 255, 255, 255 };

	float wallSpecularRoughness = 0.5f;
	float wallRefractionRoughness = 0.0f; // This controls the roughness of the refraction surface. I separate it for extra control, like V-Ray renderer
	float wallRefractionAmount = 0.0f;
	float wallIOR = 1.5f;
	float airIOR = 1.0f;

	void calculateWallNormal(Wall& wall) {
		Wall& w = wall;
		glm::vec2 tangent = glm::normalize(w.vB - w.vA);
		glm::vec2 normal = glm::vec2(-tangent.y, tangent.x);
		w.normal = normal;
		w.normalVA = normal;
		w.normalVB = normal;
	}

	void createWall(UpdateParameters& myParam) {

		glm::vec2 mouseWorldPos = myParam.myCamera.mouseWorldPos;

		if (IO::shortcutPress(KEY_V)) {
			walls.emplace_back(mouseWorldPos, mouseWorldPos, false, wallBaseColor, wallSpecularColor, wallRefractionColor, 
				wallSpecularRoughness, wallRefractionRoughness, wallRefractionAmount, wallIOR);
			shouldPileRays = false;
		}

		if (IO::shortcutDown(KEY_V)) {
			if (walls.back().isBeingSpawned) {
				walls.back().vB = mouseWorldPos;

				calculateWallNormal(walls.back());
			}
		}

		if (IO::shortcutReleased(KEY_V)) {
			if (!walls.empty() && walls.back().isBeingSpawned) {
				if (glm::length(walls.back().vB - walls.back().vA) == 0.0f) {
					walls.pop_back();
					shouldPileRays = true;
					return;
				}

				walls.back().isBeingSpawned = false;
			}
			shouldPileRays = true;
		}
	}

	void createShape(UpdateParameters& myParam) {

		glm::vec2 mouseWorldPos = myParam.myCamera.mouseWorldPos;

		if (IO::shortcutPress(KEY_SEVEN)) {
			shapes.emplace_back(mouseWorldPos, walls, wallBaseColor, wallSpecularColor, wallRefractionColor, 
				wallSpecularRoughness, wallRefractionRoughness, wallRefractionAmount, wallIOR);
		}
	}

	void createPointLight(UpdateParameters& myParam) {

		glm::vec2 mouseWorldPos = myParam.myCamera.mouseWorldPos;

		if (IO::shortcutPress(KEY_FIVE)) {
			pointLights.emplace_back(mouseWorldPos, lightColor);

			shouldPileRays = false;
		}

		if (IO::shortcutReleased(KEY_FIVE)) {
			shouldPileRays = true;
		}
	}

	void createAreaLight(UpdateParameters& myParam) {

		glm::vec2 mouseWorldPos = myParam.myCamera.mouseWorldPos;

		if (IO::shortcutPress(KEY_SIX)) {
			areaLights.emplace_back(mouseWorldPos, mouseWorldPos, lightColor);
			shouldPileRays = false;
		}

		if (IO::shortcutDown(KEY_SIX)) {
			if (areaLights.back().isBeingSpawned) {
				areaLights.back().vB = mouseWorldPos;
			}
		}

		if (IO::shortcutReleased(KEY_SIX)) {
			if (!areaLights.empty() && areaLights.back().isBeingSpawned) {
				if (glm::length(areaLights.back().vB - areaLights.back().vA) == 0.0f) {
					areaLights.pop_back();
					shouldPileRays = true;
					return;
				}

				areaLights.back().isBeingSpawned = false;
			}
			shouldPileRays = true;
		}
	}

	void movePointLights(UpdateParameters& myParam) {

		if (IO::shortcutPress(KEY_M)) {
			glm::vec2 mouseWorldPos = myParam.myCamera.mouseWorldPos;

			glm::vec2 mouseDelta = glm::vec2(GetMouseDelta().x, GetMouseDelta().y);
			glm::vec2 scaledDelta = mouseDelta * (1.0f / myParam.myCamera.camera.zoom);

			for (PointLight& pointLight : pointLights) {

				glm::vec2 d = pointLight.pos - mouseWorldPos;

				float dist = glm::length(d);

				if (dist <= myParam.brush.brushRadius) {
					pointLight.isBeingMoved = true;
				}
			}
			shouldPileRays = false;
		}

		for (PointLight& pointLight : pointLights) {

			glm::vec2 mouseDelta = glm::vec2(GetMouseDelta().x, GetMouseDelta().y);
			glm::vec2 scaledDelta = mouseDelta * (1.0f / myParam.myCamera.camera.zoom);

			if (pointLight.isBeingMoved) {
				pointLight.pos += scaledDelta;
			}
		}

		if (IO::shortcutReleased(KEY_M)) {
			for (PointLight& pointLight : pointLights) {
				pointLight.isBeingMoved = false;
			}
			shouldPileRays = true;
		}
	}

	void moveAreaLights(UpdateParameters& myParam) {
		glm::vec2 mouseWorldPos = myParam.myCamera.mouseWorldPos;

		glm::vec2 mouseDelta = glm::vec2(GetMouseDelta().x, GetMouseDelta().y);
		glm::vec2 scaledDelta = mouseDelta * (1.0f / myParam.myCamera.camera.zoom);

		if (IO::shortcutPress(KEY_M)) {
			for (AreaLight& areaLight : areaLights) {
				glm::vec2 dA = areaLight.vA - mouseWorldPos;
				glm::vec2 dB = areaLight.vB - mouseWorldPos;

				float distA = glm::length(dA);
				float distB = glm::length(dB);

				if (distA <= myParam.brush.brushRadius) {
					areaLight.vAisBeingMoved = true;
				}
				if (distB <= myParam.brush.brushRadius) {
					areaLight.vBisBeingMoved = true;
				}
			}
			shouldPileRays = false;
		}

		if (IO::shortcutDown(KEY_M)) {
			for (AreaLight& areaLight : areaLights) {
				if (areaLight.vAisBeingMoved) {
					areaLight.vA += scaledDelta;
				}
				if (areaLight.vBisBeingMoved) {
					areaLight.vB += scaledDelta;
				}
			}
		}

		if (IO::shortcutReleased(KEY_M)) {
			for (AreaLight& areaLight : areaLights) {
				areaLight.vAisBeingMoved = false;
				areaLight.vBisBeingMoved = false;
			}
			shouldPileRays = true;
		}
	}

	void moveWalls(UpdateParameters& myParam) {
		glm::vec2 mouseWorldPos = myParam.myCamera.mouseWorldPos;

		glm::vec2 mouseDelta = glm::vec2(GetMouseDelta().x, GetMouseDelta().y);
		glm::vec2 scaledDelta = mouseDelta * (1.0f / myParam.myCamera.camera.zoom);

		if (IO::shortcutPress(KEY_M)) {
			for (Wall& wall : walls) {
				glm::vec2 dA = wall.vA - mouseWorldPos;
				glm::vec2 dB = wall.vB - mouseWorldPos;

				float distA = glm::length(dA);
				float distB = glm::length(dB);

				if (distA <= myParam.brush.brushRadius) {
					wall.vAisBeingMoved = true;
				}
				if (distB <= myParam.brush.brushRadius) {
					wall.vBisBeingMoved = true;
				}
			}
			shouldPileRays = false;
		}

		if (IO::shortcutDown(KEY_M)) {
			for (Wall& wall : walls) {
				if (wall.vAisBeingMoved) {
					wall.vA += scaledDelta;
				}
				if (wall.vBisBeingMoved) {
					wall.vB += scaledDelta;
				}

				calculateWallNormal(wall);
			}

			for (Shape& shape : shapes) {
				shape.calculateWallsNormals();
			}
		}

		if (IO::shortcutReleased(KEY_M)) {
			for (Wall& wall : walls) {
				wall.vAisBeingMoved = false;
				wall.vBisBeingMoved = false;
			}
			shouldPileRays = true;
		}
	}

	float checkIntersect(const LightRay& ray, const Wall& w) {

		const float x1 = w.vA.x, y1 = w.vA.y;
		const float x2 = w.vB.x, y2 = w.vB.y;

		const float x3 = ray.source.x, y3 = ray.source.y;
		const float x4 = ray.source.x + ray.dir.x, y4 = ray.source.y + ray.dir.y;

		const float den = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
		if (den == 0.0f) {
			return ray.maxLength;
		}

		const float t = ((x1 - x3) * (y3 - y4) - (y1 - y3) * (x3 - x4)) / den;
		const float u = -((x1 - x2) * (y1 - y3) - (y1 - y2) * (x1 - x3)) / den;

		if (t > 0.0f && t < 1.0f && u > 0.0f) {
			return u;
		}

		return ray.maxLength;
	}

	void processRayIntersection(LightRay& ray) {
		ray.hasHit = false;
		ray.length = ray.maxLength;
		float minLength = ray.maxLength;
		int hitWallIdx = -1;
		glm::vec2 hitPt;

		for (int wi = 0; wi < walls.size(); ++wi) {
			float d = checkIntersect(ray, walls[wi]);
			if (d < minLength) {
				minLength = d;
				hitWallIdx = wi;
				hitPt = ray.source + ray.dir * d;
			}
		}

		if (hitWallIdx >= 0) {
			Wall& w = walls[hitWallIdx];
			ray.hasHit = true;
			ray.length = minLength;
			ray.hitPoint = hitPt;
			ray.wall = w;
		}
	}

	glm::vec2 rotateVec2(glm::vec2 v, float angle) {
		float c = cos(angle);
		float s = sin(angle);
		return glm::vec2(
			v.x * c - v.y * s,
			v.x * s + v.y * c
		);
	}

	// This controls specular lighting, meaning reflections. It uses a roughness parameter to control how smooth a surface is
	void specularReflection(int& currentBounce, LightRay& ray, std::vector<LightRay>& copyRays, std::vector<Wall>& walls) {

		ray.reflectSpecular = true;

		glm::vec2 wallVec = ray.wall.vB - ray.wall.vA;
		float wallLength = glm::length(wallVec);
		float t = glm::clamp(glm::dot(ray.hitPoint - ray.wall.vA, wallVec) / (wallLength * wallLength), 0.0f, 1.0f);

		glm::vec2 interpolatedNormal = glm::normalize(glm::mix(ray.wall.normalVA, ray.wall.normalVB, t));

		if (glm::dot(ray.wall.normal, ray.dir) > 0.0f) {
			interpolatedNormal = -interpolatedNormal;
		}

		float r0 = ((airIOR - ray.wall.IOR) / (airIOR + ray.wall.IOR)) * ((airIOR - ray.wall.IOR) / (airIOR + ray.wall.IOR));

		float cosThetaI = -glm::dot(ray.dir, interpolatedNormal);

		cosThetaI = glm::clamp(cosThetaI, 0.0f, 1.0f);

		float rTheta = r0 + (1 - r0) * std::pow(1 - cosThetaI, 5.0f);

		if (getRandomFloat() > rTheta) {
			ray.reflectSpecular = false;
			return;
		}

		float roughness = ray.wall.specularRoughness;
		float maxSpreadAngle = glm::radians(90.0f * roughness);

		float randAngle = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 2.0f * maxSpreadAngle - maxSpreadAngle;

		glm::vec2 perfectReflection = ray.dir - 2.0f * glm::dot(ray.dir, interpolatedNormal) * interpolatedNormal;
		glm::vec2 mixedReflection = glm::normalize(glm::mix(perfectReflection, interpolatedNormal, roughness));
		glm::vec2 rayDirection = rotateVec2(mixedReflection, randAngle);

		float absorptionInv = 0.92f;

		Color newColor = {
static_cast<unsigned char>(ray.color.r * ray.wall.specularColor.r / 255.0f * absorptionInv),
static_cast<unsigned char>(ray.color.g * ray.wall.specularColor.g / 255.0f * absorptionInv),
static_cast<unsigned char>(ray.color.b * ray.wall.specularColor.b / 255.0f * absorptionInv),
static_cast<unsigned char>(ray.color.a * absorptionInv)
		};

		glm::vec2 newSource = ray.hitPoint + interpolatedNormal * lightBias;

		copyRays.emplace_back(newSource, rayDirection, ray.bounceLevel + 1, newColor);

		LightRay& newRay = copyRays.back();

		processRayIntersection(newRay);
	}

	// This controls refraction
	void refraction(int& currentBounce, LightRay& ray, std::vector<LightRay>& copyRays, std::vector<Wall>& walls) {
		ray.refracted = true;

		glm::vec2 wallVec = ray.wall.vB - ray.wall.vA;
		float wallLength = glm::length(wallVec);
		float t = glm::clamp(glm::dot(ray.hitPoint - ray.wall.vA, wallVec) / (wallLength * wallLength), 0.0f, 1.0f);
		glm::vec2 interpolatedNormal = glm::normalize(glm::mix(ray.wall.normalVA, ray.wall.normalVB, t));

		float n1, n2;
		bool entering;

		if (!ray.wall.isShapeWall) {
			entering = glm::dot(interpolatedNormal, ray.dir) < 0.0f;
			if (!entering) {
				interpolatedNormal = -interpolatedNormal;
			}

			n1 = (ray.enterMediumCount == 0) ? 1.0f : ray.prevMediumIOR;
			n2 = ray.wall.IOR;
		}
		else {
			entering = glm::dot(ray.wall.normal, ray.dir) < 0.0f;

			if (entering) {
				n1 = (ray.enterMediumCount == 0) ? 1.0f : ray.prevMediumIOR;
				n2 = ray.wall.IOR;
			}
			else {
				n1 = ray.wall.IOR;
				n2 = (ray.enterMediumCount == 1) ? 1.0f : ray.prevMediumIOR;
				interpolatedNormal = -interpolatedNormal;
			}
		}

		float eta = n1 / n2;
		float cosThetaI = -glm::dot(ray.dir, interpolatedNormal);
		float sin2ThetaT = eta * eta * (1.0f - cosThetaI * cosThetaI);

		if (sin2ThetaT > 1.0f || getRandomFloat() > ray.wall.refractionAmount) {
			ray.refracted = false;
			if (isSpecularEnabled) {
				specularReflection(currentBounce, ray, copyRays, walls);
			}
			return;
		}

		float cosThetaT = sqrtf(1.0f - sin2ThetaT);
		glm::vec2 refractedDir = eta * ray.dir + (eta * cosThetaI - cosThetaT) * interpolatedNormal;
		refractedDir = glm::normalize(refractedDir);

		float roughness = ray.wall.refractionRoughness;
		float maxSpreadAngle = glm::radians(90.0f * roughness);
		float randAngle = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 2.0f * maxSpreadAngle - maxSpreadAngle;
		refractedDir = rotateVec2(refractedDir, randAngle);

		Color newColor = {
			static_cast<unsigned char>(ray.color.r * ray.wall.refractionColor.r / 255.0f * 0.92f),
			static_cast<unsigned char>(ray.color.g * ray.wall.refractionColor.g / 255.0f * 0.92f),
			static_cast<unsigned char>(ray.color.b * ray.wall.refractionColor.b / 255.0f * 0.92f),
			static_cast<unsigned char>(ray.color.a * 0.92f)
		};

		glm::vec2 newSource = ray.hitPoint - interpolatedNormal * lightBias;
		copyRays.emplace_back(newSource, refractedDir, ray.bounceLevel + 1, newColor);

		LightRay& newRay = copyRays.back();
		if (!ray.wall.isShapeWall) {
			newRay.enterMediumCount = ray.enterMediumCount;
			newRay.prevMediumIOR = ray.prevMediumIOR;
		}
		else {
			if (entering) {
				newRay.enterMediumCount = ray.enterMediumCount + 1;
				newRay.prevMediumIOR = ray.wall.IOR;
			}
			else {
				newRay.enterMediumCount = std::max(ray.enterMediumCount - 1, 0);
				newRay.prevMediumIOR = (newRay.enterMediumCount == 0) ? 1.0f : ray.prevMediumIOR;
			}
		}

		processRayIntersection(newRay);
	}

	// This controls diffuse lighting
	void diffuseLighting(int& currentBounce, LightRay& ray, std::vector<LightRay>& copyRays, std::vector<Wall>& walls) {

		glm::vec2 wallVec = ray.wall.vB - ray.wall.vA;
		float wallLength = glm::length(wallVec);
		float t = glm::clamp(glm::dot(ray.hitPoint - ray.wall.vA, wallVec) / (wallLength * wallLength), 0.0f, 1.0f);

		glm::vec2 interpolatedNormal = glm::normalize(glm::mix(ray.wall.normalVA, ray.wall.normalVB, t));

		if (glm::dot(ray.wall.normal, ray.dir) > 0.0f) {
			interpolatedNormal = -interpolatedNormal;
		}

		float spreadMultiplier = 0.95f;
		float maxSpreadAngle = glm::radians(90.0f * spreadMultiplier);

		float randAngle = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 2.0f * maxSpreadAngle - maxSpreadAngle;

		glm::vec2 rayDirection = rotateVec2(interpolatedNormal, randAngle);

		float absorptionInv = 0.92f;

		Color newColor = {
static_cast<unsigned char>(ray.color.r * ray.wall.baseColor.r / 255.0f * absorptionInv),
static_cast<unsigned char>(ray.color.g * ray.wall.baseColor.g / 255.0f * absorptionInv),
static_cast<unsigned char>(ray.color.b * ray.wall.baseColor.b / 255.0f * absorptionInv),
static_cast<unsigned char>(ray.color.a * absorptionInv)
		};

		glm::vec2 newSource = ray.hitPoint + interpolatedNormal * lightBias;

		copyRays.emplace_back(newSource, rayDirection, ray.bounceLevel + 1, newColor);

		LightRay& newRay = copyRays.back();

		processRayIntersection(newRay);
	}

	void lightRendering(UpdateParameters& myParam) {

		glm::vec2 mouseWorldPos = myParam.myCamera.mouseWorldPos;

		for (Wall& wall : walls) {
			wall.drawWall();

			if (drawNormals) {
				DrawLineV({ wall.vA.x, wall.vA.y }, { wall.vA.x + wall.normalVA.x * 10.0f, wall.vA.y + wall.normalVA.y * 10.0f }, RED);
				DrawLineV({ wall.vB.x, wall.vB.y }, { wall.vB.x + wall.normalVB.x * 10.0f, wall.vB.y + wall.normalVB.y * 10.0f }, RED);
				DrawLineV({ (wall.vA.x + wall.vB.x) * 0.5f, (wall.vA.y + wall.vB.y) * 0.5f },
					{ (wall.vA.x + wall.vB.x) * 0.5f + wall.normal.x * 10.0f, (wall.vA.y + wall.vB.y) * 0.5f + wall.normal.y * 10.0f }, RED);
			}
		}

		if (currentSamples <= maxSamples) {

			for (PointLight& pointLight : pointLights) {
				pointLight.pointLightLogic(sampleRaysAmount, rays);
			}

			for (AreaLight& areaLight : areaLights) {
				areaLight.areaLightLogic(myParam, sampleRaysAmount, rays);
			}

			for (LightRay& ray : rays) {
				processRayIntersection(ray);
			}
			for (int bounce = 1; bounce <= maxBounces; bounce++) {
				std::vector<LightRay> nextBounceRays;

				for (LightRay& ray : rays) {
					if (ray.hasHit && ray.bounceLevel == bounce) {

						if (isSpecularEnabled && ray.wall.specularColorVal > 0.0f) {
							specularReflection(bounce, ray, nextBounceRays, walls);
						}
						if ((isRefractionEnabled && !ray.reflectSpecular && ray.wall.refractionColorVal > 0.0f)) {
							refraction(bounce, ray, nextBounceRays, walls);
						}
						if ((isDiffuseEnabled && !ray.reflectSpecular && !ray.refracted)) {
							diffuseLighting(bounce, ray, nextBounceRays, walls);
						}
					}
				}

				rays.insert(rays.end(), nextBounceRays.begin(), nextBounceRays.end());
			}


			currentSamples++;
		}

		for (LightRay& ray : rays) {
			ray.drawRay();
		}
	}

	void rayLogic(UpdateParameters& myParam) {

		if (!shouldPileRays) {
			rays.clear();
			currentSamples = 1;
		}

		//rays.emplace_back(glm::vec2{ 500.0f, 500.0f }, glm::vec2{ 1.0f, 0.0f }, 1, WHITE);

		createPointLight(myParam);
		createAreaLight(myParam);
		createWall(myParam);
		createShape(myParam);

		movePointLights(myParam);
		moveAreaLights(myParam);
		moveWalls(myParam);

		lightRendering(myParam);

		if (IO::shortcutPress(KEY_C)) {
			rays.clear();
			pointLights.clear();
			areaLights.clear();
			walls.clear();
			shapes.clear();
		}
	}
};