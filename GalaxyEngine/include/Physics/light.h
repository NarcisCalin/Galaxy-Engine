#pragma once

#include "UX/randNum.h"
#include "IO/io.h"
#include "parameters.h"

extern uint32_t globalWallId;

struct Wall {

	glm::vec2 vA;
	glm::vec2 vB;

	glm::vec2 normal{ 0.0f,0.0f };
	glm::vec2 normalVA{ 0.0f, 0.0f };
	glm::vec2 normalVB{ 0.0f, 0.0f };

	bool isBeingSpawned;
	bool vAisBeingMoved;
	bool vBisBeingMoved;

	Color apparentColor; // This is the color used to visualize the wall

	Color baseColor;
	Color specularColor;
	Color refractionColor;
	Color emissionColor;

	float baseColorVal;
	float specularColorVal;
	float refractionColorVal;

	float specularRoughness;
	float refractionRoughness;
	float refractionAmount;
	float IOR;
	float dispersionStrength;

	bool isShapeWall;
	bool isShapeClosed;
	uint32_t shapeId;

	uint32_t id;

	bool isSelected;

	Wall() = default;
	Wall(glm::vec2 vA, glm::vec2 vB, bool isShapeWall, Color baseColor, Color specularColor, Color refractionColor, Color emissionColor,
		float specularRoughness, float refractionRoughness, float refractionAmount, float IOR, float dispersionStrength) {

		this->vA = vA;
		this->vB = vB;
		this->isShapeWall = isShapeWall;
		this->isShapeClosed = false;
		this->isSelected = false;
		this->shapeId = 0;
		this->isBeingSpawned = true;
		this->vAisBeingMoved = false;
		this->vBisBeingMoved = false;
		this->baseColor = baseColor;
		this->specularColor = specularColor;
		this->refractionColor = refractionColor;
		this->emissionColor = emissionColor;

		this->apparentColor = WHITE;

		this->baseColorVal = std::max({ baseColor.r, baseColor.g, baseColor.b }) * (baseColor.a / 255.0f) / 255.0f;
		this->specularColorVal = std::max({ specularColor.r, specularColor.g, specularColor.b }) * (specularColor.a / 255.0f) / 255.0f;
		this->refractionColorVal = std::max({ refractionColor.r, refractionColor.g, refractionColor.b }) * (refractionColor.a / 255.0f) / 255.0f;

		this->specularRoughness = specularRoughness;
		this->refractionRoughness = refractionRoughness;
		this->refractionAmount = refractionAmount;
		this->IOR = IOR;
		this->dispersionStrength = dispersionStrength;

		this->id = globalWallId++;
	}

	void drawHelper(glm::vec2& helper) {
		DrawCircleV({ helper.x, helper.y }, 5.0f, PURPLE);
	}

	void drawWall() {
		DrawLineV({ vA.x, vA.y }, { vB.x, vB.y }, apparentColor);
	}
};

extern uint32_t globalShapeId;

enum ShapeType {
	circle,
	draw,
	lens
};

struct Shape {

	std::vector<Wall>* walls;
	std::vector<uint32_t> myWallIds;

	std::vector<glm::vec2> polygonVerts;

	/*enum class ShapeType {
		Circle1,
		Circle2,
	};*/

	Color baseColor;
	Color specularColor;
	Color refractionColor;
	Color emissionColor;

	float specularRoughness;
	float refractionRoughness;

	float refractionAmount;

	float IOR;
	float dispersionStrength;

	uint32_t id;

	glm::vec2 h1;
	glm::vec2 h2;

	bool isBeingSpawned;
	bool isBeingMoved;

	bool isShapeClosed;

	ShapeType shapeType;

	bool drawHoverHelpers = false;

	Shape() = default;

	Shape(ShapeType shapeType, glm::vec2 h1, glm::vec2 h2, std::vector<Wall>* walls,
		Color baseColor, Color specularColor, Color refractionColor, Color emissionColor,
		float specularRoughness, float refractionRoughness, float refractionAmount, float IOR, float dispersionStrength) :

		shapeType(shapeType),
		h1(h1),
		h2(h2),
		walls(walls),
		baseColor(baseColor),
		specularColor(specularColor),
		refractionColor(refractionColor),
		emissionColor(emissionColor),
		specularRoughness(specularRoughness),
		refractionRoughness(refractionRoughness),
		refractionAmount(refractionAmount),
		IOR(IOR),
		dispersionStrength(dispersionStrength),
		id(globalShapeId++),
		isBeingSpawned(true),
		isBeingMoved(false),
		isShapeClosed(true) {
	}

	Wall* getWallById(std::vector<Wall>& walls, uint32_t id) {
		for (Wall& wall : walls) {
			if (wall.id == id)
				return &wall;
		}
		return nullptr;
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

		int n = myWallIds.size();
		if (n == 0) return;

		polygonVerts.clear();

		for (uint32_t wallId : myWallIds) {
			Wall* w = getWallById(*walls, wallId);
			if (w) {
				polygonVerts.push_back(w->vA);
			}
		}

		bool flipNormals = getSignedArea(polygonVerts) > 0.0f;

		for (uint32_t wallId : myWallIds) {
			Wall* w = getWallById(*walls, wallId);
			if (!w) continue;

			if (!w->isShapeWall) {
				continue;
			}

			glm::vec2 tangent = glm::normalize(w->vB - w->vA);
			glm::vec2 normal = flipNormals
				? glm::vec2(tangent.y, -tangent.x)
				: glm::vec2(-tangent.y, tangent.x);

			w->normal = normal;
		}

		const float smoothingAngleThreshold = glm::cos(glm::radians(35.0f));

		for (int i = 0; i < n; ++i) {
			int prev = (i - 1 + n) % n;
			int next = (i + 1) % n;

			uint32_t idPrev = myWallIds[prev];
			uint32_t id = myWallIds[i];
			uint32_t idNext = myWallIds[next];

			Wall* wPrev = getWallById(*walls, idPrev);
			Wall* w = getWallById(*walls, id);
			Wall* wNext = getWallById(*walls, idNext);

			if (!wPrev || !w || !wNext) {
				continue;
			}

			if (!wPrev->isShapeWall || !w->isShapeWall || !wNext->isShapeWall) {
				continue;
			}

			float lenPrev = glm::length(wPrev->vB - wPrev->vA);
			float len = glm::length(w->vB - w->vA);
			float lenNext = glm::length(wNext->vB - wNext->vA);

			bool smoothWithPrev = glm::dot(wPrev->normal, w->normal) >= smoothingAngleThreshold;
			bool smoothWithNext = glm::dot(w->normal, wNext->normal) >= smoothingAngleThreshold;

			glm::vec2 tangent = glm::normalize(w->vB - w->vA);

			if (smoothWithPrev) {
				w->normalVA = glm::normalize(wPrev->normal * lenPrev + w->normal * len);
			}
			else if (smoothWithNext) {
				glm::vec2 normalVB = glm::normalize(w->normal * len + wNext->normal * lenNext);
				w->normalVA = normalVB - 2.0f * glm::dot(normalVB, tangent) * tangent;
			}
			else {
				w->normalVA = w->normal;
			}

			if (smoothWithNext) {
				w->normalVB = glm::normalize(w->normal * len + wNext->normal * lenNext);
			}
			else if (smoothWithPrev) {
				w->normalVB = w->normalVA - 2.0f * glm::dot(w->normalVA, tangent) * tangent;
			}
			else {
				w->normalVB = w->normal;
			}
		}
	}

	void relaxGeometryLogic(std::vector<glm::vec2>& vertices, int& shapeRelaxiter, float& shapeRelaxFactor) {
		if (vertices.size() < 3) return;

		std::vector<glm::vec2> temp = vertices;

		for (int iter = 0; iter < shapeRelaxiter; ++iter) {
			for (size_t i = 0; i < vertices.size(); ++i) {
				size_t prev = (i - 1 + vertices.size()) % vertices.size();
				size_t next = (i + 1) % vertices.size();

				glm::vec2 average = (temp[prev] + temp[next]) * 0.5f;

				vertices[i] = glm::mix(temp[i], average, shapeRelaxFactor);
			}
			temp = vertices;
		}
	}

	void relaxShape(int& shapeRelaxiter, float& shapeRelaxFactor) {

		polygonVerts.clear();

		for (uint32_t wallId : myWallIds) {
			Wall* w = getWallById(*walls, wallId);
			if (w) {
				polygonVerts.push_back(w->vA);
			}
		}

		relaxGeometryLogic(polygonVerts, shapeRelaxiter, shapeRelaxFactor);

		for (size_t i = 0; i < myWallIds.size(); ++i) {
			uint32_t wallId = myWallIds[i];
			Wall* w = getWallById(*walls, wallId);

			if (w) {
				w->vA = polygonVerts[i];
				w->vB = polygonVerts[(i + 1) % polygonVerts.size()];
			}
		}
	}

	void makeShape() {

		if (shapeType == circle) {
			makeCircle();
		}

		if (shapeType == draw) {
			drawShape();
		}

		if (shapeType == lens) {
			makeLens();
		}
	}

	bool createShapeFlag = false;

	std::vector<glm::vec2> helpers;

	int circleSegments = 100;

	float circleRadius = 0.0f;

	void makeCircle() {

		if (isBeingSpawned) {
			circleRadius = glm::length(h2 - h1);
		}
		else {
			circleRadius = glm::length(helpers[0] - helpers[1]);
		}


		std::vector<uint32_t> newWallIds;

		for (int i = 0; i < circleSegments; ++i) {
			float theta1 = (2.0f * PI * i) / circleSegments;
			float theta2 = (2.0f * PI * (i + 1)) / circleSegments;

			if (!isBeingSpawned) {
				h1 = helpers[0];
			}

			glm::vec2 vA = {
				h1.x + cos(theta1) * circleRadius,
				h1.y + sin(theta1) * circleRadius
			};
			glm::vec2 vB = {
				h1.x + cos(theta2) * circleRadius,
				h1.y + sin(theta2) * circleRadius
			};

			if (createShapeFlag) {
				walls->emplace_back(vA, vB, true, baseColor, specularColor, refractionColor, emissionColor,
					specularRoughness, refractionRoughness, refractionAmount, IOR, dispersionStrength);

				Wall& newWall = walls->back();
				newWall.shapeId = id;
				newWall.isShapeClosed = true;

				newWallIds.push_back(newWall.id);
			}

			if (isBeingMoved && i < myWallIds.size()) {
				uint32_t wId = myWallIds[i];
				if (Wall* w = getWallById(*walls, wId)) {
					w->vA = vA;
					w->vB = vB;
				}
			}
		}

		if (isBeingMoved) {
			calculateWallsNormals();
		}


		if (createShapeFlag) {
			myWallIds = std::move(newWallIds);

			calculateWallsNormals();

			createShapeFlag = false;
		}
	}

	glm::vec2 prevPoint = h1;

	glm::vec2 oldDrawHelperPos{ 0.0f, 0.0f };

	void drawShape() {
		float maxSegmentLength = 4.0f;

		glm::vec2 dir = h2 - prevPoint;
		float dist = glm::length(dir);

		if (isBeingSpawned) {
			if (dist < maxSegmentLength) return;
		}

		glm::vec2 dirNorm = glm::normalize(dir);

		if (isBeingSpawned) {
			while (dist >= maxSegmentLength) {
				glm::vec2 nextPoint = prevPoint + dirNorm * maxSegmentLength;

				(*walls).emplace_back(prevPoint, nextPoint, true, baseColor, specularColor, refractionColor, emissionColor,
					specularRoughness, refractionRoughness, refractionAmount, IOR, dispersionStrength);

				prevPoint = nextPoint;
				dist = glm::length(h2 - prevPoint);

				(*walls).back().shapeId = id;
				(*walls).back().isShapeClosed = true;

				myWallIds.push_back((*walls).back().id);
			}
		}

		if (isBeingSpawned) {
			for (auto& wallId : myWallIds) {

				Wall* w = getWallById(*walls, wallId);

				if (w) {
					helpers[0] += w->vA;
					helpers[0] += w->vB;
				}
			}

			helpers[0] /= static_cast<float>(myWallIds.size()) * 2.0f;

			oldDrawHelperPos = helpers[0];
		}

		if (isBeingMoved) {
			glm::vec2 delta = helpers[0] - oldDrawHelperPos;



			if (delta != glm::vec2(0.0f)) {

				for (auto& wallId : myWallIds) {
					Wall* w = getWallById(*walls, wallId);

					if (w) {
						w->vA += delta;
						w->vB += delta;
					}
				}

				oldDrawHelperPos = helpers[0];
			}
		}
	}

	bool secondHelper = false;
	bool thirdHelper = false;
	bool fourthHelper = false;

	float Tempsh2Length = 0.0f;
	float Tempsh2LengthSymmetry = 0.0f;
	float tempDist = 0.0f;

	glm::vec2 moveH2 = h2;

	bool isThirdBeingMoved = false;
	bool isFourthBeingMoved = false;
	bool isFifthBeingMoved = false;

	bool isGlobalHelperMoved = false;

	glm::vec2 globalLensPrev = { 0.0f, 0.0f };

	bool isFifthFourthMoved = false;

	bool symmetricalLens = true;

	uint32_t wallAId = 1;
	uint32_t wallBId = 2;
	uint32_t wallCId = 3;

	int lensSegments = 50;

	void drawHelper(glm::vec2& helper) {
		DrawCircleV({ helper.x, helper.y }, 5.0f, PURPLE);
	}

	float startAngle = 0.0f;
	float endAngle = 0.0f;

	float startAngleSymmetry = 0.0f;
	float endAngleSymmetry = 0.0f;

	glm::vec2 center{ 0.0f, 0.0f };
	float radius = 0.0f;

	glm::vec2 centerSymmetry{ 0.0f, 0.0f };
	float radiusSymmetry = 0.0f;

	glm::vec2 arcEnd{ 0.0f, 0.0f };

	// The lens code is huge and can be improved in many apsects, but I honestly don't care much about it so I'm leaving it as it is

	void makeLens() {

		if (helpers.empty()) {
			helpers.push_back(h1);
		}

		if (secondHelper) {
			helpers.push_back(h2);
			secondHelper = false;
		}

		glm::vec2 thirdHelperPos = h2;
		glm::vec2 otherSide = h2;
		if (helpers.size() == 2 || isBeingMoved) {

			glm::vec2 tangent = glm::normalize(helpers.at(0) - helpers.at(1));

			glm::vec2 normal = glm::vec2(tangent.y, -tangent.x);

			glm::vec2 offset = h2 - helpers.at(1);

			float dist;
			if (!isBeingMoved) {

				dist = glm::dot(offset, normal);
				tempDist = dist;
			}
			else if (isBeingMoved && isThirdBeingMoved) {
				offset = moveH2 - helpers.at(1);

				dist = glm::dot(offset, normal);
				tempDist = dist;
			}
			else {
				dist = tempDist;
			}

			thirdHelperPos = helpers.at(1) + dist * normal;

			otherSide = helpers.at(0) + dist * normal;

			if (isBeingMoved) {
				helpers.at(2) = thirdHelperPos;
			}
		}

		if (thirdHelper) {

			helpers.push_back(thirdHelperPos);
			thirdHelper = false;
		}

		if (helpers.size() >= 3) {

			glm::vec2 direction = -glm::normalize(helpers.at(1) - helpers.at(0));

			glm::vec2 directionSymmetry = glm::normalize(helpers.at(1) - helpers.at(0));

			float arcWidth = std::abs(glm::length(helpers.at(1) - helpers.at(0)));
			arcEnd = helpers.at(2) + direction * arcWidth;

			glm::vec2 arcEndSymmetry = helpers.at(2) - direction * arcWidth;

			glm::vec2 normal = glm::vec2(direction.y, -direction.x);

			glm::vec2 toEnd = helpers.at(2) - helpers.at(1);
			float cross = direction.x * toEnd.y - direction.y * toEnd.x;
			if (cross < 0) {
				normal = -normal;
			}

			glm::vec2 offset = helpers.at(2) - helpers.at(1);

			float dist = glm::dot(offset, normal);

			glm::vec2 midPoint = (helpers.at(2) + (helpers.at(0) + dist * normal)) * 0.5f;

			glm::vec2 midPointSymmetry = (helpers.at(0) + helpers.at(1)) * 0.5f;

			glm::vec2 midToh2 = midPoint - h2;

			glm::vec2 midToh2Symmetry = midPointSymmetry - h2;

			float h2Length;
			float h2LengthSymmetry;

			if (!isBeingMoved) {
				h2Length = glm::dot(midToh2, normal);
				h2LengthSymmetry = glm::dot(midToh2, normal);

				Tempsh2Length = h2Length;
				Tempsh2LengthSymmetry = h2LengthSymmetry;
			}
			else if (isBeingMoved && isFifthFourthMoved) {
				midToh2 = midPoint - moveH2;

				h2Length = glm::dot(midToh2, normal);

				Tempsh2Length = h2Length;

				h2LengthSymmetry = h2Length;

				Tempsh2LengthSymmetry = h2Length;
			}
			else if (isBeingMoved && isFourthBeingMoved) {

				midToh2 = midPoint - moveH2;

				h2Length = glm::dot(midToh2, normal);

				Tempsh2Length = h2Length;
				h2LengthSymmetry = Tempsh2LengthSymmetry;
			}
			else if (isBeingMoved && isFifthBeingMoved) {
				midToh2Symmetry = midPointSymmetry - moveH2;

				h2LengthSymmetry = glm::dot(midToh2Symmetry, -normal);

				h2Length = Tempsh2Length;
				Tempsh2LengthSymmetry = h2LengthSymmetry;
			}
			else {
				h2Length = Tempsh2Length;
				h2LengthSymmetry = Tempsh2LengthSymmetry;
			}

			h2Length = std::clamp(h2Length, -arcWidth * 0.48f, arcWidth * 0.48f);
			h2LengthSymmetry = std::clamp(h2LengthSymmetry, -arcWidth * 0.48f, arcWidth * 0.48f);

			glm::vec2 p1 = helpers.at(2);
			glm::vec2 p2 = midPoint - h2Length * normal;
			glm::vec2 p3 = helpers.at(0) + dist * normal;

			glm::vec2 p1Symmetry = helpers.at(0);
			glm::vec2 p2Symmetry = midPointSymmetry - h2LengthSymmetry * -normal;
			glm::vec2 p3Symmetry = helpers.at(1);

			glm::vec2 mid1 = (p1 + p2) * 0.5f;
			glm::vec2 dir1 = glm::vec2(p2.y - p1.y, p1.x - p2.x);

			glm::vec2 mid2 = (p2 + p3) * 0.5f;
			glm::vec2 dir2 = glm::vec2(p3.y - p2.y, p2.x - p3.x);

			float denominator = dir2.x * dir1.y - dir2.y * dir1.x;
			if (std::abs(denominator) > 1e-6f) {
				float t = (dir2.x * (mid2.y - mid1.y) - dir2.y * (mid2.x - mid1.x)) / denominator;
				center = mid1 + t * dir1;
				radius = glm::length(center - p1);
			}
			else {
				center = mid1;
				radius = std::numeric_limits<float>::max();
			}

			startAngle = atan2(p1.y - center.y, p1.x - center.x);
			endAngle = atan2(p3.y - center.y, p3.x - center.x);


			float angleDiff = endAngle - startAngle;


			glm::vec2 mid1Symmetry = (p1Symmetry + p2Symmetry) * 0.5f;
			glm::vec2 dir1Symmetry = glm::vec2(p2Symmetry.y - p1Symmetry.y, p1Symmetry.x - p2Symmetry.x);

			glm::vec2 mid2Symmetry = (p2Symmetry + p3Symmetry) * 0.5f;
			glm::vec2 dir2Symmetry = glm::vec2(p3Symmetry.y - p2Symmetry.y, p2Symmetry.x - p3Symmetry.x);

			float denominatorSymmetry = dir2Symmetry.x * dir1Symmetry.y - dir2Symmetry.y * dir1Symmetry.x;
			if (std::abs(denominatorSymmetry) > 1e-6f) {
				float tSymmetry = (dir2Symmetry.x * (mid2Symmetry.y - mid1Symmetry.y) - dir2Symmetry.y * (mid2Symmetry.x - mid1Symmetry.x)) / denominatorSymmetry;
				centerSymmetry = mid1Symmetry + tSymmetry * dir1Symmetry;
				radiusSymmetry = glm::length(centerSymmetry - p1Symmetry);
			}
			else {
				centerSymmetry = mid1Symmetry;
				radiusSymmetry = std::numeric_limits<float>::max();
			}

			startAngleSymmetry = atan2(p1Symmetry.y - centerSymmetry.y, p1Symmetry.x - centerSymmetry.x);
			endAngleSymmetry = atan2(p3Symmetry.y - centerSymmetry.y, p3Symmetry.x - centerSymmetry.x);


			float angleDiffSymmetry = endAngleSymmetry - startAngleSymmetry;

			if (angleDiff > PI) {
				endAngle -= 2 * PI;
			}
			else if (angleDiff < -PI) {
				endAngle += 2 * PI;
			}

			if (angleDiffSymmetry > PI) {
				endAngleSymmetry -= 2 * PI;
			}
			else if (angleDiffSymmetry < -PI) {
				endAngleSymmetry += 2 * PI;
			}

			std::vector<uint32_t> newWallIds;

			if (fourthHelper || isBeingMoved) {
				if (!isBeingMoved) {
					helpers.push_back(p2);

					if (symmetricalLens) {
						helpers.push_back(p2Symmetry);
					}
				}
				else {
					helpers.at(3) = p2;
					if (symmetricalLens) {
						helpers.at(4) = p2Symmetry;
					}
				}

				if (!isBeingMoved) {
					if (!symmetricalLens) {
						(*walls).emplace_back(helpers.at(0), helpers.at(1), true, baseColor, specularColor, refractionColor, emissionColor,
							specularRoughness, refractionRoughness, refractionAmount, IOR, dispersionStrength);
						(*walls).back().shapeId = id;
						(*walls).back().isShapeClosed = true;
						newWallIds.push_back((*walls).back().id);
						wallAId = (*walls).back().id;
					}

					(*walls).emplace_back(helpers.at(1), helpers.at(2), true, baseColor, specularColor, refractionColor, emissionColor,
						specularRoughness, refractionRoughness, refractionAmount, IOR, dispersionStrength);
					(*walls).back().shapeId = id;
					(*walls).back().isShapeClosed = true;
					newWallIds.push_back((*walls).back().id);
					wallBId = (*walls).back().id;
				}
				else {
					if (!symmetricalLens) {
						for (auto& wId : myWallIds) {

							if (wId == wallAId) {

								Wall* w = getWallById(*walls, wId);

								w->vA = helpers[0];
								w->vB = helpers[1];
							}
						}
					}

					for (auto& wId : myWallIds) {

						if (wId == wallBId) {

							Wall* w = getWallById(*walls, wId);

							w->vA = helpers[1];
							w->vB = helpers[2];
						}
					}
				}
			}

			for (int i = 0; i < lensSegments; i++) {
				float t1 = static_cast<float>(i) / lensSegments;
				float t2 = static_cast<float>((i + 1)) / lensSegments;

				float angle1 = startAngle + t1 * (endAngle - startAngle);
				float angle2 = startAngle + t2 * (endAngle - startAngle);

				glm::vec2 arcP1 = center + glm::vec2(cos(angle1), sin(angle1)) * radius;
				glm::vec2 arcP2 = center + glm::vec2(cos(angle2), sin(angle2)) * radius;

				if (fourthHelper || isBeingMoved) {
					if (!isBeingMoved) {
						(*walls).emplace_back(arcP1, arcP2, true, baseColor, specularColor, refractionColor, emissionColor,
							specularRoughness, refractionRoughness, refractionAmount, IOR, dispersionStrength);
						(*walls).back().shapeId = id;
						(*walls).back().isShapeClosed = true;
						newWallIds.push_back((*walls).back().id);
					}
					else {

						for (auto& wId : myWallIds) {

							if (wId == wallBId + i + 1) {

								Wall* w = getWallById(*walls, wId);

								w->vA = arcP1;
								w->vB = arcP2;
							}
						}
					}
				}
			}

			if (fourthHelper || isBeingMoved) {

				if (!isBeingMoved) {
					(*walls).emplace_back(arcEnd, helpers.at(0), true, baseColor, specularColor, refractionColor, emissionColor,
						specularRoughness, refractionRoughness, refractionAmount, IOR, dispersionStrength);

					(*walls).back().shapeId = id;
					(*walls).back().isShapeClosed = true;

					newWallIds.push_back((*walls).back().id);
					wallCId = (*walls).back().id;
				}
				else {

					for (auto& wId : myWallIds) {

						if (wId == wallCId) {

							Wall* w = getWallById(*walls, wId);

							w->vA = arcEnd;
							w->vB = helpers[0];
						}
					}
				}
			}

			if (symmetricalLens) {
				for (int i = 0; i < lensSegments; i++) {
					float t1Symmetry = static_cast<float>(i) / lensSegments;
					float t2Symmetry = static_cast<float>((i + 1)) / lensSegments;

					float angle1Symmetry = startAngleSymmetry + t1Symmetry * (endAngleSymmetry - startAngleSymmetry);
					float angle2Symmetry = startAngleSymmetry + t2Symmetry * (endAngleSymmetry - startAngleSymmetry);

					glm::vec2 arcP1Symmetry = centerSymmetry + glm::vec2(cos(angle1Symmetry), sin(angle1Symmetry)) * radiusSymmetry;
					glm::vec2 arcP2Symmetry = centerSymmetry + glm::vec2(cos(angle2Symmetry), sin(angle2Symmetry)) * radiusSymmetry;

					if (fourthHelper || isBeingMoved) {
						if (!isBeingMoved) {
							(*walls).emplace_back(arcP1Symmetry, arcP2Symmetry, true, baseColor, specularColor, refractionColor, emissionColor,
								specularRoughness, refractionRoughness, refractionAmount, IOR, dispersionStrength);
							(*walls).back().shapeId = id;
							(*walls).back().isShapeClosed = true;
							newWallIds.push_back((*walls).back().id);
						}
						else {

							for (auto& wId : myWallIds) {

								if (wId == wallCId + i + 1) {

									Wall* w = getWallById(*walls, wId);

									w->vA = arcP1Symmetry;
									w->vB = arcP2Symmetry;
								}
							}
						}
					}
				}
			}

			if (fourthHelper || isBeingMoved) {

				if (!isBeingMoved) {
					myWallIds = std::move(newWallIds);

					glm::vec2 averageHelper = { 0.0f, 0.0f };

					for (glm::vec2& h : helpers) {
						averageHelper += h;
					}

					averageHelper /= helpers.size();

					helpers.push_back(averageHelper);

					globalLensPrev = helpers.back();
				}
				else if(isBeingMoved && !isGlobalHelperMoved) {
					helpers.back() = {0.0f, 0.0f};

					for (size_t i = 0; i < helpers.size() - 1; i++) {
						helpers.back() += helpers[i];
					}

					helpers.back() /= helpers.size() - 1;

					globalLensPrev = helpers.back();
				}
				else if (isBeingMoved && isGlobalHelperMoved) {
					glm::vec2 delta = helpers.back() - globalLensPrev;

					if (delta != glm::vec2(0.0f)) {
						for (size_t i = 0; i < helpers.size() - 1; i++) {
							helpers[i] += delta;
						}

						globalLensPrev = helpers.back();
					}
				}

				calculateWallsNormals();

				fourthHelper = false;

				if (!isBeingMoved) {
					isBeingSpawned = false;
				}
			}
		}
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

	std::vector<float> mediumIORStack;

	bool hasBeenDispersed;
	bool hasBeenScattered;

	glm::vec2 scatterSource;

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
		this->mediumIORStack = { 1.0f };
		this->hasBeenDispersed = false;
		this->hasBeenScattered = false;
		this->scatterSource = { 0.0f, 0.0f };
	}

	void drawRay() {
		DrawLineV({ source.x, source.y }, { source.x + (dir.x * length), source.y + (dir.y * length) }, color);
	}
};

struct PointLight {

	glm::vec2 pos;
	bool isBeingMoved;

	Color color;

	Color apparentColor;

	bool isSelected;

	PointLight() = default;

	PointLight(glm::vec2 pos, Color color) {
		this->pos = pos;
		this->isBeingMoved = false;
		this->color = color;

		this->apparentColor = WHITE;

		this->isSelected = false;
	}

	void drawHelper(glm::vec2& helper) {
		DrawCircleV({ helper.x, helper.y }, 5.0f, PURPLE);
	}

	void pointLightLogic(int& sampleRaysAmount, int& currentSamples, int& maxSamples, std::vector<LightRay>& rays) {
		float radius = 100.0f;

		const float goldenAngle = PI * (3.0f - std::sqrt(5.0f));

		int startIndex = currentSamples * sampleRaysAmount;

		for (int i = 0; i < sampleRaysAmount; i++) {
			int rayIndex = startIndex + i;

			float angle = rayIndex * goldenAngle;

			glm::vec2 d = glm::vec2(std::cos(angle), std::sin(angle));
			rays.emplace_back(
				pos,
				d,
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

	Color apparentColor;

	bool isSelected;

	float spread;

	AreaLight() = default;

	AreaLight(glm::vec2 vA, glm::vec2 vB, Color color, float spread) {
		this->vA = vA;
		this->vB = vB;
		this->isBeingSpawned = true;
		this->vAisBeingMoved = false;
		this->vBisBeingMoved = false;
		this->color = color;
		this->apparentColor = WHITE;

		this->spread = spread;

		this->isSelected = false;
	}

	void drawHelper(glm::vec2& helper) {
		DrawCircleV({ helper.x, helper.y }, 5.0f, PURPLE);
	}

	glm::vec2 rotateVec2(glm::vec2 v, float angle) {
		float c = cos(angle);
		float s = sin(angle);
		return glm::vec2(
			v.x * c - v.y * s,
			v.x * s + v.y * c
		);
	}

	void drawAreaLight() {
		DrawLineV({ vA.x, vA.y }, { vB.x, vB.y }, apparentColor);
	}

	void areaLightLogic(int& sampleRaysAmount, std::vector<LightRay>& rays) {

		for (int i = 0; i < sampleRaysAmount; i++) {

			float maxSpreadAngle = glm::radians(90.0f * spread);

			float randAngle = getRandomFloat() * 2.0f * maxSpreadAngle - maxSpreadAngle;

			glm::vec2 d = vB - vA;
			float length = glm::length(d);
			glm::vec2 dNormal = d / length;

			float t = getRandomFloat();
			glm::vec2 source = vA + d * t;

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

struct ConeLight {

	glm::vec2 vA;
	glm::vec2 vB;

	bool isBeingSpawned;
	bool vAisBeingMoved;
	bool vBisBeingMoved;

	Color color;

	Color apparentColor;

	bool isSelected;

	float spread;

	ConeLight() = default;

	ConeLight(glm::vec2 vA, glm::vec2 vB, Color color, float spread) {
		this->vA = vA;
		this->vB = vB;
		this->isBeingSpawned = true;
		this->vAisBeingMoved = false;
		this->vBisBeingMoved = false;
		this->color = color;
		this->apparentColor = WHITE;

		this->spread = spread;

		this->isSelected = false;
	}

	void drawHelper(glm::vec2& helper) {
		DrawCircleV({ helper.x, helper.y }, 5.0f, PURPLE);
	}

	glm::vec2 rotateVec2(glm::vec2 v, float angle) {
		float c = cos(angle);
		float s = sin(angle);
		return glm::vec2(
			v.x * c - v.y * s,
			v.x * s + v.y * c
		);
	}

	void coneLightLogic(int& sampleRaysAmount, std::vector<LightRay>& rays) {

		for (int i = 0; i < sampleRaysAmount; i++) {
			float maxSpreadAngle = glm::radians(90.0f * spread);

			float randAngle = getRandomFloat() * 2.0f * maxSpreadAngle - maxSpreadAngle;

			glm::vec2 d = vB - vA;
			float length = glm::length(d);
			glm::vec2 dNormal = d / length;

			glm::vec2 direction = glm::normalize(vB - vA);

			direction = rotateVec2(dNormal, randAngle);

			rays.emplace_back(
				vA,
				direction,
				1,
				color
			);
		}
	}
};

struct AABB2D {
	glm::vec2 min;
	glm::vec2 max;
	AABB2D() : min(0), max(0) {}
	AABB2D(glm::vec2 a, glm::vec2 b) {
		min = glm::min(a, b);
		max = glm::max(a, b);
	}
	void expand(const AABB2D& other) {
		min = glm::min(min, other.min);
		max = glm::max(max, other.max);
	}
	bool intersectsRay(const glm::vec2& origin, const glm::vec2& dir, float maxDist = FLT_MAX) const {
		const float epsilon = 1e-8f;
		glm::vec2 invDir;
		invDir.x = (std::abs(dir.x) < epsilon) ? (dir.x >= 0 ? 1e8f : -1e8f) : 1.0f / dir.x;
		invDir.y = (std::abs(dir.y) < epsilon) ? (dir.y >= 0 ? 1e8f : -1e8f) : 1.0f / dir.y;

		glm::vec2 t0s = (min - origin) * invDir;
		glm::vec2 t1s = (max - origin) * invDir;
		glm::vec2 tMin = glm::min(t0s, t1s);
		glm::vec2 tMax = glm::max(t0s, t1s);
		float tEntry = std::max(tMin.x, tMin.y);
		float tExit = std::min(tMax.x, tMax.y);
		return tExit >= 0 && tEntry <= tExit && tEntry <= maxDist;
	}
};

struct BVHNode {
	AABB2D bounds;
	BVHNode* left = nullptr;
	BVHNode* right = nullptr;
	Wall* wall = nullptr;
	bool isLeaf() const { return wall != nullptr; }
};

class BVH {
public:
	BVHNode* root = nullptr;
	BVH() = default;
	~BVH() {
		destroyBVH(root);
	}

	void build(std::vector<Wall*>& walls) {
		destroyBVH(root);
		root = nullptr;

		if (walls.empty()) {
			return;
		}
		root = buildBVH(walls, 0, walls.size(), 0);
	}

	bool traverse(const LightRay& ray, float& closestT, Wall*& hitWall, glm::vec2& hitPoint) {
		if (!root) return false;
		return traverseBVH(root, ray, closestT, hitWall, hitPoint);
	}

private:
	void destroyBVH(BVHNode* node) {
		if (!node) return;
		destroyBVH(node->left);
		destroyBVH(node->right);
		delete node;
	}

	BVHNode* buildBVH(std::vector<Wall*>& walls, int start, int end, int depth) {
		if (start >= end) return nullptr;

		BVHNode* node = new BVHNode{};

		AABB2D bounds = AABB2D(walls[start]->vA, walls[start]->vB);
		for (int i = start + 1; i < end; ++i) {
			AABB2D wallBox(walls[i]->vA, walls[i]->vB);
			bounds.expand(wallBox);
		}
		node->bounds = bounds;

		int count = end - start;

		if (count <= 1 || depth >= 20) {
			node->wall = walls[start];
			return node;
		}

		glm::vec2 extent = bounds.max - bounds.min;
		int axis = extent.x > extent.y ? 0 : 1;

		std::sort(walls.begin() + start, walls.begin() + end, [axis](Wall* a, Wall* b) {
			float midA = (a->vA[axis] + a->vB[axis]) * 0.5f;
			float midB = (b->vA[axis] + b->vB[axis]) * 0.5f;
			return midA < midB;
			});

		int mid = start + count / 2;

		if (mid == start) mid = start + 1;
		if (mid >= end) mid = end - 1;

		node->left = buildBVH(walls, start, mid, depth + 1);
		node->right = buildBVH(walls, mid, end, depth + 1);

		return node;
	}

	bool intersectWall(const LightRay& ray, const Wall& wall, float& t, glm::vec2& hitPoint) {
		glm::vec2 p = ray.source;
		glm::vec2 r = ray.dir;
		glm::vec2 q = wall.vA;
		glm::vec2 s = wall.vB - wall.vA;

		float rxs = r.x * s.y - r.y * s.x;
		const float epsilon = 1e-8f;
		if (std::abs(rxs) < epsilon) return false;

		glm::vec2 qp = q - p;
		float t1 = (qp.x * s.y - qp.y * s.x) / rxs;
		float t2 = (qp.x * r.y - qp.y * r.x) / rxs;

		if (t1 >= 0 && t1 <= ray.maxLength && t2 >= 0.0f && t2 <= 1.0f) {
			t = t1;
			hitPoint = p + t * r;
			return true;
		}
		return false;
	}

	bool traverseBVH(const BVHNode* root, const LightRay& ray, float& closestT, Wall*& hitWall, glm::vec2& hitPoint) {
		if (!root) return false;

		std::stack<const BVHNode*> stack;
		stack.push(root);
		bool hit = false;

		while (!stack.empty()) {
			const BVHNode* node = stack.top();
			stack.pop();

			if (!node->bounds.intersectsRay(ray.source, ray.dir, closestT))
				continue;

			if (node->isLeaf()) {
				float t;
				glm::vec2 pt;
				if (intersectWall(ray, *node->wall, t, pt) && t < closestT) {
					closestT = t;
					hitWall = node->wall;
					hitPoint = pt;
					hit = true;
				}
			}
			else {
				if (node->right) stack.push(node->right);
				if (node->left) stack.push(node->left);
			}
		}
		return hit;
	}
};

struct Lighting {

	std::vector<Wall> walls;
	std::vector<Wall*> wallPointers;

	std::vector<Shape> shapes;

	std::vector<LightRay> rays;

	std::vector<PointLight> pointLights;
	std::vector<AreaLight> areaLights;
	std::vector<ConeLight> coneLights;

	size_t firstPassTotalRays = 0;

	BVH bvh;

	int sampleRaysAmount = 1024;
	int maxBounces = 3;
	int maxSamples = 500;
	int currentSamples = 0;

	bool isDiffuseEnabled = true;
	bool isSpecularEnabled = true;
	bool isRefractionEnabled = true;
	bool isDispersionEnabled = true;
	bool isEmissionEnabled = true;

	bool symmetricalLens = false;

	bool shouldRender = true;

	bool drawNormals = false;

	bool relaxMove = false;

	const float lightBias = 0.1f;

	Color lightColor = { 255, 255, 255, 64 };

	Color wallBaseColor = { 200, 200, 200, 255 };
	Color wallSpecularColor = { 255, 255, 255, 255 };
	Color wallRefractionColor = { 255, 255, 255, 255 };
	Color wallEmissionColor = { 255, 255, 255, 0 };

	float lightGain = 0.25f;

	float lightSpread = 0.85f;

	float wallSpecularRoughness = 0.5f;
	float wallRefractionRoughness = 0.0f; // This controls the roughness of the refraction surface. I separate it for extra control, like V-Ray renderer
	float wallRefractionAmount = 0.0f;
	float wallIOR = 1.5f;
	float airIOR = 1.0f;

	float wallDispersion = 0.0f;

	float wallEmissionGain = 0.0f;

	int shapeRelaxIter = 15;
	float shapeRelaxFactor = 0.65f;

	float absorptionInvBias = 0.99f;

	Wall* getWallById(std::vector<Wall>& walls, uint32_t id) {
		for (Wall& wall : walls) {
			if (wall.id == id)
				return &wall;
		}
		return nullptr;
	}

	void calculateWallNormal(Wall& wall) {
		Wall& w = wall;
		glm::vec2 tangent = glm::normalize(w.vB - w.vA);
		glm::vec2 normal = glm::vec2(-tangent.y, tangent.x);
		w.normal = normal;
		w.normalVA = normal;
		w.normalVB = normal;
	}

	void createWall(UpdateVariables& myVar, UpdateParameters& myParam);

	bool firstHelper = true;
	bool isCreatingLens = false;

	float minHelperLength = FLT_MAX;
	int selectedHelper = -1;
	size_t selectedShape = -1;

	const float helperMinDist = 100.0f;

	void createShape(UpdateVariables& myVar, UpdateParameters& myParam);

	void createPointLight(UpdateVariables& myVar, UpdateParameters& myParam);

	void createAreaLight(UpdateVariables& myVar, UpdateParameters& myParam);

	void createConeLight(UpdateVariables& myVar, UpdateParameters& myParam);

	void movePointLights(UpdateVariables& myVar, UpdateParameters& myParam);

	void moveAreaLights(UpdateVariables& myVar, UpdateParameters& myParam);

	void moveConeLights(UpdateVariables& myVar, UpdateParameters& myParam);

	void moveWalls(UpdateVariables& myVar, UpdateParameters& myParam);

	bool isAnyShapeBeingSpawned = false;

	void moveLogic(UpdateVariables& myVar, UpdateParameters& myParam);

	void eraseLogic(UpdateVariables& myVar, UpdateParameters& myParam);

	float lightGainAvg = 0.0f;
	bool isSliderLightGain = false;

	float lightSpreadAvg = 0.0f;
	bool isSliderlightSpread = false;

	Color lightColorAvg = { 0, 0, 0, 0 };
	bool isSliderLightColor = false;

	Color baseColorAvg = { 0, 0, 0, 0 };
	bool isSliderBaseColor = false;

	Color specularColorAvg = { 0, 0, 0, 0 };
	bool isSliderSpecularColor = false;

	Color refractionColorAvg = { 0, 0, 0, 0 };
	bool isSliderRefractionCol = false;

	Color emissionColorAvg = { 0, 0, 0, 0 };
	bool isSliderEmissionCol = false;

	float specularRoughAvg = 0.0f;
	bool isSliderSpecularRough = false;

	float refractionRoughAvg = 0.0f;
	bool isSliderRefractionRough = false;

	float refractionAmountAvg = 0.0f;
	bool isSliderRefractionAmount = false;

	float iorAvg = 0.0f;
	bool isSliderIor = false;

	float dispersionAvg = 0.0f;
	bool isSliderDispersion = false;

	float emissionGainAvg = 0.0f;
	bool isSliderEmissionGain = false;

	// Add the UI bools for optics in here
	std::vector<bool*> uiOpticElements = {

		&isSliderLightGain,
		&isSliderlightSpread,
		&isSliderLightColor,
		&isSliderBaseColor,
		&isSliderSpecularColor,
		&isSliderRefractionCol,
		&isSliderEmissionCol,
		&isSliderSpecularRough,
		&isSliderRefractionRough,
		&isSliderRefractionAmount,
		&isSliderIor,
		&isSliderDispersion,
		&isSliderEmissionGain

	};

	glm::vec2 boxInitialPos{ 0.0f, 0.0f };

	bool isBoxSelecting = false;
	bool isBoxDeselecting = false;

	float boxX = 0.0f;
	float boxY = 0.0f;
	float boxWidth = 0.0f;
	float boxHeight = 0.0f;

	int selectedWalls = 0;
	int selectedLights = 0;

	void selectLogic(UpdateVariables& myVar, UpdateParameters& myParam);



	float checkIntersect(const LightRay& ray, const Wall& w);

	void processRayIntersection(LightRay& ray);

	glm::vec2 rotateVec2(glm::vec2 v, float angle) {
		float c = cos(angle);
		float s = sin(angle);
		return glm::vec2(
			v.x * c - v.y * s,
			v.x * s + v.y * c
		);
	}

	void specularReflection(int& currentBounce, LightRay& ray, std::vector<LightRay>& copyRays, std::vector<Wall>& walls);

	void refraction(int& currentBounce, LightRay& ray, std::vector<LightRay>& copyRays, std::vector<Wall>& walls);

	// Currently unfinished and unused. Might work on it some time in the future
	//void volumeScatter(int& currentBounce, LightRay& ray, std::vector<LightRay>& copyRays, std::vector<Wall>& walls);

	void diffuseLighting(int& currentBounce, LightRay& ray, std::vector<LightRay>& copyRays, std::vector<Wall>& walls);

	void emission();

	void lightRendering(UpdateParameters& myParam);

	void drawRays() {

		if (currentSamples <= maxSamples) {
			for (LightRay& ray : rays) {
				ray.drawRay();
			}
		}
	}

	void drawScene() {
		for (Wall& wall : walls) {
			wall.drawWall();

			if (drawNormals) {
				DrawLineV({ wall.vA.x, wall.vA.y }, { wall.vA.x + wall.normalVA.x * 10.0f, wall.vA.y + wall.normalVA.y * 10.0f }, RED);
				DrawLineV({ wall.vB.x, wall.vB.y }, { wall.vB.x + wall.normalVB.x * 10.0f, wall.vB.y + wall.normalVB.y * 10.0f }, RED);
				DrawLineV({ (wall.vA.x + wall.vB.x) * 0.5f, (wall.vA.y + wall.vB.y) * 0.5f },
					{ (wall.vA.x + wall.vB.x) * 0.5f + wall.normal.x * 10.0f, (wall.vA.y + wall.vB.y) * 0.5f + wall.normal.y * 10.0f }, RED);
			}
		}

		for (AreaLight& areaLight : areaLights) {
			areaLight.drawAreaLight();
		}
	}

	void drawMisc(UpdateVariables& myVar, UpdateParameters& myParam);

	inline Color glmVec4ToColor(const glm::vec4& v) {
		float r = glm::clamp(v.x, 0.0f, 1.0f);
		float g = glm::clamp(v.y, 0.0f, 1.0f);
		float b = glm::clamp(v.z, 0.0f, 1.0f);
		float a = glm::clamp(v.w, 0.0f, 1.0f);

		return Color{
			static_cast<unsigned char>(r * 255.0f),
			static_cast<unsigned char>(g * 255.0f),
			static_cast<unsigned char>(b * 255.0f),
			static_cast<unsigned char>(a * 255.0f)
		};
	}

	inline glm::vec4 ColorToGlmVec4(const Color& c) {
		return glm::vec4{
			static_cast<float>(c.r) / 255.0f,
			static_cast<float>(c.g) / 255.0f,
			static_cast<float>(c.b) / 255.0f,
			static_cast<float>(c.a) / 255.0f
		};
	}

	void processApparentColor() {

		for (Wall& w : walls) {
			if (w.isSelected) {
				continue;
			}

			float emissionGain = static_cast<float>(w.emissionColor.a) / 255.0f;

			float baseWeight = 1.0f - w.refractionAmount;
			float specularWeight = 0.2f * w.IOR * (1.0f - w.refractionAmount * 0.5f);

			glm::vec4 baseCol = { 0.0f, 0.0f, 0.0f, 0.0f };
			baseCol += ColorToGlmVec4(w.baseColor) * baseWeight;
			baseCol += ColorToGlmVec4(w.specularColor) * specularWeight;
			baseCol += ColorToGlmVec4(w.refractionColor) * w.refractionAmount * 0.6f;

			float totalWeight = baseWeight + specularWeight + w.refractionAmount;
			if (totalWeight > 0.0f) {
				baseCol /= totalWeight;
			}

			glm::vec4 emissionVec = ColorToGlmVec4(w.emissionColor);
			glm::vec4 finalCol = glm::mix(baseCol, emissionVec, emissionGain);

			w.apparentColor = glmVec4ToColor(finalCol);
		}

		for (AreaLight& al : areaLights) {
			al.apparentColor = al.color;
		}

	}

	int accumulatedRays = 0;

	int totalLights = 0;

	void rayLogic(UpdateVariables& myVar, UpdateParameters& myParam) {

		if (shouldRender) {
			rays.clear();
			currentSamples = 0;

			accumulatedRays = 0;
			shouldRender = false;
		}

		if (IO::shortcutPress(KEY_C)) {
			shouldRender = true;
		}

		createPointLight(myVar, myParam);
		createAreaLight(myVar, myParam);
		createConeLight(myVar, myParam);
		createWall(myVar, myParam);
		createShape(myVar, myParam);

		if (!walls.empty()) {
			wallPointers.clear();
			for (Wall& wall : walls) {
				wallPointers.push_back(&wall);
			}
			bvh.build(wallPointers);
		}

		moveLogic(myVar, myParam);

		eraseLogic(myVar, myParam);

		selectLogic(myVar, myParam);

		lightRendering(myParam);

		drawRays();

		totalLights = static_cast<int>(pointLights.size()) + static_cast<int>(areaLights.size()) + static_cast<int>(coneLights.size());

		if (currentSamples <= maxSamples) {
			accumulatedRays += static_cast<int>(rays.size());
		}

		if (IO::shortcutPress(KEY_C)) {
			rays.clear();
			pointLights.clear();
			areaLights.clear();
			coneLights.clear();
			walls.clear();
			shapes.clear();

			wallPointers.clear();
			for (Wall& wall : walls) {
				wallPointers.push_back(&wall);
			}
			bvh.build(wallPointers);
		}
	}
};