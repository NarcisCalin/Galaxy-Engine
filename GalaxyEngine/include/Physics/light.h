#pragma once

#include "UX/randNum.h"

extern uint32_t globalWallId;

struct Wall {

	glm::vec2 vA;
	glm::vec2 vB;

	glm::vec2 normal;
	glm::vec2 normalVA;
	glm::vec2 normalVB;

	bool isBeingSpawned;
	bool vAisBeingMoved;
	bool vBisBeingMoved;

	Color aparentColor; // This is the color used to visualize the wall

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
	float dispersionStrength;

	bool isShapeWall;
	bool isShapeClosed;
	uint32_t shapeId;

	uint32_t id;

	bool isSelected;

	Wall() = default;
	Wall(glm::vec2 vA, glm::vec2 vB, bool isShapeWall, Color baseColor, Color specularColor, Color refractionColor,
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

		this->aparentColor = WHITE;

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
		DrawLineV({ vA.x, vA.y }, { vB.x, vB.y }, aparentColor);
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

	Shape(ShapeType shapeType, glm::vec2 h1, glm::vec2 h2, std::vector<Wall>* walls, Color baseColor, Color specularColor, Color refractionColor,
		float specularRoughness, float refractionRoughness, float refractionAmount, float IOR, float dispersionStrength) :

		shapeType(shapeType),
		h1(h1),
		h2(h2),
		walls(walls),
		baseColor(baseColor),
		specularColor(specularColor),
		refractionColor(refractionColor),
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

	void relaxGeometryLogic(std::vector<glm::vec2>& vertices, int iterations = 15, float relaxationFactor = 0.65f) {
		if (vertices.size() < 3) return;

		std::vector<glm::vec2> temp = vertices;

		for (int iter = 0; iter < iterations; ++iter) {
			for (size_t i = 0; i < vertices.size(); ++i) {
				size_t prev = (i - 1 + vertices.size()) % vertices.size();
				size_t next = (i + 1) % vertices.size();

				glm::vec2 average = (temp[prev] + temp[next]) * 0.5f;

				vertices[i] = glm::mix(temp[i], average, relaxationFactor);
			}
			temp = vertices;
		}
	}

	void relaxShape() {

		polygonVerts.clear();

		for (uint32_t wallId : myWallIds) {
			Wall* w = getWallById(*walls, wallId);
			if (w) {
				polygonVerts.push_back(w->vA);
			}
		}

		relaxGeometryLogic(polygonVerts);

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

			if(!isBeingSpawned){
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
				(*walls).emplace_back(vA, vB, true, baseColor, specularColor, refractionColor, specularRoughness, refractionRoughness, refractionAmount, IOR, dispersionStrength);

				(*walls).back().shapeId = id;
				(*walls).back().isShapeClosed = true;

				newWallIds.push_back((*walls).back().id);
			}
			
			if (isBeingMoved) {
				if (i < myWallIds.size()) {
					uint32_t wId = myWallIds[i];
					Wall* w = getWallById(*walls, wId);

					if (w) {
						w->vA = vA;
						w->vB = vB;
					}
				}
			}
		}

		if (createShapeFlag) {
			myWallIds = std::move(newWallIds);

			calculateWallsNormals();

			createShapeFlag = false;
		}
	}

	glm::vec2 prevPoint = h1;

	glm::vec2 oldHelperPos{ 0.0f, 0.0f };

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

				(*walls).emplace_back(prevPoint, nextPoint, true, baseColor, specularColor, refractionColor,
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

			oldHelperPos = helpers[0];
		}

		if (isBeingMoved) {
			glm::vec2 delta = helpers[0] - oldHelperPos;

			

			if (delta != glm::vec2(0.0f)) {

				

				for (auto& wallId : myWallIds) {
					Wall* w = getWallById(*walls, wallId);

					if (w) {
						w->vA += delta;
						w->vB += delta;
					}
				}

				oldHelperPos = helpers[0];
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
						(*walls).emplace_back(helpers.at(0), helpers.at(1), true, baseColor, specularColor, refractionColor, specularRoughness, refractionRoughness, refractionAmount, IOR, dispersionStrength);
						(*walls).back().shapeId = id;
						(*walls).back().isShapeClosed = true;
						newWallIds.push_back((*walls).back().id);
						wallAId = (*walls).back().id;
					}

					(*walls).emplace_back(helpers.at(1), helpers.at(2), true, baseColor, specularColor, refractionColor, specularRoughness, refractionRoughness, refractionAmount, IOR, dispersionStrength);
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
						(*walls).emplace_back(arcP1, arcP2, true, baseColor, specularColor, refractionColor, specularRoughness, refractionRoughness, refractionAmount, IOR, dispersionStrength);
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
					(*walls).emplace_back(arcEnd, helpers.at(0), true, baseColor, specularColor, refractionColor, specularRoughness, refractionRoughness, refractionAmount, IOR, dispersionStrength);

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
							(*walls).emplace_back(arcP1Symmetry, arcP2Symmetry, true, baseColor, specularColor, refractionColor, specularRoughness, refractionRoughness, refractionAmount, IOR, dispersionStrength);
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
	}

	void drawRay() {
		DrawLineV({ source.x, source.y }, { source.x + (dir.x * length), source.y + (dir.y * length) }, color);
	}
};

struct PointLight {

	glm::vec2 pos;
	bool isBeingMoved;

	Color color;

	Color aparentColor;

	bool isSelected;

	PointLight(glm::vec2 pos, Color color) {
		this->pos = pos;
		this->isBeingMoved = false;
		this->color = color;

		this->aparentColor = WHITE;

		this->isSelected = false;
	}

	void drawHelper(glm::vec2& helper) {
		DrawCircleV({ helper.x, helper.y }, 25.0f, aparentColor);
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

	Color aparentColor;

	bool isSelected;

	AreaLight() = default;
	AreaLight(glm::vec2 vA, glm::vec2 vB, Color color) {
		this->vA = vA;
		this->vB = vB;
		this->isBeingSpawned = true;
		this->vAisBeingMoved = false;
		this->vBisBeingMoved = false;
		this->color = color;
		this->aparentColor = WHITE;

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
		DrawLineV({ vA.x, vA.y }, { vB.x, vB.y }, aparentColor);
	}

	void areaLightLogic(UpdateParameters& myParam, int& sampleRaysAmount, std::vector<LightRay>& rays) {

		glm::vec2 mouseWorldPos = myParam.myCamera.mouseWorldPos;

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
		glm::vec2 invDir = 1.0f / dir;
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

	void build(std::vector<Wall*>& walls) {
		if (walls.empty()) {
			root = nullptr;
			return;
		}
		root = buildBVH(walls, 0, walls.size());
	}

	bool traverse(const LightRay& ray, float& closestT, Wall*& hitWall, glm::vec2& hitPoint) {
		if (!root) return false;
		return traverseBVH(root, ray, closestT, hitWall, hitPoint);
	}

private:
	BVHNode* buildBVH(std::vector<Wall*>& walls, int start, int end) {
		BVHNode* node = new BVHNode();

		AABB2D bounds = AABB2D(walls[start]->vA, walls[start]->vB);
		for (int i = start + 1; i < end; ++i) {
			AABB2D wallBox(walls[i]->vA, walls[i]->vB);
			bounds.expand(wallBox);
		}
		node->bounds = bounds;

		int count = end - start;
		if (count == 1) {
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
		node->left = buildBVH(walls, start, mid);
		node->right = buildBVH(walls, mid, end);
		return node;
	}

	bool intersectWall(const LightRay& ray, const Wall& wall, float& t, glm::vec2& hitPoint) {
		glm::vec2 p = ray.source;
		glm::vec2 r = ray.dir;
		glm::vec2 q = wall.vA;
		glm::vec2 s = wall.vB - wall.vA;

		float rxs = r.x * s.y - r.y * s.x;
		if (rxs == 0.0f) return false;

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

	bool traverseBVH(const BVHNode* node, const LightRay& ray, float& closestT, Wall*& hitWall, glm::vec2& hitPoint) {
		if (!node->bounds.intersectsRay(ray.source, ray.dir, closestT)) return false;

		bool hit = false;

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
			hit |= traverseBVH(node->left, ray, closestT, hitWall, hitPoint);
			hit |= traverseBVH(node->right, ray, closestT, hitWall, hitPoint);
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

	size_t firstPassTotalRays = 0;

	BVH bvh;

	int sampleRaysAmount = 1024;
	int maxBounces = 3;
	int maxSamples = 500;
	int currentSamples = 0;

	bool isDiffuseEnabled = true;
	bool isSpecularEnabled = true;
	bool isRefractionEnabled = true;
	bool isDispersionEnabled = false;

	bool symmetricalLens = false;

	bool shouldRender = true;

	bool drawNormals = false;

	const float lightBias = 0.1f;

	Color lightColor = { 255, 255, 255, 64 };

	Color wallBaseColor = { 200, 200, 200, 255 };
	Color wallSpecularColor = { 255, 255, 255, 255 };
	Color wallRefractionColor = { 255, 255, 255, 255 };

	float wallSpecularRoughness = 0.5f;
	float wallRefractionRoughness = 0.0f; // This controls the roughness of the refraction surface. I separate it for extra control, like V-Ray renderer
	float wallRefractionAmount = 0.0f;
	float wallIOR = 1.5f;
	float airIOR = 1.0f;

	float wallDispersion = 0.0f;

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

	void createWall(UpdateVariables& myVar, UpdateParameters& myParam) {

		glm::vec2 mouseWorldPos = myParam.myCamera.mouseWorldPos;

		if (IO::mousePress(0) && myVar.toolWall) {
			walls.emplace_back(mouseWorldPos, mouseWorldPos, false, wallBaseColor, wallSpecularColor, wallRefractionColor,
				wallSpecularRoughness, wallRefractionRoughness, wallRefractionAmount, wallIOR, wallDispersion);
		}

		if (IO::mouseDown(0) && myVar.toolWall) {
			if (walls.back().isBeingSpawned) {
				walls.back().vB = mouseWorldPos;

				calculateWallNormal(walls.back());

				shouldRender = true;
			}
		}

		if (IO::mouseReleased(0) && myVar.toolWall) {
			if (!walls.empty() && walls.back().isBeingSpawned) {
				if (glm::length(walls.back().vB - walls.back().vA) == 0.0f) {
					walls.pop_back();
					return;
				}

				walls.back().isBeingSpawned = false;
			}
		}
	}

	bool firstHelper = true;
	bool isCreatingLens = false;

	float minHelperLength = FLT_MAX;
	int selectedHelper = -1;
	size_t selectedShape = -1;

	const float helperMinDist = 100.0f;

	void createShape(UpdateVariables& myVar, UpdateParameters& myParam) {

		glm::vec2 mouseWorldPos = myParam.myCamera.mouseWorldPos;

		// ---- Circle ---- //
		if (IO::mousePress(0) && myVar.toolCircle) {
			shapes.emplace_back(circle, mouseWorldPos, mouseWorldPos, &walls, wallBaseColor, wallSpecularColor, wallRefractionColor,
				wallSpecularRoughness, wallRefractionRoughness, wallRefractionAmount, wallIOR, wallDispersion);
		}

		if (IO::mouseDown(0) && myVar.toolCircle) {
			if (shapes.back().isBeingSpawned) {
				shapes.back().h2 = mouseWorldPos;

				shapes.back().makeShape();

				shapes.back().calculateWallsNormals();
			}
		}

		if (IO::mouseReleased(0) && myVar.toolCircle) {
			if (!shapes.empty() && shapes.back().isBeingSpawned) {
				if (glm::length(shapes.back().h2 - shapes.back().h1) == 0.0f) {
					shapes.pop_back();
					return;
				}

				shapes.back().isBeingSpawned = false;

				shapes.back().helpers.push_back(shapes.back().h1);
				shapes.back().helpers.push_back(shapes.back().h2);


				shapes.back().createShapeFlag = true;
				shapes.back().makeShape();
			}
			shouldRender = true;
		}

		// ---- Draw Shape ---- //
		if (IO::mousePress(0) && myVar.toolDrawShape) {
			shapes.emplace_back(draw, mouseWorldPos, mouseWorldPos, &walls, wallBaseColor, wallSpecularColor, wallRefractionColor,
				wallSpecularRoughness, wallRefractionRoughness, wallRefractionAmount, wallIOR, wallDispersion);

			shapes.back().helpers.push_back(mouseWorldPos);
		}

		if (IO::mouseDown(0) && myVar.toolDrawShape) {
			if (shapes.back().isBeingSpawned) {
				shapes.back().h2 = mouseWorldPos;

				shapes.back().makeShape();

				shapes.back().calculateWallsNormals();
			}

			shouldRender = true;
		}

		if (IO::mouseReleased(0) && myVar.toolDrawShape) {
			if (!shapes.empty() && shapes.back().isBeingSpawned) {
				if (glm::length(shapes.back().h2 - shapes.back().h1) == 0.0f) {
					shapes.pop_back();
					return;
				}

				shapes.back().isBeingSpawned = false;

				shapes.back().makeShape();

				const Wall* lastWall = getWallById(walls, shapes.back().myWallIds.back());
				const Wall* firstWall = getWallById(walls, shapes.back().myWallIds.front());

				if (!lastWall || !firstWall) {
					return;
				}

				const glm::vec2& lastPoint = lastWall->vB;
				const glm::vec2& firstPoint = firstWall->vA;

				walls.emplace_back(lastPoint, firstPoint, true, wallBaseColor, wallSpecularColor, wallRefractionColor,
					wallSpecularRoughness, wallRefractionRoughness, wallRefractionAmount, wallIOR, wallDispersion);

				walls.back().shapeId = shapes.back().id;
				shapes.back().myWallIds.push_back(walls.back().id);

				shapes.back().relaxShape();

				shapes.back().calculateWallsNormals();
			}
			shouldRender = true;
		}

		// ---- Lens ---- //
		if (IO::mousePress(0) && myVar.toolLens && firstHelper) {
			shapes.emplace_back(lens, mouseWorldPos, mouseWorldPos, &walls, wallBaseColor, wallSpecularColor, wallRefractionColor,
				wallSpecularRoughness, wallRefractionRoughness, wallRefractionAmount, wallIOR, wallDispersion);

			shapes.back().symmetricalLens = symmetricalLens;

			isCreatingLens = true;
		}
		else if (IO::mousePress(0) && myVar.toolLens) {

			if (shapes.back().helpers.size() == 2) {
				shapes.back().thirdHelper = true;
			}

			if (shapes.back().helpers.size() == 3) {
				shapes.back().fourthHelper = true;
				firstHelper = true;
			}
		}

		if (IO::mouseDown(0) && myVar.toolLens && firstHelper) {
			if (shapes.back().isBeingSpawned) {
				shapes.back().h2 = mouseWorldPos;
			}
		}
		else if (isCreatingLens) {
			if (shapes.back().isBeingSpawned) {
				shapes.back().h2 = mouseWorldPos;
			}
		}

		if (IO::mouseReleased(0) && myVar.toolLens) {
			if (!shapes.empty() && shapes.back().isBeingSpawned) {
				if (glm::length(shapes.back().h2 - shapes.back().h1) == 0.0f) {
					shapes.pop_back();
					return;
				}

				if (shapes.back().helpers.size() == 1) {
					shapes.back().secondHelper = true;
				}

				firstHelper = false;
			}
			if (!shapes.back().isBeingSpawned) {
				shouldRender = true;
			}
		}

		if (!shapes.empty()) {
			if (shapes.back().isBeingSpawned && shapes.back().shapeType == lens) {
				shapes.back().makeShape();
			}
		}
		else {
			firstHelper = true;
		}
	}

	void createPointLight(UpdateVariables& myVar, UpdateParameters& myParam) {

		glm::vec2 mouseWorldPos = myParam.myCamera.mouseWorldPos;

		if (IO::mousePress(0) && myVar.toolPointLight) {
			pointLights.emplace_back(mouseWorldPos, lightColor);

			shouldRender = true;
		}
	}

	void createAreaLight(UpdateVariables& myVar, UpdateParameters& myParam) {

		glm::vec2 mouseWorldPos = myParam.myCamera.mouseWorldPos;

		if (IO::mousePress(0) && myVar.toolAreaLight) {
			areaLights.emplace_back(mouseWorldPos, mouseWorldPos, lightColor);
		}

		if (IO::mouseDown(0) && myVar.toolAreaLight) {
			if (areaLights.back().isBeingSpawned) {
				areaLights.back().vB = mouseWorldPos;

				shouldRender = true;
			}
		}

		if (IO::mouseReleased(0) && myVar.toolAreaLight) {
			if (!areaLights.empty() && areaLights.back().isBeingSpawned) {
				if (glm::length(areaLights.back().vB - areaLights.back().vA) == 0.0f) {
					areaLights.pop_back();
					return;
				}

				areaLights.back().isBeingSpawned = false;
			}
		}
	}

	void movePointLights(UpdateVariables& myVar, UpdateParameters& myParam) {

		if (IO::mousePress(0) && myVar.toolMoveOptics) {
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
		}

		for (PointLight& pointLight : pointLights) {

			glm::vec2 mouseDelta = glm::vec2(GetMouseDelta().x, GetMouseDelta().y);
			glm::vec2 scaledDelta = mouseDelta * (1.0f / myParam.myCamera.camera.zoom);

			if (pointLight.isBeingMoved) {
				pointLight.pos += scaledDelta;

				shouldRender = true;
			}
		}

		if (IO::mouseReleased(0) && myVar.toolMoveOptics) {
			for (PointLight& pointLight : pointLights) {
				pointLight.isBeingMoved = false;
			}
		}
	}

	void moveAreaLights(UpdateVariables& myVar, UpdateParameters& myParam) {
		glm::vec2 mouseWorldPos = myParam.myCamera.mouseWorldPos;

		glm::vec2 mouseDelta = glm::vec2(GetMouseDelta().x, GetMouseDelta().y);
		glm::vec2 scaledDelta = mouseDelta * (1.0f / myParam.myCamera.camera.zoom);

		if (IO::mousePress(0) && myVar.toolMoveOptics) {
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
		}

		if (IO::mouseDown(0) && myVar.toolMoveOptics) {
			for (AreaLight& areaLight : areaLights) {
				if (areaLight.vAisBeingMoved) {
					areaLight.vA += scaledDelta;

					shouldRender = true;
				}
				if (areaLight.vBisBeingMoved) {
					areaLight.vB += scaledDelta;

					shouldRender = true;
				}
			}
		}

		if (IO::mouseReleased(0) && myVar.toolMoveOptics) {
			for (AreaLight& areaLight : areaLights) {
				areaLight.vAisBeingMoved = false;
				areaLight.vBisBeingMoved = false;
			}
		}
	}

	void moveWalls(UpdateVariables& myVar, UpdateParameters& myParam) {
		glm::vec2 mouseWorldPos = myParam.myCamera.mouseWorldPos;

		glm::vec2 mouseDelta = glm::vec2(GetMouseDelta().x, GetMouseDelta().y);
		glm::vec2 scaledDelta = mouseDelta * (1.0f / myParam.myCamera.camera.zoom);

		if (IO::mousePress(0) && myVar.toolMoveOptics) {
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
		}

		if (IO::mouseDown(0) && myVar.toolMoveOptics) {
			for (Wall& wall : walls) {
				if (wall.vAisBeingMoved) {
					wall.vA += scaledDelta;
					shouldRender = true;
				}
				if (wall.vBisBeingMoved) {
					wall.vB += scaledDelta;
					shouldRender = true;
				}

				calculateWallNormal(wall);
			}

			for (Shape& shape : shapes) {
				shape.calculateWallsNormals();
			}
		}

		if (IO::mouseReleased(0) && myVar.toolMoveOptics) {
			for (Wall& wall : walls) {
				wall.vAisBeingMoved = false;
				wall.vBisBeingMoved = false;
			}
		}
	}

	bool isAnyShapeBeingSpawned = false;

	void moveLogic(UpdateVariables& myVar, UpdateParameters& myParam) {

		glm::vec2 mouseWorldPos = myParam.myCamera.mouseWorldPos;



		minHelperLength = FLT_MAX;

		if (!shapes.empty()) {
			for (size_t i = 0; i < shapes.size(); i++) {
				shapes[i].drawHoverHelpers = false;

				for (size_t j = 0; j < shapes[i].helpers.size(); j++) {
					float helperDist = glm::length(mouseWorldPos - shapes[i].helpers[j]);

					if (shapes[i].shapeType == circle) {
						float helperCircleDist = glm::length(mouseWorldPos - shapes[i].helpers[0]);

						if (helperCircleDist <= shapes[i].circleRadius && !shapes[i].isBeingSpawned) {
							shapes[i].drawHoverHelpers = true;
						}
					}

					if (helperDist <= helperMinDist) {
						if (!shapes[i].isBeingSpawned) {
							shapes[i].drawHoverHelpers = true;
						}

						if (helperDist < minHelperLength) {
							minHelperLength = helperDist;

							if (IO::mousePress(0) && myVar.toolMoveOptics) {
								selectedShape = i;
								selectedHelper = j;
							}
						}
					}
				}
			}
		}

		if (selectedHelper == -1 && selectedShape == -1 && !isAnyShapeBeingSpawned) {
			movePointLights(myVar, myParam);
			moveAreaLights(myVar, myParam);
			moveWalls(myVar, myParam);

			return; // We are not moving helpers, so get out of the function
		}

		if (IO::mouseDown(0) && myVar.toolMoveOptics) {

			for (Shape& shape : shapes) {
				if (shape.isBeingSpawned) {
					isAnyShapeBeingSpawned = true;
					break;
				}
			}

			if (selectedHelper != -1 && selectedShape != -1 && !isAnyShapeBeingSpawned) {

				shapes.at(selectedShape).isBeingMoved = true;

				if (selectedHelper != 2 || selectedHelper != 3) {

					glm::vec2 oldCenter = shapes.at(selectedShape).helpers[0];
					glm::vec2 oldEdge = shapes.at(selectedShape).helpers[1];

					glm::vec2 radiusVec = oldEdge - oldCenter;
					float radius = glm::length(radiusVec);
					glm::vec2 radiusDir = glm::normalize(radiusVec);

					shapes.at(selectedShape).helpers.at(selectedHelper) = mouseWorldPos;

					if (shapes.at(selectedShape).shapeType == circle) {
						if (selectedHelper == 0) {

							shapes.at(selectedShape).helpers[1] = mouseWorldPos + radiusDir * radius;
						}
					}
				}

				if (selectedHelper == 2) {
					shapes.at(selectedShape).isThirdBeingMoved = true;
					shapes.at(selectedShape).moveH2 = mouseWorldPos;
				}

				if (selectedHelper == 3) {
					shapes.at(selectedShape).isFourthBeingMoved = true;
					shapes.at(selectedShape).moveH2 = mouseWorldPos;
				}

				if (shapes[selectedShape].symmetricalLens) {
					if (selectedHelper == 4) {
						shapes.at(selectedShape).isFifthBeingMoved = true;
						shapes.at(selectedShape).moveH2 = mouseWorldPos;
					}
				}

				if (shapes[selectedShape].symmetricalLens) {
					if ((selectedHelper == 3 || selectedHelper == 4) && IO::shortcutDown(KEY_RIGHT_SHIFT)) {
						shapes.at(selectedShape).isFifthFourthMoved = true;
						shapes.at(selectedShape).moveH2 = mouseWorldPos;
					}
				}


				shapes.at(selectedShape).makeShape();
			}

			shouldRender = true;
		}

		if (IO::mouseReleased(0) && myVar.toolMoveOptics) {

			if (selectedHelper != -1 && selectedShape != -1 && !isAnyShapeBeingSpawned) {
				shapes.at(selectedShape).isBeingMoved = false;

				shapes.at(selectedShape).isThirdBeingMoved = false;
				shapes.at(selectedShape).isFourthBeingMoved = false;
				shapes.at(selectedShape).isFifthBeingMoved = false;
				shapes.at(selectedShape).isFifthFourthMoved = false;
			}

			shouldRender = true;

			minHelperLength = FLT_MAX;
			selectedShape = -1;
			selectedHelper = -1;
		}
	}

	void eraseLogic(UpdateVariables& myVar, UpdateParameters& myParam) {

		bool anySelectedWalls = false;
		bool anySelectedLights = false;

		for (Wall& wall : walls) {
			if (wall.isSelected) {
				anySelectedWalls = true;
				break;
			}
		}

		for (PointLight& p : pointLights) {
			if (p.isSelected) {
				anySelectedLights = true;
				break;
			}
		}

		for (AreaLight& a : areaLights) {
			if (a.isSelected) {
				anySelectedLights = true;
				break;
			}
		}

		glm::vec2 mouseWorldPos = myParam.myCamera.mouseWorldPos;

		if (IO::mouseDown(0) && myVar.toolEraseOptics) {
			for (int i = static_cast<int>(walls.size()) - 1; i >= 0; --i) {

				Wall& wall = walls[i];

				glm::vec2 dA = wall.vA - mouseWorldPos;
				glm::vec2 dB = wall.vB - mouseWorldPos;

				float distA = glm::length(dA);
				float distB = glm::length(dB);

				if (distA <= myParam.brush.brushRadius || distB <= myParam.brush.brushRadius) {

					if (wall.isShapeWall) {
						for (size_t shapeIdx = 0; shapeIdx < shapes.size(); shapeIdx++) {

							Shape& shape = shapes[shapeIdx];

							if (shape.id == wall.shapeId) {
								std::vector<uint32_t>& myIds = shape.myWallIds;

								uint32_t wallId = wall.id;
								myIds.erase(std::remove(myIds.begin(), myIds.end(), wallId), myIds.end());

								shape.isShapeClosed = false;

								for (uint32_t id : myIds) {
									Wall* shapeWall = getWallById(*shape.walls, id);
									if (shapeWall) {
										shapeWall->isShapeClosed = false;
									}
								}
							}
						}
					}

					walls.erase(walls.begin() + i);

					shouldRender = true;
				}
			}

			for (int i = static_cast<int>(pointLights.size()) - 1; i >= 0; --i) {

				PointLight& pointLight = pointLights[i];

				glm::vec2 d = pointLight.pos - mouseWorldPos;

				float dist = glm::length(d);

				if (dist <= myParam.brush.brushRadius) {

					pointLights.erase(pointLights.begin() + i);

					shouldRender = true;
				}
			}

			for (int i = static_cast<int>(areaLights.size()) - 1; i >= 0; --i) {

				AreaLight& areaLight = areaLights[i];

				glm::vec2 dA = areaLight.vA - mouseWorldPos;
				glm::vec2 dB = areaLight.vB - mouseWorldPos;

				float distA = glm::length(dA);
				float distB = glm::length(dB);

				if (distA <= myParam.brush.brushRadius || distB <= myParam.brush.brushRadius) {

					areaLights.erase(areaLights.begin() + i);

					shouldRender = true;
				}
			}
		}

		if (IO::shortcutPress(KEY_DELETE)) {
			for (int i = static_cast<int>(walls.size()) - 1; i >= 0; --i) {
				Wall& wall = walls[i];

				if (wall.isSelected) {
					for (size_t shapeIdx = 0; shapeIdx < shapes.size(); shapeIdx++) {

							Shape& shape = shapes[shapeIdx];

							if (shape.id == wall.shapeId) {
								std::vector<uint32_t>& myIds = shape.myWallIds;

								uint32_t wallId = wall.id;
								myIds.erase(std::remove(myIds.begin(), myIds.end(), wallId), myIds.end());

								shape.isShapeClosed = false;

								for (uint32_t id : myIds) {
									Wall* shapeWall = getWallById(*shape.walls, id);
									if (shapeWall) {
										shapeWall->isShapeClosed = false;
									}
								}
							}
						}

					walls.erase(walls.begin() + i);

				}
			}

			for (int i = static_cast<int>(pointLights.size()) - 1; i >= 0; --i) {
				PointLight& pointLight = pointLights[i];

				if (pointLight.isSelected) {
					pointLights.erase(pointLights.begin() + i);
				}
			}

			for (int i = static_cast<int>(areaLights.size()) - 1; i >= 0; --i) {
				AreaLight& areaLight = areaLights[i];

				if (areaLight.isSelected) {
					areaLights.erase(areaLights.begin() + i);
				}
			}

			for (int i = static_cast<int>(shapes.size()) - 1; i >= 0; --i) {
				if (shapes[i].myWallIds.empty()) {
					shapes.erase(shapes.begin() + i);
				}
			}
		}


		if ((IO::mouseReleased(0) && myVar.toolEraseOptics) || ((anySelectedWalls || anySelectedLights) && IO::shortcutPress(KEY_DELETE))) {
			shouldRender = true;
		}
	}

	Color lightColorAvg = { 0, 0, 0, 0 };
	bool isSliderLightColor = false;

	Color baseColorAvg = { 0, 0, 0, 0 };
	bool isSliderBaseColor = false;

	Color specularColorAvg = { 0, 0, 0, 0 };
	bool isSliderSpecularColor = false;

	Color refractionColorAvg = { 0, 0, 0, 0 };
	bool isSliderRefractionCol = false;

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

	// Add the UI bools for optics in here
	std::vector<bool*> uiOpticElements = {

		&isSliderLightColor,
		&isSliderBaseColor,
		&isSliderSpecularColor,
		&isSliderRefractionCol,
		&isSliderSpecularRough,
		&isSliderRefractionRough,
		&isSliderRefractionAmount,
		&isSliderIor,
		&isSliderDispersion

	};

	glm::vec2 boxInitialPos{ 0.0f, 0.0f };

	bool isBoxSelecting = false;
	bool isBoxDeselecting = false;

	// I'm also sorry for this large chunk of ugly code, but I really hate working on anything that involves UI or UX stuff (:

	float boxX = 0.0f;
	float boxY = 0.0f;
	float boxWidth = 0.0f;
	float boxHeight = 0.0f;

	void selectLogic(UpdateVariables& myVar, UpdateParameters& myParam) {

		int selectedWalls = 0;

		int selectedAreaLights = 0;

		int selectedPointLights = 0;

		bool isHoveringAnything = false;

		if (myVar.toolSelectOptics) {

			glm::vec2 mouseWorldPos = myParam.myCamera.mouseWorldPos;

			if (IO::mousePress(0)) {
				boxInitialPos = mouseWorldPos;
				isBoxSelecting = true;

				isBoxDeselecting = IO::shortcutDown(KEY_LEFT_ALT);
			}

			if (IO::mouseDown(0) && isBoxSelecting) {
				boxX = fmin(boxInitialPos.x, mouseWorldPos.x);
				boxY = fmin(boxInitialPos.y, mouseWorldPos.y);
				boxWidth = fabs(mouseWorldPos.x - boxInitialPos.x);
				boxHeight = fabs(mouseWorldPos.y - boxInitialPos.y);
			}

			if (IO::mouseReleased(0) && isBoxSelecting) {
				float boxX1 = fmin(boxInitialPos.x, mouseWorldPos.x);
				float boxX2 = fmax(boxInitialPos.x, mouseWorldPos.x);
				float boxY1 = fmin(boxInitialPos.y, mouseWorldPos.y);
				float boxY2 = fmax(boxInitialPos.y, mouseWorldPos.y);

				for (Wall& wall : walls) {
					bool vAInBox = wall.vA.x >= boxX1 && wall.vA.x <= boxX2 &&
						wall.vA.y >= boxY1 && wall.vA.y <= boxY2;

					bool vBInBox = wall.vB.x >= boxX1 && wall.vB.x <= boxX2 &&
						wall.vB.y >= boxY1 && wall.vB.y <= boxY2;

					if (vAInBox || vBInBox) {
						if (isBoxDeselecting && wall.isSelected) {
							wall.isSelected = false;
						}
						else if (!isBoxDeselecting) {
							wall.isSelected = true;
						}
					}
				}

				for (AreaLight& areaLight : areaLights) {
					bool vAInBox = areaLight.vA.x >= boxX1 && areaLight.vA.x <= boxX2 &&
						areaLight.vA.y >= boxY1 && areaLight.vA.y <= boxY2;

					bool vBInBox = areaLight.vB.x >= boxX1 && areaLight.vB.x <= boxX2 &&
						areaLight.vB.y >= boxY1 && areaLight.vB.y <= boxY2;

					if (vAInBox || vBInBox) {
						if (isBoxDeselecting && areaLight.isSelected) {
							areaLight.isSelected = false;
						}
						else if (!isBoxDeselecting) {
							areaLight.isSelected = true;
						}
					}
				}

				for (PointLight& pointLight : pointLights) {
					bool pointInBox = pointLight.pos.x >= boxX1 && pointLight.pos.x <= boxX2 &&
						pointLight.pos.y >= boxY1 && pointLight.pos.y <= boxY2;

					if (pointInBox) {
						if (isBoxDeselecting && pointLight.isSelected) {
							pointLight.isSelected = false;
						}
						else if (!isBoxDeselecting) {
							pointLight.isSelected = true;
						}
					}
				}

				boxX = 0.0f;
				boxY = 0.0f;
				boxWidth = 0.0f;
				boxHeight = 0.0f;

				isBoxSelecting = false;
				isBoxDeselecting = false;
			}

			for (Wall& wall : walls) {

				glm::vec2 wallLine = wall.vB - wall.vA;
				glm::vec2 mouseToA = mouseWorldPos - wall.vA;

				float lineLenSquared = glm::dot(wallLine, wallLine);

				if (lineLenSquared == 0.0f) {

					float dist = glm::length(mouseToA);
					continue;
				}

				float t = glm::dot(mouseToA, wallLine) / lineLenSquared;
				t = glm::clamp(t, 0.0f, 1.0f);

				glm::vec2 closestPoint = wall.vA + wallLine * t;

				glm::vec2 diff = mouseWorldPos - closestPoint;
				float distance = glm::length(diff);

				if (distance < 5.0f) {

					isHoveringAnything = true;

					if (IO::mousePress(0)) {

						if (!IO::shortcutDown(KEY_LEFT_CONTROL) && !IO::shortcutDown(KEY_LEFT_ALT)) {
							for (Wall& wallToDeselect : walls) {
								wallToDeselect.isSelected = false;
							}

							for (AreaLight& areaLightToDeselect : areaLights) {
								areaLightToDeselect.isSelected = false;
							}

							for (PointLight& pointLight : pointLights) {
								pointLight.isSelected = false;
							}
						}

						if (!IO::shortcutDown(KEY_LEFT_ALT)) {
							wall.isSelected = true;
						}

						if (IO::shortcutDown(KEY_LEFT_ALT) && !IO::shortcutDown(KEY_LEFT_SHIFT) && wall.isShapeWall) {
							wall.isSelected = false;
						}

						if (wall.isShapeWall) {

							if (IO::shortcutDown(KEY_LEFT_SHIFT)) {
								for (Shape& shape : shapes) {
									if (shape.id == wall.shapeId) {
										for (uint32_t wallId : shape.myWallIds) {
											Wall* wall = getWallById(walls, wallId);
											if (wall) wall->isSelected = true;
										}
									}
								}
							}
						}

						if (IO::shortcutDown(KEY_LEFT_ALT) && IO::shortcutDown(KEY_LEFT_SHIFT) && wall.isShapeWall) {
							for (Shape& shape : shapes) {
								if (shape.id == wall.shapeId) {
									for (uint32_t wallId : shape.myWallIds) {
										Wall* wall = getWallById(walls, wallId);
										if (wall) wall->isSelected = true;
									}
								}
							}
						}
					}

					wall.aparentColor = RED;
				}
				else {

					if (!wall.isSelected) {
						wall.aparentColor = WHITE;
					}
				}
			}

			for (AreaLight& areaLight : areaLights) {

				glm::vec2 areaLightLine = areaLight.vB - areaLight.vA;
				glm::vec2 mouseToA = mouseWorldPos - areaLight.vA;

				float lineLenSquared = glm::dot(areaLightLine, areaLightLine);

				if (lineLenSquared == 0.0f) {

					float dist = glm::length(mouseToA);
					continue;
				}

				float t = glm::dot(mouseToA, areaLightLine) / lineLenSquared;
				t = glm::clamp(t, 0.0f, 1.0f);

				glm::vec2 closestPoint = areaLight.vA + areaLightLine * t;

				glm::vec2 diff = mouseWorldPos - closestPoint;
				float distance = glm::length(diff);

				if (distance < 5.0f) {

					isHoveringAnything = true;

					if (IO::mousePress(0)) {

						if (!IO::shortcutDown(KEY_LEFT_CONTROL) && !IO::shortcutDown(KEY_LEFT_ALT)) {
							for (AreaLight& areaLightToDeselect : areaLights) {
								areaLightToDeselect.isSelected = false;
							}

							for (PointLight& pointLight : pointLights) {
								pointLight.isSelected = false;
							}

							for (Wall& wall : walls) {
								wall.isSelected = false;
							}
						}

						if (!IO::shortcutDown(KEY_LEFT_ALT)) {
							areaLight.isSelected = true;
						}

						if (IO::shortcutDown(KEY_LEFT_ALT) && !IO::shortcutDown(KEY_LEFT_SHIFT)) {
							areaLight.isSelected = false;
						}
					}

					areaLight.aparentColor = RED;
				}
				else {

					if (!areaLight.isSelected) {
						areaLight.aparentColor = WHITE;
					}
				}
			}

			for (PointLight& pointLight : pointLights) {

				glm::vec2 mouseToA = mouseWorldPos - pointLight.pos;

				float distance = glm::length(mouseToA);

				if (distance < 5.0f) {

					isHoveringAnything = true;

					if (IO::mousePress(0)) {

						if (!IO::shortcutDown(KEY_LEFT_CONTROL) && !IO::shortcutDown(KEY_LEFT_ALT)) {

							for (PointLight& pointLight : pointLights) {
								pointLight.isSelected = false;
							}

							for (AreaLight& areaLightToDeselect : areaLights) {
								areaLightToDeselect.isSelected = false;
							}

							for (Wall& wall : walls) {
								wall.isSelected = false;
							}
						}

						if (!IO::shortcutDown(KEY_LEFT_ALT)) {
							pointLight.isSelected = true;
						}

						if (IO::shortcutDown(KEY_LEFT_ALT) && !IO::shortcutDown(KEY_LEFT_SHIFT)) {
							pointLight.isSelected = false;
						}
					}

					pointLight.aparentColor = RED;
				}
				else {

					if (!pointLight.isSelected) {
						pointLight.aparentColor = WHITE;
					}
				}
			}

			if (!isHoveringAnything && !IO::shortcutDown(KEY_LEFT_CONTROL) && !IO::shortcutDown(KEY_LEFT_ALT)) {
				if (IO::mousePress(0)) {

					for (Wall& wall : walls) {
						wall.isSelected = false;
					}

					for (AreaLight& areaLight : areaLights) {
						areaLight.isSelected = false;
					}

					for (PointLight& pointLight : pointLights) {
						pointLight.isSelected = false;
					}
				}
			}

			for (Wall& wall : walls) {
				if (wall.isSelected) {
					wall.aparentColor = RED;

					selectedWalls++;
				}
			}

			for (AreaLight& areaLight : areaLights) {
				if (areaLight.isSelected) {
					areaLight.aparentColor = RED;

					selectedAreaLights++;
				}
			}

			for (PointLight& pointLight : pointLights) {
				if (pointLight.isSelected) {
					pointLight.aparentColor = RED;

					pointLight.drawHelper(pointLight.pos);

					selectedPointLights++;
				}
			}

			if (IO::mouseReleased(0)) {

				if (selectedWalls > 0) {

					baseColorAvg = { 0, 0, 0, 0 };
					ImVec4 baseColorAvgImgui = { 0.0f, 0.0f, 0.0f, 0.0f };

					specularColorAvg = { 0, 0, 0, 0 };
					ImVec4 specularColorAvgImgui = { 0.0f, 0.0f, 0.0f, 0.0f };

					refractionColorAvg = { 0, 0, 0, 0 };
					ImVec4 refractionColAvgImgui = { 0.0f, 0.0f, 0.0f, 0.0f };

					specularRoughAvg = 0.0f;

					refractionRoughAvg = 0.0f;

					refractionAmountAvg = 0.0f;

					iorAvg = 0.0f;

					dispersionAvg = 0.0f;

					for (Wall& wall : walls) {
						if (wall.isSelected) {

							// Base Color
							ImVec4 wallBaseColImgui = rlImGuiColors::Convert(wall.baseColor);

							baseColorAvgImgui.x += wallBaseColImgui.x;
							baseColorAvgImgui.y += wallBaseColImgui.y;
							baseColorAvgImgui.z += wallBaseColImgui.z;
							baseColorAvgImgui.w += wallBaseColImgui.w;

							// Specular Color
							ImVec4 wallSpecularColImgui = rlImGuiColors::Convert(wall.specularColor);

							specularColorAvgImgui.x += wallSpecularColImgui.x;
							specularColorAvgImgui.y += wallSpecularColImgui.y;
							specularColorAvgImgui.z += wallSpecularColImgui.z;
							specularColorAvgImgui.w += wallSpecularColImgui.w;

							// Refraction Color
							ImVec4 wallRefractionColImgui = rlImGuiColors::Convert(wall.refractionColor);

							refractionColAvgImgui.x += wallRefractionColImgui.x;
							refractionColAvgImgui.y += wallRefractionColImgui.y;
							refractionColAvgImgui.z += wallRefractionColImgui.z;
							refractionColAvgImgui.w += wallRefractionColImgui.w;

							// Specular Roughness
							specularRoughAvg += wall.specularRoughness;

							// Refraction Surface Roughness
							refractionRoughAvg += wall.refractionRoughness;

							// Refraction Amount
							refractionAmountAvg += wall.refractionAmount;

							// IOR
							iorAvg += wall.IOR;

							// Dispersion
							dispersionAvg += wall.dispersionStrength;
						}
					}

					// Base Color
					baseColorAvgImgui.x /= selectedWalls;
					baseColorAvgImgui.y /= selectedWalls;
					baseColorAvgImgui.z /= selectedWalls;
					baseColorAvgImgui.w /= selectedWalls;
					wallBaseColor = rlImGuiColors::Convert(baseColorAvgImgui);

					// Specular Color
					specularColorAvgImgui.x /= selectedWalls;
					specularColorAvgImgui.y /= selectedWalls;
					specularColorAvgImgui.z /= selectedWalls;
					specularColorAvgImgui.w /= selectedWalls;
					wallSpecularColor = rlImGuiColors::Convert(specularColorAvgImgui);

					// Refraction Color
					refractionColAvgImgui.x /= selectedWalls;
					refractionColAvgImgui.y /= selectedWalls;
					refractionColAvgImgui.z /= selectedWalls;
					refractionColAvgImgui.w /= selectedWalls;
					wallRefractionColor = rlImGuiColors::Convert(refractionColAvgImgui);

					// Specular Roughness
					specularRoughAvg /= selectedWalls;
					wallSpecularRoughness = specularRoughAvg;

					// Refraction Surface Roughness
					refractionRoughAvg /= selectedWalls;
					wallRefractionRoughness = refractionRoughAvg;

					// Refraction Amount
					refractionAmountAvg /= selectedWalls;
					wallRefractionAmount = refractionAmountAvg;

					// IOR
					iorAvg /= selectedWalls;
					wallIOR = iorAvg;

					// Dispersion
					dispersionAvg /= selectedWalls;
					wallDispersion = dispersionAvg;
				}

				if (selectedAreaLights > 0 || selectedPointLights > 0) {

					lightColorAvg = { 0, 0, 0, 0 };
					ImVec4 lightColorAvgImgui = { 0.0f, 0.0f, 0.0f, 0.0f };

					if (selectedAreaLights > 0) {

						for (AreaLight& arealight : areaLights) {
							if (arealight.isSelected) {

								// Light Color
								ImVec4 lightColImgui = rlImGuiColors::Convert(arealight.color);

								lightColorAvgImgui.x += lightColImgui.x;
								lightColorAvgImgui.y += lightColImgui.y;
								lightColorAvgImgui.z += lightColImgui.z;
								lightColorAvgImgui.w += lightColImgui.w;
							}
						}
					}

					if (selectedPointLights > 0) {

						for (PointLight& pointLight : pointLights) {
							if (pointLight.isSelected) {

								// Light Color
								ImVec4 lightColImgui = rlImGuiColors::Convert(pointLight.color);

								lightColorAvgImgui.x += lightColImgui.x;
								lightColorAvgImgui.y += lightColImgui.y;
								lightColorAvgImgui.z += lightColImgui.z;
								lightColorAvgImgui.w += lightColImgui.w;
							}
						}
					}

					// Light Color
					lightColorAvgImgui.x /= selectedAreaLights + selectedPointLights;
					lightColorAvgImgui.y /= selectedAreaLights + selectedPointLights;
					lightColorAvgImgui.z /= selectedAreaLights + selectedPointLights;
					lightColorAvgImgui.w /= selectedAreaLights + selectedPointLights;
					lightColor = rlImGuiColors::Convert(lightColorAvgImgui);
				}
			}
		}

		if (IO::mouseReleased(0) && !myVar.toolSelectOptics) {
			for (Wall& wall : walls) {
				wall.isSelected = false;
				wall.aparentColor = WHITE;
			}

			for (AreaLight& areaLight : areaLights) {
				areaLight.isSelected = false;
				areaLight.aparentColor = WHITE;
			}

			for (PointLight& pointLight : pointLights) {
				pointLight.isSelected = false;
				pointLight.aparentColor = WHITE;
			}
		}

		bool isAnyActive = false;

		for (bool* param : uiOpticElements) {
			if (*param) {
				isAnyActive = true;
			}
		}

		if (isAnyActive && selectedWalls > 0) {

			for (Wall& wall : walls) {
				if (wall.isSelected) {

					if (isSliderBaseColor) {
						wall.baseColor = wallBaseColor;
					}
					if (isSliderSpecularColor) {
						wall.specularColor = wallSpecularColor;
					}
					if (isSliderRefractionCol) {
						wall.refractionColor = wallRefractionColor;
					}

					if (isSliderSpecularRough) {
						wall.specularRoughness = wallSpecularRoughness;
					}

					if (isSliderRefractionRough) {
						wall.refractionRoughness = wallRefractionRoughness;
					}

					if (isSliderRefractionAmount) {
						wall.refractionAmount = wallRefractionAmount;
					}

					if (isSliderIor) {
						wall.IOR = wallIOR;
					}

					if (isSliderDispersion) {
						wall.dispersionStrength = wallDispersion;
					}
				}
			}

			shouldRender = true;
		}

		if (isAnyActive && (selectedAreaLights > 0 || selectedPointLights > 0)) {

			if (selectedAreaLights > 0) {
				for (AreaLight& areaLight : areaLights) {
					if (areaLight.isSelected) {

						if (isSliderLightColor) {
							areaLight.color = lightColor;
						}
					}
				}
			}

			if (selectedPointLights > 0) {
				for (PointLight& pointLight : pointLights) {
					if (pointLight.isSelected) {

						if (isSliderLightColor) {
							pointLight.color = lightColor;
						}
					}
				}
			}

			shouldRender = true;
		}

		isSliderLightColor = false;

		isSliderBaseColor = false;
		isSliderSpecularColor = false;
		isSliderRefractionCol = false;

		isSliderSpecularRough = false;

		isSliderRefractionRough = false;

		isSliderRefractionAmount = false;

		isSliderIor = false;

		isSliderDispersion = false;
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

		float closestT = ray.maxLength;
		Wall* hitWall = nullptr;
		glm::vec2 hitPt;

		if (bvh.traverse(ray, closestT, hitWall, hitPt)) {
			ray.hasHit = true;
			ray.length = closestT;
			ray.hitPoint = hitPt;
			ray.wall = *hitWall;
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

		Color newColor = {
static_cast<unsigned char>(ray.color.r * ray.wall.specularColor.r / 255.0f * absorptionInvBias),
static_cast<unsigned char>(ray.color.g * ray.wall.specularColor.g / 255.0f * absorptionInvBias),
static_cast<unsigned char>(ray.color.b * ray.wall.specularColor.b / 255.0f * absorptionInvBias),
static_cast<unsigned char>(ray.color.a)
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

		entering = glm::dot(interpolatedNormal, ray.dir) < 0.0f;

		if (!entering) {
			interpolatedNormal = -interpolatedNormal;
		}

		n1 = ray.mediumIORStack.back();
		n2 = ray.wall.IOR;

		if (!entering && ray.mediumIORStack.size() > 1) {
			n2 = ray.mediumIORStack[ray.mediumIORStack.size() - 2];
		}

		std::vector<std::pair<float, Color>> activeChannels;

		Color newColor = {
				static_cast<unsigned char>(ray.color.r * ray.wall.refractionColor.r / 255.0f * absorptionInvBias),
				static_cast<unsigned char>(ray.color.g * ray.wall.refractionColor.g / 255.0f * absorptionInvBias),
				static_cast<unsigned char>(ray.color.b * ray.wall.refractionColor.b / 255.0f * absorptionInvBias),
				static_cast<unsigned char>(ray.color.a)
		};

		float dispersionStrength = ray.wall.dispersionStrength;

		if (!entering) {
			newColor = ray.color;
			dispersionStrength *= -1.0f;
		}

		if (entering && isDispersionEnabled && ray.wall.dispersionStrength > 0.0f && !ray.hasBeenDispersed) {
			activeChannels = {
				{1.0f - dispersionStrength, {newColor.r, 0, 0, ray.color.a}}, // Red
				{1.0f,                      {0, newColor.g, 0, ray.color.a}}, // Green
				{1.0f + dispersionStrength, {0, 0, newColor.b, ray.color.a}}  // Blue
			};

			ray.hasBeenDispersed = true;
		}
		else {
			float scale = 1.0f;

			if (isDispersionEnabled && ray.wall.dispersionStrength > 0.0f && !entering) {
				if (ray.color.r > ray.color.g && ray.color.r > ray.color.b) {
					scale = 1.0f - dispersionStrength; // Red ray
				}
				else if (ray.color.g > ray.color.r && ray.color.g > ray.color.b) {
					scale = 1.0f; // Green ray
				}
				else {
					scale = 1.0f + dispersionStrength; // Blue ray
				}
			}

			activeChannels = {
				{scale, newColor}
			};
		}

		for (const auto& [scale, channelColor] : activeChannels) {

			float dispersedN2 = n2 * scale;
			float eta = n1 / dispersedN2;

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

			glm::vec2 newSource = ray.hitPoint - interpolatedNormal * lightBias;

			copyRays.emplace_back(newSource, refractedDir, ray.bounceLevel + 1, channelColor);

			LightRay& newRay = copyRays.back();

			newRay.mediumIORStack = ray.mediumIORStack;

			if (entering) {
				newRay.mediumIORStack.push_back(n2);
			}
			else if (newRay.mediumIORStack.size() > 1) {
				newRay.mediumIORStack.pop_back();
			}

			newRay.hasBeenDispersed = true;

			processRayIntersection(newRay);
		}
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

		Color newColor = {
static_cast<unsigned char>(ray.color.r * ray.wall.baseColor.r / 255.0f * absorptionInvBias),
static_cast<unsigned char>(ray.color.g * ray.wall.baseColor.g / 255.0f * absorptionInvBias),
static_cast<unsigned char>(ray.color.b * ray.wall.baseColor.b / 255.0f * absorptionInvBias),
static_cast<unsigned char>(ray.color.a)
		};

		glm::vec2 newSource = ray.hitPoint + interpolatedNormal * lightBias;

		copyRays.emplace_back(newSource, rayDirection, ray.bounceLevel + 1, newColor);

		LightRay& newRay = copyRays.back();

		processRayIntersection(newRay);
	}

	void lightRendering(UpdateParameters& myParam) {

		if (currentSamples <= maxSamples) {

			rays.clear();

			for (PointLight& pointLight : pointLights) {
				pointLight.pointLightLogic(sampleRaysAmount, currentSamples, maxSamples, rays);
			}

			for (AreaLight& areaLight : areaLights) {
				areaLight.areaLightLogic(myParam, sampleRaysAmount, rays);
			}

#pragma omp parallel for
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
						if (isRefractionEnabled && !ray.reflectSpecular && ray.wall.refractionColorVal > 0.0f) {
							refraction(bounce, ray, nextBounceRays, walls);
						}
						if (isDiffuseEnabled && ray.wall.refractionAmount < 1.0f &&
							!ray.reflectSpecular && !ray.refracted) {
							diffuseLighting(bounce, ray, nextBounceRays, walls);
						}
					}
				}

				rays.insert(rays.end(), nextBounceRays.begin(), nextBounceRays.end());
			}

			currentSamples++;
		}
	}

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

	void drawMisc(UpdateVariables& myVar, UpdateParameters& myParam) {

		glm::vec2 mouseWorldPos = myParam.myCamera.mouseWorldPos;

		//Draw selection box
		if (IO::mouseDown(0) && isBoxSelecting) {
			DrawRectangleV({ boxX, boxY }, { boxWidth, boxHeight }, { 40, 40, 40, 160 });
			DrawRectangleLinesEx({ boxX, boxY, boxWidth, boxHeight }, 1.6f, WHITE);
		}

		// Draw circle spawn guide
		if (IO::mouseDown(0) && myVar.toolCircle) {

			if (!shapes.empty()) {

				if (shapes.back().isBeingSpawned) {

					Shape& shape = shapes.back();

					float radius = glm::length(shape.h2 - shape.h1);

					for (int i = 0; i < shape.circleSegments; ++i) {
						float theta1 = (2.0f * PI * i) / shape.circleSegments;
						float theta2 = (2.0f * PI * (i + 1)) / shape.circleSegments;

						glm::vec2 vA = {
							shape.h1.x + cos(theta1) * radius,
							shape.h1.y + sin(theta1) * radius
						};
						glm::vec2 vB = {
							shape.h1.x + cos(theta2) * radius,
							shape.h1.y + sin(theta2) * radius
						};
						DrawLineV({ vA.x, vA.y }, { vB.x, vB.y }, WHITE);
					}
				}
			}
		}

		// Draw lens spawn guide
		if (myVar.toolLens) {

			if (!shapes.empty()) {

				if (shapes.back().isBeingSpawned) {

					Shape& shape = shapes.back();

					if (shape.helpers.size() == 1) {
						DrawLineV({ shape.h1.x, shape.h1.y }, { shape.h2.x, shape.h2.y }, WHITE);

						DrawCircleV({ shape.h2.x, shape.h2.y }, 5.0f, PURPLE);
					}

					if (!symmetricalLens) {
						if (shape.helpers.size() >= 2) {
							DrawLineV({ shape.helpers.at(0).x, shape.helpers.at(0).y }, { shape.helpers.at(1).x, shape.helpers.at(1).y }, WHITE);
						}
					}
					else {
						if (shape.helpers.size() == 2) {
							DrawLineV({ shape.helpers.at(0).x, shape.helpers.at(0).y }, { shape.helpers.at(1).x, shape.helpers.at(1).y }, WHITE);
						}
					}

					glm::vec2 thirdHelperPos = shape.h2;
					glm::vec2 otherSide = shape.h2;
					if (shape.helpers.size() == 2) {

						glm::vec2 tangent = glm::normalize(shape.helpers.at(0) - shape.helpers.at(1));

						glm::vec2 normal = glm::vec2(tangent.y, -tangent.x);

						glm::vec2 offset = shape.h2 - shape.helpers.at(1);

						float dist;

						dist = glm::dot(offset, normal);
						shape.tempDist = dist;

						thirdHelperPos = shape.helpers.at(1) + dist * normal;

						otherSide = shape.helpers.at(0) + dist * normal;

						if (shape.helpers.size() == 2) {
							DrawLineV({ shape.helpers.at(1).x, shape.helpers.at(1).y }, { thirdHelperPos.x, thirdHelperPos.y }, WHITE);

							DrawLineV({ shape.helpers.at(0).x, shape.helpers.at(0).y }, { otherSide.x, otherSide.y }, WHITE);
						}

						DrawCircleV({ thirdHelperPos.x, thirdHelperPos.y }, 5.0f, PURPLE);
					}

					if (shape.helpers.size() >= 3) {
						DrawLineV({ shape.helpers.at(1).x, shape.helpers.at(1).y }, { shape.helpers.at(2).x, shape.helpers.at(2).y }, WHITE);
					}

					for (int i = 0; i < shape.lensSegments; i++) {
						float t1 = static_cast<float>(i) / shape.lensSegments;
						float t2 = static_cast<float>((i + 1)) / shape.lensSegments;

						float angle1 = shape.startAngle + t1 * (shape.endAngle - shape.startAngle);
						float angle2 = shape.startAngle + t2 * (shape.endAngle - shape.startAngle);

						glm::vec2 arcP1 = shape.center + glm::vec2(cos(angle1), sin(angle1)) * shape.radius;
						glm::vec2 arcP2 = shape.center + glm::vec2(cos(angle2), sin(angle2)) * shape.radius;

						if (shape.helpers.size() == 3 && !shape.fourthHelper) {
							DrawLineV({ arcP1.x, arcP1.y }, { arcP2.x, arcP2.y }, WHITE);
						}
					}

					if (symmetricalLens) {
						for (int i = 0; i < shape.lensSegments; i++) {
							float t1Symmetry = static_cast<float>(i) / shape.lensSegments;
							float t2Symmetry = static_cast<float>((i + 1)) / shape.lensSegments;

							float angle1Symmetry = shape.startAngleSymmetry + t1Symmetry * (shape.endAngleSymmetry - shape.startAngleSymmetry);
							float angle2Symmetry = shape.startAngleSymmetry + t2Symmetry * (shape.endAngleSymmetry - shape.startAngleSymmetry);

							glm::vec2 arcP1Symmetry = shape.centerSymmetry + glm::vec2(cos(angle1Symmetry), sin(angle1Symmetry)) * shape.radiusSymmetry;
							glm::vec2 arcP2Symmetry = shape.centerSymmetry + glm::vec2(cos(angle2Symmetry), sin(angle2Symmetry)) * shape.radiusSymmetry;

							if (shape.helpers.size() == 3 && !shape.fourthHelper) {
								DrawLineV({ arcP1Symmetry.x, arcP1Symmetry.y }, { arcP2Symmetry.x, arcP2Symmetry.y }, WHITE);
							}
						}
					}

					if (shape.helpers.size() >= 3) {
						DrawLineV({ shape.helpers.at(0).x, shape.helpers.at(0).y }, { shape.arcEnd.x, shape.arcEnd.y }, WHITE);
					}

					if (!shape.helpers.empty()) {
						for (auto& helper : shape.helpers) {
							shape.drawHelper(helper);
						}
					}
				}
			}
		}

		// Draw wall helpers
		if (myVar.toolMoveOptics) {
			for (Wall& wall : walls) {
				glm::vec2 dA = wall.vA - mouseWorldPos;
				glm::vec2 dB = wall.vB - mouseWorldPos;

				float distA = glm::length(dA);
				float distB = glm::length(dB);

				if (distA <= helperMinDist && !wall.isShapeWall) {
					wall.drawHelper(wall.vA);
				}
				if (distB <= helperMinDist && !wall.isShapeWall) {
					wall.drawHelper(wall.vB);
				}
			}
		}

		// Draw shape helpers
		if (!shapes.empty()) {
			for (size_t i = 0; i < shapes.size(); i++) {

				if ((shapes[i].drawHoverHelpers|| selectedHelper != -1 && selectedShape != -1) && !isAnyShapeBeingSpawned && myVar.toolMoveOptics) {
					for (glm::vec2& helper : shapes[i].helpers) {
						shapes[i].drawHelper(helper);
					}
				}
			}
		}

		// Draw light helpers
		if (myVar.toolMoveOptics) {
			for (AreaLight& areaLight : areaLights) {
				glm::vec2 dA = areaLight.vA - mouseWorldPos;
				glm::vec2 dB = areaLight.vB - mouseWorldPos;

				float distA = glm::length(dA);
				float distB = glm::length(dB);

				if (distA <= helperMinDist) {
					areaLight.drawHelper(areaLight.vA);
				}
				if (distB <= helperMinDist) {
					areaLight.drawHelper(areaLight.vB);
				}
			}
		}
	}

	void rayLogic(UpdateVariables& myVar, UpdateParameters& myParam) {

		if (shouldRender) {
			rays.clear();
			currentSamples = 0;

			wallPointers.clear();
			for (Wall& wall : walls) {
				wallPointers.push_back(&wall);
			}

			bvh.build(wallPointers);

			shouldRender = false;
		}

		if (IO::shortcutPress(KEY_C)) {
			shouldRender = true;
		}

		//rays.emplace_back(glm::vec2{ 500.0f, 500.0f }, glm::vec2{ 1.0f, 0.0f }, 1, WHITE);

		createPointLight(myVar, myParam);
		createAreaLight(myVar, myParam);
		createWall(myVar, myParam);
		createShape(myVar, myParam);

		moveLogic(myVar, myParam);

		eraseLogic(myVar, myParam);

		selectLogic(myVar, myParam);

		lightRendering(myParam);

		drawRays();

		if (IO::shortcutPress(KEY_C)) {
			rays.clear();
			pointLights.clear();
			areaLights.clear();
			walls.clear();
			shapes.clear();
		}
	}
};