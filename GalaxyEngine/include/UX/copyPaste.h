#pragma once

#include "Particles/particle.h"
#include "Physics/physics.h"
#include "Physics/light.h"

struct CopyPaste {

	glm::vec2 avgParticlePos = { 0.0f, 0.0f };
	std::vector<ParticlePhysics> pParticleCopies;
	std::vector<ParticleRendering> rParticleCopies;
	std::vector<ParticleConstraint> constraintsCopied;

	void copyPasteParticles(UpdateVariables& myVar, UpdateParameters& myParam, Physics& physics) {

		if (IO::shortcutPress(KEY_H) && !myParam.pParticlesSelected.empty()) {

			pParticleCopies.clear();
			rParticleCopies.clear();
			constraintsCopied.clear();

			size_t maxId = 0;
			for (const auto& p : myParam.pParticles) {
				if (p.id > maxId) maxId = p.id;
			}

			if (myParam.neighborSearch.idToIndexTable.size() <= maxId) {
				myParam.neighborSearch.idToIndexTable.resize(maxId + 1);
			}

			for (size_t i = 0; i < myParam.pParticles.size(); i++) {
				myParam.neighborSearch.idToIndexTable[myParam.pParticles[i].id] = i;
			}

			for (size_t i = 0; i < myParam.pParticles.size(); i++) {
				if (myParam.rParticles[i].isSelected) {
					pParticleCopies.push_back(ParticlePhysics(myParam.pParticles[i]));
					rParticleCopies.push_back(ParticleRendering(myParam.rParticles[i]));
				}
			}

			avgParticlePos = { 0.0f, 0.0f };
			for (const ParticlePhysics& pCopy : pParticleCopies) {
				avgParticlePos += pCopy.pos;
			}
			if (!pParticleCopies.empty()) {
				avgParticlePos /= static_cast<float>(pParticleCopies.size());
			}

			for (const auto& constraint : physics.particleConstraints) {

				if (constraint.id1 >= myParam.neighborSearch.idToIndexTable.size() ||
					constraint.id2 >= myParam.neighborSearch.idToIndexTable.size()) {
					continue;
				}

				size_t indexA = myParam.neighborSearch.idToIndexTable[constraint.id1];
				size_t indexB = myParam.neighborSearch.idToIndexTable[constraint.id2];

				if (indexA >= myParam.pParticles.size() || myParam.pParticles[indexA].id != constraint.id1) continue;
				if (indexB >= myParam.pParticles.size() || myParam.pParticles[indexB].id != constraint.id2) continue;

				if (myParam.rParticles[indexA].isSelected && myParam.rParticles[indexB].isSelected) {
					constraintsCopied.push_back(constraint);
				}
			}
		}

		Slingshot slingshot = slingshot.particleSlingshot(myVar, myParam.myCamera);

		if (IO::shortcutReleased(KEY_J)) {

			std::unordered_map<uint32_t, uint32_t> oldIdToNewIdMap;

			for (ParticlePhysics pCopy : pParticleCopies) {

				glm::vec2 copyRelPos = pCopy.pos - avgParticlePos;
				pCopy.pos = myParam.myCamera.mouseWorldPos + copyRelPos;

				pCopy.vel += slingshot.length * slingshot.norm * 0.3f;
				pCopy.prevVel = pCopy.vel;
				pCopy.ke = 0.0f;
				pCopy.prevKe = 0.0f;

				uint32_t oldID = pCopy.id;
				uint32_t newID = globalId++;

				pCopy.id = newID;
				oldIdToNewIdMap[oldID] = newID;

				myParam.pParticles.push_back(ParticlePhysics(pCopy));
			}

			for (ParticleRendering rCopy : rParticleCopies) {
				rCopy.isSelected = false;
				rCopy.isBeingDrawn = true;
				myParam.rParticles.push_back(ParticleRendering(rCopy));
			}

			if (!myVar.hasAVX2) {
				myParam.neighborSearchV2.newGrid(myParam.pParticles);
				myParam.neighborSearchV2.neighborAmount(myParam.pParticles, myParam.rParticles);
			}
			else {
				myParam.neighborSearchV2AVX2.newGridAVX2(myParam.pParticles);
				myParam.neighborSearchV2AVX2.neighborAmount(myParam.pParticles, myParam.rParticles);
			}

			if (myParam.neighborSearch.idToIndexTable.size() <= globalId) {
				myParam.neighborSearch.idToIndexTable.resize(globalId + 1);
			}
#pragma omp parallel for
			for (size_t i = 0; i < myParam.pParticles.size(); i++) {
				myParam.neighborSearch.idToIndexTable[myParam.pParticles[i].id] = i;
			}

			bool constraintsAdded = false;

			for (const ParticleConstraint& srcConst : constraintsCopied) {

				auto itA = oldIdToNewIdMap.find(srcConst.id1);
				auto itB = oldIdToNewIdMap.find(srcConst.id2);

				if (itA != oldIdToNewIdMap.end() && itB != oldIdToNewIdMap.end()) {

					uint32_t newIDA = itA->second;
					uint32_t newIDB = itB->second;

					ParticleConstraint newConst = srcConst;
					newConst.id1 = newIDA;
					newConst.id2 = newIDB;

					physics.particleConstraints.push_back(newConst);
					constraintsAdded = true;
				}
			}

			if (constraintsAdded) {
				physics.constraintMap.clear();

				for (ParticleConstraint& c : physics.particleConstraints) {

					if (c.isBroken) continue;

					uint64_t key = physics.makeKey(c.id1, c.id2);
					physics.constraintMap[key] = &c;
				}
			}

			for (size_t i = 0; i < myParam.pParticles.size(); i++) {
				myParam.rParticles[i].isBeingDrawn = false;
			}

			myVar.isDragging = false;
		}
	}

	glm::vec3 avgParticlePos3D = { 0.0f, 0.0f, 0.0f };
	std::vector<ParticlePhysics3D> pParticleCopies3D;
	std::vector<ParticleRendering3D> rParticleCopies3D;

	std::vector<ParticleConstraint> constraintsCopied3D;

	void copyPasteParticles3D(UpdateVariables& myVar, UpdateParameters& myParam, Physics3D& physics3D) {

		if (IO::shortcutPress(KEY_H) && !myParam.pParticlesSelected3D.empty()) {

			pParticleCopies3D.clear();
			rParticleCopies3D.clear();
			constraintsCopied3D.clear();

			size_t maxId = 0;
			for (const auto& p : myParam.pParticles3D) {
				if (p.id > maxId) maxId = p.id;
			}

			if (myParam.neighborSearch3D.idToIndexTable.size() <= maxId) {
				myParam.neighborSearch3D.idToIndexTable.resize(maxId + 1);
			}

			for (size_t i = 0; i < myParam.pParticles3D.size(); i++) {
				myParam.neighborSearch3D.idToIndexTable[myParam.pParticles3D[i].id] = i;
			}

			for (size_t i = 0; i < myParam.pParticles3D.size(); i++) {
				if (myParam.rParticles3D[i].isSelected) {
					pParticleCopies3D.push_back(ParticlePhysics3D(myParam.pParticles3D[i]));
					rParticleCopies3D.push_back(ParticleRendering3D(myParam.rParticles3D[i]));
				}
			}

			avgParticlePos3D = { 0.0f, 0.0f, 0.0f };
			for (const ParticlePhysics3D& pCopy : pParticleCopies3D) {
				avgParticlePos3D += pCopy.pos;
			}
			if (!pParticleCopies3D.empty()) {
				avgParticlePos3D /= static_cast<float>(pParticleCopies3D.size());
			}

			for (const auto& constraint : physics3D.particleConstraints) {

				if (constraint.id1 >= myParam.neighborSearch3D.idToIndexTable.size() ||
					constraint.id2 >= myParam.neighborSearch3D.idToIndexTable.size()) {
					continue;
				}

				size_t indexA = myParam.neighborSearch3D.idToIndexTable[constraint.id1];
				size_t indexB = myParam.neighborSearch3D.idToIndexTable[constraint.id2];

				if (indexA >= myParam.pParticles3D.size() || myParam.pParticles3D[indexA].id != constraint.id1) continue;
				if (indexB >= myParam.pParticles3D.size() || myParam.pParticles3D[indexB].id != constraint.id2) continue;

				if (myParam.rParticles3D[indexA].isSelected && myParam.rParticles3D[indexB].isSelected) {
					constraintsCopied3D.push_back(constraint);
				}
			}
		}

		Slingshot3D slingshot = slingshot.particleSlingshot(myVar, myParam.brush3D.brushPos);

		if (IO::shortcutReleased(KEY_J)) {

			std::unordered_map<uint32_t, uint32_t> oldIdToNewIdMap;

			for (ParticlePhysics3D pCopy : pParticleCopies3D) {

				glm::vec3 copyRelPos = pCopy.pos - avgParticlePos3D;
				pCopy.pos = myParam.brush3D.brushPos + copyRelPos;

				pCopy.vel += slingshot.length * slingshot.norm * 0.3f;
				pCopy.prevVel = pCopy.vel;
				pCopy.ke = 0.0f;
				pCopy.prevKe = 0.0f;

				uint32_t oldID = pCopy.id;
				uint32_t newID = globalId++;

				pCopy.id = newID;
				oldIdToNewIdMap[oldID] = newID;

				myParam.pParticles3D.push_back(ParticlePhysics3D(pCopy));
			}

			for (ParticleRendering3D rCopy : rParticleCopies3D) {
				rCopy.isSelected = false;
				rCopy.isBeingDrawn = true;
				myParam.rParticles3D.push_back(ParticleRendering3D(rCopy));
			}

			if (!myVar.hasAVX2) {
				myParam.neighborSearch3DV2.newGrid(myParam.pParticles3D);
				myParam.neighborSearch3DV2.neighborAmount(myParam.pParticles3D, myParam.rParticles3D);
			}
			else {
				myParam.neighborSearch3DV2AVX2.newGridAVX2(myParam.pParticles3D);
				myParam.neighborSearch3DV2AVX2.neighborAmount(myParam.pParticles3D, myParam.rParticles3D);
			}

			if (myParam.neighborSearch3D.idToIndexTable.size() <= globalId) {
				myParam.neighborSearch3D.idToIndexTable.resize(globalId + 1);
			}
#pragma omp parallel for
			for (size_t i = 0; i < myParam.pParticles3D.size(); i++) {
				myParam.neighborSearch3D.idToIndexTable[myParam.pParticles3D[i].id] = i;
			}

			bool constraintsAdded = false;

			for (const ParticleConstraint& srcConst : constraintsCopied3D) {

				auto itA = oldIdToNewIdMap.find(srcConst.id1);
				auto itB = oldIdToNewIdMap.find(srcConst.id2);

				if (itA != oldIdToNewIdMap.end() && itB != oldIdToNewIdMap.end()) {

					uint32_t newIDA = itA->second;
					uint32_t newIDB = itB->second;

					ParticleConstraint newConst = srcConst;
					newConst.id1 = newIDA;
					newConst.id2 = newIDB;

					physics3D.particleConstraints.push_back(newConst);
					constraintsAdded = true;
				}
			}

			if (constraintsAdded) {
				physics3D.constraintMap.clear();

				for (ParticleConstraint& c : physics3D.particleConstraints) {

					if (c.isBroken) continue;

					uint64_t key = physics3D.makeKey(c.id1, c.id2);
					physics3D.constraintMap[key] = &c;
				}
			}

			for (size_t i = 0; i < myParam.pParticles3D.size(); i++) {
				myParam.rParticles3D[i].isBeingDrawn = false;
			}

			myVar.isDragging = false;
		}
	}

	glm::vec2 avgOpticsPos = { 0.0f, 0.0f };
	glm::vec2 avgShapeHelpersPos = { 0.0f, 0.0f };
	std::vector<Wall> wallCopies;
	std::vector<Shape> shapeCopies;

	std::vector<PointLight> pointLightCopies;
	std::vector<AreaLight> areaLightCopies;
	std::vector<ConeLight> coneLightCopies;

	void copyPasteOptics(UpdateParameters& myParam, Lighting& lighting) {

		if (IO::shortcutPress(KEY_H) && (!lighting.walls.empty() || !lighting.pointLights.empty() || !lighting.areaLights.empty() || !lighting.coneLights.empty())) {

			pointLightCopies.clear();
			areaLightCopies.clear();
			coneLightCopies.clear();

			avgOpticsPos = { 0.0f, 0.0f };

			// Point Lights
			for (size_t i = 0; i < lighting.pointLights.size(); i++) {
				if (lighting.pointLights[i].isSelected) {
					pointLightCopies.push_back(PointLight(lighting.pointLights[i]));
				}
			}

			if (!pointLightCopies.empty()) {
				for (const PointLight& pl : pointLightCopies) {
					avgOpticsPos += pl.pos;
				}
				avgOpticsPos /= static_cast<float>(pointLightCopies.size());
			}

			// Area Lights
			for (size_t i = 0; i < lighting.areaLights.size(); i++) {
				if (lighting.areaLights[i].isSelected) {
					areaLightCopies.push_back(AreaLight(lighting.areaLights[i]));
				}
			}

			if (!areaLightCopies.empty()) {
				for (const AreaLight& al : areaLightCopies) {
					avgOpticsPos += al.vA;
					avgOpticsPos += al.vB;
				}
				avgOpticsPos /= static_cast<float>(areaLightCopies.size() * 2.0f);
			}

			// Cone Lights
			for (size_t i = 0; i < lighting.coneLights.size(); i++) {
				if (lighting.coneLights[i].isSelected) {
					coneLightCopies.push_back(ConeLight(lighting.coneLights[i]));
				}
			}

			if (!coneLightCopies.empty()) {
				for (const ConeLight& cl : coneLightCopies) {
					avgOpticsPos += cl.vA;
					avgOpticsPos += cl.vB;
				}
				avgOpticsPos /= static_cast<float>(coneLightCopies.size() * 2.0f);
			}

			wallCopies.clear();
			shapeCopies.clear();

			for (size_t i = 0; i < lighting.walls.size(); i++) {
				if (lighting.walls[i].isSelected) {
					wallCopies.push_back(Wall(lighting.walls[i]));
				}
			}

			std::unordered_set<uint32_t> copiedWallIds;
			for (const Wall& wall : wallCopies)
				copiedWallIds.insert(wall.id);

			for (const Shape& shape : lighting.shapes) {
				bool copyThisShape = true;
				for (uint32_t wallId : shape.myWallIds) {
					if (copiedWallIds.find(wallId) == copiedWallIds.end()) {
						copyThisShape = false;
						break;
					}
				}
				if (copyThisShape)
					shapeCopies.push_back(shape);
			}

			if (!wallCopies.empty()) {
				for (const Wall& wall : wallCopies) {
					avgOpticsPos += wall.vA;
					avgOpticsPos += wall.vB;
				}
				avgOpticsPos /= static_cast<float>(wallCopies.size() * 2);
			}

			avgShapeHelpersPos = avgOpticsPos;
		}

		if (IO::shortcutReleased(KEY_J)) {

			for (PointLight plCopy : pointLightCopies) {

				glm::vec2 copyRelPos = plCopy.pos - avgOpticsPos;

				plCopy.pos = myParam.myCamera.mouseWorldPos + copyRelPos;

				lighting.pointLights.push_back(PointLight(plCopy));
			}

			for (AreaLight alCopy : areaLightCopies) {

				glm::vec2 copyRelVA = alCopy.vA - avgOpticsPos;
				glm::vec2 copyRelVB = alCopy.vB - avgOpticsPos;

				alCopy.vA = myParam.myCamera.mouseWorldPos + copyRelVA;
				alCopy.vB = myParam.myCamera.mouseWorldPos + copyRelVB;

				lighting.areaLights.push_back(AreaLight(alCopy));
			}

			for (ConeLight clCopy : coneLightCopies) {

				glm::vec2 copyRelVA = clCopy.vA - avgOpticsPos;
				glm::vec2 copyRelVB = clCopy.vB - avgOpticsPos;

				clCopy.vA = myParam.myCamera.mouseWorldPos + copyRelVA;
				clCopy.vB = myParam.myCamera.mouseWorldPos + copyRelVB;

				lighting.coneLights.push_back(ConeLight(clCopy));
			}

			for (PointLight plCopy : pointLightCopies) {

				glm::vec2 copyRelPos = plCopy.pos - avgOpticsPos;

				plCopy.pos = myParam.myCamera.mouseWorldPos + copyRelPos;

				lighting.pointLights.push_back(PointLight(plCopy));
			}

			std::unordered_map<uint32_t, uint32_t> oldToNewWallIds;

			std::unordered_set<uint32_t> usedInShapes;
			for (const Shape& shape : shapeCopies) {
				for (uint32_t wallId : shape.myWallIds)
					usedInShapes.insert(wallId);
			}

			for (const Wall& wallCopy : wallCopies) {
				Wall newWall = wallCopy;

				glm::vec2 copyRelVA = newWall.vA - avgOpticsPos;
				glm::vec2 copyRelVB = newWall.vB - avgOpticsPos;
				newWall.vA = myParam.myCamera.mouseWorldPos + copyRelVA;
				newWall.vB = myParam.myCamera.mouseWorldPos + copyRelVB;

				uint32_t oldId = newWall.id;
				newWall.id = globalWallId++;
				oldToNewWallIds[oldId] = newWall.id;

				if (usedInShapes.find(oldId) == usedInShapes.end()) {
					newWall.shapeId = static_cast<uint32_t>(-1);
					newWall.isShapeWall = false;
				}

				newWall.isSelected = false;
				lighting.walls.push_back(newWall);
			}

			for (Shape& shapeCopy : shapeCopies) {
				Shape newShape = shapeCopy;

				newShape.id = globalShapeId++;

				for (glm::vec2& helper : newShape.helpers) {
					glm::vec2 helperRelPos = helper - avgShapeHelpersPos;
					helper = myParam.myCamera.mouseWorldPos + helperRelPos;
				}

				newShape.myWallIds.clear();
				for (uint32_t oldWallId : shapeCopy.myWallIds) {
					auto it = oldToNewWallIds.find(oldWallId);
					if (it != oldToNewWallIds.end()) {
						uint32_t newWallId = it->second;
						newShape.myWallIds.emplace_back(newWallId);

						Wall* w = lighting.getWallById(lighting.walls, newWallId);
						if (w) {
							w->shapeId = newShape.id;
						}
					}
				}

				newShape.walls = &lighting.walls;

				if (newShape.shapeType == draw) {
					newShape.helpers[0] = glm::vec2(0.0f);
					for (auto& wallId : newShape.myWallIds) {
						Wall* w = lighting.getWallById(lighting.walls, wallId);
						if (w) {
							newShape.helpers[0] += w->vA;
							newShape.helpers[0] += w->vB;
						}
					}
					newShape.helpers[0] /= static_cast<float>(newShape.myWallIds.size()) * 2.0f;
					newShape.oldDrawHelperPos = newShape.helpers[0];
				}

				if (newShape.shapeType == lens) {

					auto itC = oldToNewWallIds.find(shapeCopy.wallCId);
					if (itC != oldToNewWallIds.end()) {
						newShape.wallCId = itC->second;
					}

					auto itB = oldToNewWallIds.find(shapeCopy.wallBId);
					if (itB != oldToNewWallIds.end()) {
						newShape.wallBId = itB->second;
					}

					auto itA = oldToNewWallIds.find(shapeCopy.wallAId);
					if (itA != oldToNewWallIds.end()) {
						newShape.wallAId = itA->second;
					}

					newShape.globalLensPrev = newShape.helpers.back();
				}

				lighting.shapes.push_back(newShape);
			}

			if (!wallCopies.empty() || !pointLightCopies.empty() || !areaLightCopies.empty() || !coneLightCopies.empty()) {
				lighting.shouldRender = true;
			}
		}
	}
};