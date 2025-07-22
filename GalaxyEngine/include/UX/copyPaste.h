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

			for (size_t i = 0; i < myParam.pParticles.size(); i++) {
				if (myParam.rParticles[i].isSelected) {
					pParticleCopies.push_back(ParticlePhysics(myParam.pParticles[i]));
					rParticleCopies.push_back(ParticleRendering(myParam.rParticles[i]));
				}
			}

			avgParticlePos = { 0.0f, 0.0f };

			for (ParticlePhysics& pCopy : pParticleCopies) {

				avgParticlePos += pCopy.pos;
			}

			avgParticlePos /= static_cast<float>(pParticleCopies.size());
		}

		Slingshot slingshot = slingshot.particleSlingshot(myVar, myParam.myCamera);

		if (IO::shortcutReleased(KEY_J)) {

			for (ParticlePhysics pCopy : pParticleCopies) {

				glm::vec2 copyRelPos = pCopy.pos - avgParticlePos;

				pCopy.pos = myParam.myCamera.mouseWorldPos + copyRelPos;

				pCopy.vel += slingshot.length * slingshot.norm * 0.3f;
				pCopy.prevVel = pCopy.vel;

				pCopy.ke = 0.0f;
				pCopy.prevKe = 0.0f;

				pCopy.id = globalId++;

				myParam.pParticles.push_back(ParticlePhysics(pCopy));
			}

			for (ParticleRendering rCopy : rParticleCopies) {

				rCopy.isSelected = false;

				rCopy.isBeingDrawn = true;

				myParam.rParticles.push_back(ParticleRendering(rCopy));
			}

			NeighborSearch::idToI(myParam.pParticles);
			myParam.neighborSearch.neighborSearchHash(myParam.pParticles, myParam.rParticles);

			bool enabled = true;
			physics.createConstraints(myParam.pParticles, myParam.rParticles, enabled, myVar);

			for (size_t i = 0; i < myParam.pParticles.size(); i++) {
				myParam.rParticles[i].isBeingDrawn = false;
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

		if (IO::shortcutPress(KEY_H) && (!lighting.walls.empty() || !lighting.pointLights.empty())) {

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

			if (!pointLightCopies.empty()) {
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
				}

				lighting.shapes.push_back(newShape);
			}

			if (!wallCopies.empty() || !pointLightCopies.empty() || !areaLightCopies.empty() || !coneLightCopies.empty()) {
				lighting.shouldRender = true;
			}
		}
	}
};