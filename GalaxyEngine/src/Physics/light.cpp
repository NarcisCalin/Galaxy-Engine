#include "Physics/light.h"

void Lighting::createWall(UpdateVariables& myVar, UpdateParameters& myParam) {

	glm::vec2 mouseWorldPos = myParam.myCamera.mouseWorldPos;

	if (IO::mousePress(0) && myVar.toolWall) {

		ImVec4 colorConvert = rlImGuiColors::Convert(wallEmissionColor);

		colorConvert.w = wallEmissionGain;

		Color emissionColorFinal = rlImGuiColors::Convert(colorConvert);

		walls.emplace_back(mouseWorldPos, mouseWorldPos, false, wallBaseColor, wallSpecularColor, wallRefractionColor, emissionColorFinal,
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

void Lighting::createShape(UpdateVariables& myVar, UpdateParameters& myParam) {

	glm::vec2 mouseWorldPos = myParam.myCamera.mouseWorldPos;

	// ---- Circle ---- //
	if (IO::mousePress(0) && myVar.toolCircle) {

		ImVec4 colorConvert = rlImGuiColors::Convert(wallEmissionColor);

		colorConvert.w = wallEmissionGain;

		Color emissionColorFinal = rlImGuiColors::Convert(colorConvert);

		shapes.emplace_back(circle, mouseWorldPos, mouseWorldPos, &walls, wallBaseColor, wallSpecularColor, wallRefractionColor, emissionColorFinal,
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

		ImVec4 colorConvert = rlImGuiColors::Convert(wallEmissionColor);

		colorConvert.w = wallEmissionGain;

		Color emissionColorFinal = rlImGuiColors::Convert(colorConvert);

		shapes.emplace_back(draw, mouseWorldPos, mouseWorldPos, &walls, wallBaseColor, wallSpecularColor, wallRefractionColor, emissionColorFinal,
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

			ImVec4 colorConvert = rlImGuiColors::Convert(wallEmissionColor);

			colorConvert.w = wallEmissionGain;

			Color emissionColorFinal = rlImGuiColors::Convert(colorConvert);

			walls.emplace_back(lastPoint, firstPoint, true, wallBaseColor, wallSpecularColor, wallRefractionColor, emissionColorFinal,
				wallSpecularRoughness, wallRefractionRoughness, wallRefractionAmount, wallIOR, wallDispersion);

			walls.back().shapeId = shapes.back().id;
			shapes.back().myWallIds.push_back(walls.back().id);

			shapes.back().relaxShape(shapeRelaxIter, shapeRelaxFactor);

			shapes.back().calculateWallsNormals();
		}
		shouldRender = true;
	}

	// ---- Lens ---- //
	if (IO::mousePress(0) && myVar.toolLens && firstHelper) {

		ImVec4 colorConvert = rlImGuiColors::Convert(wallEmissionColor);

		colorConvert.w = wallEmissionGain;

		Color emissionColorFinal = rlImGuiColors::Convert(colorConvert);

		shapes.emplace_back(lens, mouseWorldPos, mouseWorldPos, &walls, wallBaseColor, wallSpecularColor, wallRefractionColor, emissionColorFinal,
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

void Lighting::createPointLight(UpdateVariables& myVar, UpdateParameters& myParam) {

	glm::vec2 mouseWorldPos = myParam.myCamera.mouseWorldPos;

	if (IO::mousePress(0) && myVar.toolPointLight) {

		ImVec4 colorConvert = rlImGuiColors::Convert(lightColor);

		colorConvert.w = lightGain;

		Color lightColorFinal = rlImGuiColors::Convert(colorConvert);

		pointLights.emplace_back(mouseWorldPos, lightColorFinal);

		shouldRender = true;
	}
}

void Lighting::createAreaLight(UpdateVariables& myVar, UpdateParameters& myParam) {

	glm::vec2 mouseWorldPos = myParam.myCamera.mouseWorldPos;

	if (IO::mousePress(0) && myVar.toolAreaLight) {

		ImVec4 colorConvert = rlImGuiColors::Convert(lightColor);

		colorConvert.w = lightGain;

		Color lightColorFinal = rlImGuiColors::Convert(colorConvert);

		areaLights.emplace_back(mouseWorldPos, mouseWorldPos, lightColorFinal, lightSpread);
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

void Lighting::createConeLight(UpdateVariables& myVar, UpdateParameters& myParam) {

	glm::vec2 mouseWorldPos = myParam.myCamera.mouseWorldPos;

	if (IO::mousePress(0) && myVar.toolConeLight) {

		ImVec4 colorConvert = rlImGuiColors::Convert(lightColor);

		colorConvert.w = lightGain;

		Color lightColorFinal = rlImGuiColors::Convert(colorConvert);

		coneLights.emplace_back(mouseWorldPos, mouseWorldPos, lightColorFinal, lightSpread);
	}

	if (IO::mouseDown(0) && myVar.toolConeLight) {
		if (coneLights.back().isBeingSpawned) {
			coneLights.back().vB = mouseWorldPos;

			shouldRender = true;
		}
	}

	if (IO::mouseReleased(0) && myVar.toolConeLight) {
		if (!coneLights.empty() && coneLights.back().isBeingSpawned) {
			if (glm::length(coneLights.back().vB - coneLights.back().vA) == 0.0f) {
				coneLights.pop_back();
				return;
			}

			coneLights.back().isBeingSpawned = false;
		}
	}
}

void Lighting::movePointLights(UpdateVariables& myVar, UpdateParameters& myParam) {

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

void Lighting::moveAreaLights(UpdateVariables& myVar, UpdateParameters& myParam) {
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

void Lighting::moveConeLights(UpdateVariables& myVar, UpdateParameters& myParam) {
	glm::vec2 mouseWorldPos = myParam.myCamera.mouseWorldPos;

	glm::vec2 mouseDelta = glm::vec2(GetMouseDelta().x, GetMouseDelta().y);
	glm::vec2 scaledDelta = mouseDelta * (1.0f / myParam.myCamera.camera.zoom);

	if (IO::mousePress(0) && myVar.toolMoveOptics) {
		for (ConeLight& coneLight : coneLights) {
			glm::vec2 dA = coneLight.vA - mouseWorldPos;
			glm::vec2 dB = coneLight.vB - mouseWorldPos;

			float distA = glm::length(dA);
			float distB = glm::length(dB);

			if (distA <= myParam.brush.brushRadius) {
				coneLight.vAisBeingMoved = true;
			}
			if (distB <= myParam.brush.brushRadius) {
				coneLight.vBisBeingMoved = true;
			}
		}
	}

	if (IO::mouseDown(0) && myVar.toolMoveOptics) {
		for (ConeLight& coneLight : coneLights) {
			if (coneLight.vAisBeingMoved) {
				coneLight.vA += scaledDelta;

				shouldRender = true;
			}
			if (coneLight.vBisBeingMoved) {
				coneLight.vB += scaledDelta;

				shouldRender = true;
			}
		}
	}

	if (IO::mouseReleased(0) && myVar.toolMoveOptics) {
		for (ConeLight& coneLight : coneLights) {
			coneLight.vAisBeingMoved = false;
			coneLight.vBisBeingMoved = false;
		}
	}
}

void Lighting::moveWalls(UpdateVariables& myVar, UpdateParameters& myParam) {
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

		float moveRelaxFactor = shapeRelaxFactor * 0.06f;

		for (Wall& wall : walls) {
			if (wall.vAisBeingMoved) {
				wall.vA += scaledDelta;

				if (wall.isShapeWall && relaxMove) {
					for (Shape& shape : shapes) {
						if (shape.id == wall.shapeId) {

							shape.relaxShape(shapeRelaxIter, moveRelaxFactor);
						}
					}
				}

				shouldRender = true;
			}
			if (wall.vBisBeingMoved) {
				wall.vB += scaledDelta;

				if (wall.isShapeWall && relaxMove) {
					for (Shape& shape : shapes) {
						if (shape.id == wall.shapeId) {
							shape.relaxShape(shapeRelaxIter, moveRelaxFactor);
						}
					}
				}

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

void Lighting::moveLogic(UpdateVariables& myVar, UpdateParameters& myParam) {

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
		moveConeLights(myVar, myParam);
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
				if ((selectedHelper == 3 || selectedHelper == 4) && IO::shortcutDown(KEY_LEFT_CONTROL)) {
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

void Lighting::eraseLogic(UpdateVariables& myVar, UpdateParameters& myParam) {

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

	for (ConeLight& l : coneLights) {
		if (l.isSelected) {
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

		for (int i = static_cast<int>(coneLights.size()) - 1; i >= 0; --i) {

			ConeLight& coneLight = coneLights[i];

			glm::vec2 dA = coneLight.vA - mouseWorldPos;
			glm::vec2 dB = coneLight.vB - mouseWorldPos;

			float distA = glm::length(dA);
			float distB = glm::length(dB);

			if (distA <= myParam.brush.brushRadius || distB <= myParam.brush.brushRadius) {

				coneLights.erase(coneLights.begin() + i);

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

		for (int i = static_cast<int>(coneLights.size()) - 1; i >= 0; --i) {
			ConeLight& coneLight = coneLights[i];

			if (coneLight.isSelected) {
				coneLights.erase(coneLights.begin() + i);
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


void Lighting::selectLogic(UpdateVariables& myVar, UpdateParameters& myParam) {

	int selectedWalls = 0;

	int selectedAreaLights = 0;

	int selectedConeLights = 0;

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

			for (ConeLight& coneLight : coneLights) {
				bool vAInBox = coneLight.vA.x >= boxX1 && coneLight.vA.x <= boxX2 &&
					coneLight.vA.y >= boxY1 && coneLight.vA.y <= boxY2;

				bool vBInBox = coneLight.vB.x >= boxX1 && coneLight.vB.x <= boxX2 &&
					coneLight.vB.y >= boxY1 && coneLight.vB.y <= boxY2;

				if (vAInBox || vBInBox) {
					if (isBoxDeselecting && coneLight.isSelected) {
						coneLight.isSelected = false;
					}
					else if (!isBoxDeselecting) {
						coneLight.isSelected = true;
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

						for (ConeLight& coneLightToDeselect : coneLights) {
							coneLightToDeselect.isSelected = false;
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

				wall.apparentColor = RED;
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

						for (ConeLight& coneLightToDeselect : coneLights) {
							coneLightToDeselect.isSelected = false;
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

				areaLight.apparentColor = RED;
			}
		}

		for (ConeLight& coneLight : coneLights) {

			glm::vec2 areaLightLine = coneLight.vB - coneLight.vA;
			glm::vec2 mouseToA = mouseWorldPos - coneLight.vA;

			float lineLenSquared = glm::dot(areaLightLine, areaLightLine);

			if (lineLenSquared == 0.0f) {

				float dist = glm::length(mouseToA);
				continue;
			}

			float t = glm::dot(mouseToA, areaLightLine) / lineLenSquared;
			t = glm::clamp(t, 0.0f, 1.0f);

			glm::vec2 closestPoint = coneLight.vA + areaLightLine * t;

			glm::vec2 diff = mouseWorldPos - closestPoint;
			float distance = glm::length(diff);

			if (distance < 5.0f) {

				isHoveringAnything = true;

				if (IO::mousePress(0)) {

					if (!IO::shortcutDown(KEY_LEFT_CONTROL) && !IO::shortcutDown(KEY_LEFT_ALT)) {
						for (AreaLight& areaLightToDeselect : areaLights) {
							areaLightToDeselect.isSelected = false;
						}

						for (ConeLight& coneLightToDeselect : coneLights) {
							coneLightToDeselect.isSelected = false;
						}

						for (PointLight& pointLight : pointLights) {
							pointLight.isSelected = false;
						}

						for (Wall& wall : walls) {
							wall.isSelected = false;
						}
					}

					if (!IO::shortcutDown(KEY_LEFT_ALT)) {
						coneLight.isSelected = true;
					}

					if (IO::shortcutDown(KEY_LEFT_ALT) && !IO::shortcutDown(KEY_LEFT_SHIFT)) {
						coneLight.isSelected = false;
					}
				}

				coneLight.apparentColor = RED;
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

						for (ConeLight& coneLightToDeselect : coneLights) {
							coneLightToDeselect.isSelected = false;
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

				pointLight.apparentColor = RED;
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

				for (ConeLight& coneLight : coneLights) {
					coneLight.isSelected = false;
				}

				for (PointLight& pointLight : pointLights) {
					pointLight.isSelected = false;
				}
			}
		}

		for (Wall& wall : walls) {
			if (wall.isSelected) {
				wall.apparentColor = RED;

				selectedWalls++;
			}
		}

		for (AreaLight& areaLight : areaLights) {
			if (areaLight.isSelected) {
				areaLight.apparentColor = RED;

				selectedAreaLights++;
			}
		}

		for (ConeLight& coneLight : coneLights) {
			if (coneLight.isSelected) {
				coneLight.apparentColor = RED;

				selectedConeLights++;
			}
		}

		for (PointLight& pointLight : pointLights) {
			if (pointLight.isSelected) {
				pointLight.apparentColor = RED;

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

				emissionColorAvg = { 0, 0, 0, 0 };
				ImVec4 emissionColAvgImgui = { 0.0f, 0.0f, 0.0f, 0.0f };

				specularRoughAvg = 0.0f;

				refractionRoughAvg = 0.0f;

				refractionAmountAvg = 0.0f;

				iorAvg = 0.0f;

				dispersionAvg = 0.0f;

				emissionGainAvg = 0.0f;

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

						// Emission Color
						ImVec4 wallEmissionColImgui = rlImGuiColors::Convert(wall.emissionColor);

						emissionColAvgImgui.x += wallEmissionColImgui.x;
						emissionColAvgImgui.y += wallEmissionColImgui.y;
						emissionColAvgImgui.z += wallEmissionColImgui.z;
						emissionColAvgImgui.w += wallEmissionColImgui.w;

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

						// Emission
						emissionGainAvg = wallEmissionColImgui.w;
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

				// Emission Color
				emissionColAvgImgui.x /= selectedWalls;
				emissionColAvgImgui.y /= selectedWalls;
				emissionColAvgImgui.z /= selectedWalls;
				emissionColAvgImgui.w /= selectedWalls;
				wallEmissionColor = rlImGuiColors::Convert(emissionColAvgImgui);

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

				// Emission
				emissionGainAvg = emissionColAvgImgui.w;
				wallEmissionGain = emissionGainAvg;
			}

			if (selectedAreaLights > 0 || selectedPointLights > 0 || selectedConeLights > 0) {

				lightColorAvg = { 0, 0, 0, 0 };
				ImVec4 lightColorAvgImgui = { 0.0f, 0.0f, 0.0f, 0.0f };

				lightSpreadAvg = 0.0f;

				lightGainAvg = 0.0f;

				if (selectedAreaLights > 0) {

					for (AreaLight& arealight : areaLights) {
						if (arealight.isSelected) {

							// Light Color
							ImVec4 lightColImgui = rlImGuiColors::Convert(arealight.color);

							lightColorAvgImgui.x += lightColImgui.x;
							lightColorAvgImgui.y += lightColImgui.y;
							lightColorAvgImgui.z += lightColImgui.z;
							lightColorAvgImgui.w += lightColImgui.w;

							// Light Spread
							lightSpreadAvg += arealight.spread;

							// Light Gain
							lightGainAvg = lightColorAvgImgui.w;
						}
					}
				}

				if (selectedConeLights > 0) {

					for (ConeLight& coneLight : coneLights) {
						if (coneLight.isSelected) {

							// Light Color
							ImVec4 lightColImgui = rlImGuiColors::Convert(coneLight.color);

							lightColorAvgImgui.x += lightColImgui.x;
							lightColorAvgImgui.y += lightColImgui.y;
							lightColorAvgImgui.z += lightColImgui.z;
							lightColorAvgImgui.w += lightColImgui.w;

							// Light Spread
							lightSpreadAvg += coneLight.spread;

							// Light Gain
							lightGainAvg = lightColorAvgImgui.w;
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

							// Light Gain
							lightGainAvg = lightColorAvgImgui.w;
						}
					}
				}

				// Light Color
				lightColorAvgImgui.x /= selectedAreaLights + selectedPointLights + selectedConeLights;
				lightColorAvgImgui.y /= selectedAreaLights + selectedPointLights + selectedConeLights;
				lightColorAvgImgui.z /= selectedAreaLights + selectedPointLights + selectedConeLights;
				lightColorAvgImgui.w /= selectedAreaLights + selectedPointLights + selectedConeLights;
				lightColor = rlImGuiColors::Convert(lightColorAvgImgui);

				// Light Spread
				if (selectedAreaLights + selectedConeLights > 0) {
					lightSpreadAvg /= selectedAreaLights + selectedConeLights;
					lightSpread = lightSpreadAvg;
				}

				// Light Gain
				lightGainAvg = lightColorAvgImgui.w;
				lightGain = lightGainAvg;
			}
		}
	}

	if (IO::mouseReleased(0) && !myVar.toolSelectOptics) {
		for (Wall& wall : walls) {
			wall.isSelected = false;
		}

		for (AreaLight& areaLight : areaLights) {
			areaLight.isSelected = false;
		}

		for (ConeLight& coneLight : coneLights) {
			coneLight.isSelected = false;
		}

		for (PointLight& pointLight : pointLights) {
			pointLight.isSelected = false;
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
				if (isSliderEmissionCol) {
					wall.emissionColor = wallEmissionColor;
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

				if (isSliderEmissionGain) {
					ImVec4 convertedColor = rlImGuiColors::Convert(wall.emissionColor);

					wall.emissionColor = rlImGuiColors::Convert(ImVec4{ convertedColor.x, convertedColor.y, convertedColor.z, wallEmissionGain });
				}
			}
		}

		shouldRender = true;
	}

	if (isAnyActive && (selectedAreaLights > 0 || selectedPointLights > 0 || selectedConeLights > 0)) {

		if (selectedAreaLights > 0) {
			for (AreaLight& areaLight : areaLights) {
				if (areaLight.isSelected) {

					if (isSliderLightGain) {
						ImVec4 convertedColor = rlImGuiColors::Convert(areaLight.color);

						areaLight.color = rlImGuiColors::Convert(ImVec4{ convertedColor.x, convertedColor.y, convertedColor.z, lightGain });
					}

					if (isSliderlightSpread) {
						areaLight.spread = lightSpread;
					}

					if (isSliderLightColor) {
						areaLight.color = lightColor;
					}
				}
			}
		}

		if (selectedConeLights > 0) {
			for (ConeLight& coneLight : coneLights) {
				if (coneLight.isSelected) {

					if (isSliderLightGain) {
						ImVec4 convertedColor = rlImGuiColors::Convert(coneLight.color);

						coneLight.color = rlImGuiColors::Convert(ImVec4{ convertedColor.x, convertedColor.y, convertedColor.z, lightGain });
					}

					if (isSliderlightSpread) {
						coneLight.spread = lightSpread;
					}

					if (isSliderLightColor) {
						coneLight.color = lightColor;
					}
				}
			}
		}

		if (selectedPointLights > 0) {
			for (PointLight& pointLight : pointLights) {
				if (pointLight.isSelected) {

					if (isSliderLightGain) {
						ImVec4 convertedColor = rlImGuiColors::Convert(pointLight.color);

						pointLight.color = rlImGuiColors::Convert(ImVec4{ convertedColor.x, convertedColor.y, convertedColor.z, lightGain });
					}

					if (isSliderLightColor) {
						pointLight.color = lightColor;
					}
				}
			}
		}

		shouldRender = true;
	}

	isSliderLightGain = false;

	isSliderlightSpread = false;

	isSliderLightColor = false;

	isSliderBaseColor = false;
	isSliderSpecularColor = false;
	isSliderRefractionCol = false;
	isSliderEmissionCol = false;

	isSliderSpecularRough = false;

	isSliderRefractionRough = false;

	isSliderRefractionAmount = false;

	isSliderIor = false;

	isSliderDispersion = false;

	isSliderEmissionGain = false;
}



float Lighting::checkIntersect(const LightRay& ray, const Wall& w) {

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

void Lighting::processRayIntersection(LightRay& ray) {
	ray.hasHit = false;
	ray.length = ray.maxLength;
	float closestT = ray.maxLength;
	Wall* hitWall = nullptr;
	glm::vec2 hitPt;

	if (bvh.traverse(ray, closestT, hitWall, hitPt) && hitWall != nullptr) {
		ray.hasHit = true;
		ray.length = closestT;
		ray.hitPoint = hitPt;
		ray.wall = *hitWall;
	}
}

// This controls specular lighting, meaning reflections. It uses a roughness parameter to control how smooth a surface is
void Lighting::specularReflection(int& currentBounce, LightRay& ray, std::vector<LightRay>& copyRays, std::vector<Wall>& walls) {

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
void Lighting::refraction(int& currentBounce, LightRay& ray, std::vector<LightRay>& copyRays, std::vector<Wall>& walls) {
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

// UNFINISHED VOLUME SCATTERING CODE. I might work on it sometime in the future

//void Lighting::volumeScatter(int& currentBounce, LightRay& ray, std::vector<LightRay>& copyRays, std::vector<Wall>& walls) {
//
//	glm::vec2 wallVec = ray.wall.vB - ray.wall.vA;
//	float wallLength = glm::length(wallVec);
//	float t = glm::clamp(glm::dot(ray.hitPoint - ray.wall.vA, wallVec) / (wallLength * wallLength), 0.0f, 1.0f);
//
//	glm::vec2 interpolatedNormal = glm::normalize(glm::mix(ray.wall.normalVA, ray.wall.normalVB, t));
//
//	bool entering = glm::dot(interpolatedNormal, ray.dir) < 0.0f;
//
//	if (glm::dot(ray.wall.normal, ray.dir) > 0.0f) {
//		interpolatedNormal = -interpolatedNormal;
//	}
//
//	float spreadMultiplier = 0.55f;
//	float maxSpreadAngle = glm::radians(90.0f * spreadMultiplier);
//
//	float randAngle = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * PI;
//
//	glm::vec2 rayDirection = glm::vec2(cos(randAngle), sin(randAngle));
//
//	bool enterAfterBounce = glm::dot(rayDirection, ray.dir) < 0.0f;
//
//	if (enterAfterBounce) {
//		interpolatedNormal = -interpolatedNormal;
//	}
//
//	Color newColor = {
//static_cast<unsigned char>(ray.color.r * ray.wall.baseColor.r / 255.0f),
//static_cast<unsigned char>(ray.color.g * ray.wall.baseColor.g / 255.0f),
//static_cast<unsigned char>(ray.color.b * ray.wall.baseColor.b / 255.0f),
//static_cast<unsigned char>(ray.color.a)
//	};
//
//	glm::vec2 newSource = ray.hitPoint - interpolatedNormal * lightBias;
//
//	if (ray.hasBeenScattered && !ray.hasHit) {
//		newSource = ray.scatterSource + ray.maxLength * ray.dir;
//	}
//
//	copyRays.emplace_back(newSource, rayDirection, ray.bounceLevel + 1, newColor);
//
//	LightRay& newRay = copyRays.back();
//
//	if (!enterAfterBounce) {
//		newRay.hasBeenScattered = true;
//		newRay.maxLength = 50.0f;
//	}
//	else {
//		newRay.hasBeenScattered = false;
//		newRay.maxLength = 10000.0f;
//	}
//
//	if (glm::dot(ray.wall.normal, ray.dir) > 0.0f && ray.hasBeenScattered && ray.hasHit) {
//		newRay.hasBeenScattered = false;
//		newRay.maxLength = 10000.0f;
//	}
//
//
//	if (!ray.hasBeenScattered) {
//		newRay.scatterSource = ray.hitPoint;
//	}
//	else {
//		newRay.scatterSource = newSource;
//	}
//
//	processRayIntersection(newRay);
//}

// This controls diffuse lighting
void Lighting::diffuseLighting(int& currentBounce, LightRay& ray, std::vector<LightRay>& copyRays, std::vector<Wall>& walls) {

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

int emissionWallsAmount = 0;

void Lighting::emission() {
	int emissionWallsAmount = 0;

	for (Wall& w : walls) {
		if (w.emissionColor.a > 0.0f) {
			emissionWallsAmount++;
		}
	}

	if (emissionWallsAmount == 0) {
		return;
	}

	int totalRays = sampleRaysAmount / emissionWallsAmount;

	totalRays = std::max(totalRays, 3);

	for (Wall& w : walls) {
		if (w.emissionColor.a <= 0.0f) continue;

		for (int i = 0; i < totalRays; i++) {
			float maxSpreadAngle = glm::radians(90.0f * 0.99f);
			float randAngle = getRandomFloat() * 2.0f * maxSpreadAngle - maxSpreadAngle;

			glm::vec2 d = w.vB - w.vA;
			float length = glm::length(d);
			glm::vec2 dNormal = d / length;

			float t = getRandomFloat();
			float bias = 0.01f;

			glm::vec2 source = w.vA + d * t + w.normal * bias;

			glm::vec2 rayDirection = rotateVec2(dNormal, randAngle);
			rayDirection = glm::vec2(rayDirection.y, -rayDirection.x);

			rays.emplace_back(
				source,
				rayDirection,
				1,
				w.emissionColor
			);
		}
	}
}

void Lighting::lightRendering(UpdateParameters& myParam) {

	if (currentSamples <= maxSamples) {

		rays.clear();

		for (PointLight& pointLight : pointLights) {
			pointLight.pointLightLogic(sampleRaysAmount, currentSamples, maxSamples, rays);
		}

		for (AreaLight& areaLight : areaLights) {
			areaLight.areaLightLogic(sampleRaysAmount, rays);
		}

		for (ConeLight& coneLight : coneLights) {
			coneLight.coneLightLogic(sampleRaysAmount, rays);
		}

		emission();

#pragma omp parallel for
		for (LightRay& ray : rays) {
			processRayIntersection(ray);
		}

		for (int bounce = 1; bounce <= maxBounces; bounce++) {
			std::vector<LightRay> nextBounceRays;

			for (LightRay& ray : rays) {
				if ((ray.hasHit || ray.hasBeenScattered) && ray.bounceLevel == bounce) {

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

void Lighting::drawMisc(UpdateVariables& myVar, UpdateParameters& myParam) {

	glm::vec2 mouseWorldPos = myParam.myCamera.mouseWorldPos;

	processApparentColor();

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

			if ((shapes[i].drawHoverHelpers || selectedHelper != -1 && selectedShape != -1) && !isAnyShapeBeingSpawned && myVar.toolMoveOptics) {
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

		for (ConeLight& coneLight : coneLights) {
			glm::vec2 dA = coneLight.vA - mouseWorldPos;
			glm::vec2 dB = coneLight.vB - mouseWorldPos;

			float distA = glm::length(dA);
			float distB = glm::length(dB);

			if (distA <= helperMinDist + 40.0f) {
				coneLight.drawHelper(coneLight.vA);
				coneLight.drawHelper(coneLight.vB);
			}
			if (distB <= helperMinDist + 40.0f) {
				coneLight.drawHelper(coneLight.vB);
				coneLight.drawHelper(coneLight.vA);
			}
		}

		for (PointLight& pointLight : pointLights) {
			glm::vec2 dA = pointLight.pos - mouseWorldPos;

			float dist = glm::length(dA);

			if (dist <= helperMinDist) {
				pointLight.drawHelper(pointLight.pos);
			}
		}
	}

	for (PointLight& pointLight : pointLights) {
		if (pointLight.isSelected) {
			pointLight.drawHelper(pointLight.pos);
		}
	}

	for (ConeLight& coneLight : coneLights) {
		if (coneLight.isSelected) {
			coneLight.drawHelper(coneLight.vA);
			coneLight.drawHelper(coneLight.vB);
		}
	}
}