#pragma once

#include "UX/randNum.h"

struct Wall {

	glm::vec2 vA;
	glm::vec2 vB;

	glm::vec2 normal;

	bool isBeingSpawned;
	bool vAisBeingMoved;
	bool vBisBeingMoved;

	Color color;

	Wall() = default;
	Wall(glm::vec2 vA, glm::vec2 vB, Color color) {
		this->vA = vA;
		this->vB = vB;
		this->isBeingSpawned = true;
		this->vAisBeingMoved = false;
		this->vBisBeingMoved = false;
		this->color = color;
	}

	void drawWall() {
		DrawLineV({ vA.x, vA.y }, { vB.x, vB.y }, color);
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

	LightRay() = default;
	LightRay(glm::vec2 source, glm::vec2 dir, int bounceLevel, Color color) {
		this->source = source;
		this->dir = dir;
		this->hitPoint = { 0.0f, 0.0f };
		this->hasHit = false;
		this->bounceLevel = bounceLevel;
		this->color = color;
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

	std::vector<LightRay> rays;

	std::vector<PointLight> pointLights;

	std::vector<AreaLight> areaLights;

	size_t firstPassTotalRays = 0;

	int sampleRaysAmount = 60000;
	int maxBounces = 3;
	int maxSamples = 2;
	int currentSamples = 1;

	bool shouldPileRays = true;

	const float lightBias = 0.1f;

	Color globalColor = { 14, 14, 14, 9 };

	Color wallColor = { 200, 200, 200, 255 };

	void createWall(UpdateParameters& myParam) {

		glm::vec2 mouseWorldPos = myParam.myCamera.mouseWorldPos;

		if (IO::shortcutPress(KEY_V)) {
			walls.emplace_back(mouseWorldPos, mouseWorldPos, wallColor);
			shouldPileRays = false;
		}

		if (IO::shortcutDown(KEY_V)) {
			if (walls.back().isBeingSpawned) {
				walls.back().vB = mouseWorldPos;
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

	void createPointLight(UpdateParameters& myParam) {

		glm::vec2 mouseWorldPos = myParam.myCamera.mouseWorldPos;

		if (IO::shortcutPress(KEY_FIVE)) {
			pointLights.emplace_back(mouseWorldPos, globalColor);

			shouldPileRays = false;
		}

		if (IO::shortcutReleased(KEY_FIVE)) {
			shouldPileRays = true;
		}
	}

	void createAreaLight(UpdateParameters& myParam) {

		glm::vec2 mouseWorldPos = myParam.myCamera.mouseWorldPos;

		if (IO::shortcutPress(KEY_SIX)) {
			areaLights.emplace_back(mouseWorldPos, mouseWorldPos, globalColor);
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
			glm::vec2 tangent = glm::normalize(w.vB - w.vA);
			glm::vec2 normal = glm::vec2(-tangent.y, tangent.x);
			if (glm::dot(normal, ray.dir) > 0.0f)
				normal = -normal;
			ray.hasHit = true;
			ray.length = minLength;
			ray.hitPoint = hitPt;
			ray.wall = w;
			ray.wall.normal = normal;
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

	void globalIllumination(int& currentBounce, LightRay& ray, std::vector<LightRay>& copyRays, std::vector<Wall>& walls) {
		if (ray.hasHit && ray.bounceLevel == currentBounce) {

			float spreadMultiplier = 0.95f;
			float maxSpreadAngle = glm::radians(90.0f * spreadMultiplier);

			float randAngle = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 2.0f * maxSpreadAngle - maxSpreadAngle;

			glm::vec2 rayDirection = rotateVec2(ray.wall.normal, randAngle);

			float absorptionInv = 0.92f;

			Color newColor = {
	static_cast<unsigned char>(ray.color.r * ray.wall.color.r / 255.0f * absorptionInv),
	static_cast<unsigned char>(ray.color.g * ray.wall.color.g / 255.0f * absorptionInv),
	static_cast<unsigned char>(ray.color.b * ray.wall.color.b / 255.0f * absorptionInv),
	static_cast<unsigned char>(ray.color.a * absorptionInv)
			};

			glm::vec2 newSource = ray.hitPoint + ray.wall.normal * lightBias;

			copyRays.emplace_back(newSource, rayDirection, ray.bounceLevel + 1, newColor);

			LightRay& newRay = copyRays.back();

			processRayIntersection(newRay);
		}
	}

	void lightRendering(UpdateParameters& myParam) {

		glm::vec2 mouseWorldPos = myParam.myCamera.mouseWorldPos;

		for (Wall& wall : walls) {
			wall.drawWall();
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
			if (maxBounces > 0) {
				for (int bounce = 1; bounce <= maxBounces; bounce++) {
					std::vector<LightRay> copyRays;
					copyRays = rays;
					for (size_t i = 0; i < copyRays.size(); i++) {

						LightRay& ray = copyRays[i];
						globalIllumination(bounce, ray, copyRays, walls);
					}
					rays = copyRays;
				}
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

		//ImGui::SliderInt("LightIntensity", &globalColor.a, 0, 255);

		//rays.emplace_back(glm::vec2{ 500.0f, 500.0f }, glm::vec2{ 1.0f, 0.0f }, 1, WHITE);

		createPointLight(myParam);
		createAreaLight(myParam);
		createWall(myParam);

		movePointLights(myParam);
		moveAreaLights(myParam);
		moveWalls(myParam);

		lightRendering(myParam);

		if (IO::shortcutPress(KEY_C)) {
			rays.clear();
			pointLights.clear();
			areaLights.clear();
			walls.clear();
		}
	}
};