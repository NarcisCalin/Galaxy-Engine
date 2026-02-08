#include "UX/saveSystem.h"

void SaveSystem::saveSystem(const std::string& filename, UpdateVariables& myVar, UpdateParameters& myParam, SPH& sph, Physics& physics, Lighting& lighting, Field& field) {

	YAML::Emitter out;
	/*out << YAML::BeginMap;*/

	// ----- Trails -----
	paramIO(filename, out, "GlobalTrails", myVar.isGlobalTrailsEnabled);
	paramIO(filename, out, "SelectedTrails", myVar.isSelectedTrailsEnabled);
	paramIO(filename, out, "LocalTrails", myVar.isLocalTrailsEnabled);
	paramIO(filename, out, "WhiteTrails", myParam.trails.whiteTrails);
	paramIO(filename, out, "TrailsMaxLength", myVar.trailMaxLength);
	paramIO(filename, out, "TrailsThickness", myParam.trails.trailThickness);

	// ----- Color parameters -----
	paramIO(filename, out, "SolidColor", myParam.colorVisuals.solidColor);
	paramIO(filename, out, "DensityColor", myParam.colorVisuals.densityColor);
	paramIO(filename, out, "ForceColor", myParam.colorVisuals.forceColor);
	paramIO(filename, out, "VelocityColor", myParam.colorVisuals.velocityColor);
	paramIO(filename, out, "ShockwaveColor", myParam.colorVisuals.shockwaveColor);
	paramIO(filename, out, "TurbulenceColor", myParam.colorVisuals.turbulenceColor);
	paramIO(filename, out, "PressureColor", myParam.colorVisuals.pressureColor);
	paramIO(filename, out, "SPHColor", myParam.colorVisuals.SPHColor);
	paramIO(filename, out, "SelectedColor", myParam.colorVisuals.selectedColor);
	paramIO(filename, out, "ShowDarkMatter", myParam.colorVisuals.showDarkMatterEnabled);

	// ----- Color sliders -----
	paramIO(filename, out, "ColorMaxVel", myParam.colorVisuals.maxVel);
	paramIO(filename, out, "ColorMaxPressure", myParam.colorVisuals.maxPress);
	paramIO(filename, out, "MaxColorForce", myParam.colorVisuals.maxColorAcc);
	paramIO(filename, out, "ShockwaveMaxAcc", myParam.colorVisuals.ShockwaveMaxAcc);
	paramIO(filename, out, "MaxColorTurbulence", myParam.colorVisuals.maxColorTurbulence);
	paramIO(filename, out, "TurbulenceFadeRate", myParam.colorVisuals.turbulenceFadeRate);
	paramIO(filename, out, "TurbulenceContrast", myParam.colorVisuals.turbulenceContrast);
	paramIO(filename, out, "TurbulenceCustomColor", myParam.colorVisuals.turbulenceCustomCol);
	paramIO(filename, out, "TemperatureColor", myParam.colorVisuals.temperatureColor);
	paramIO(filename, out, "TemperatureGasColor", myParam.colorVisuals.gasTempColor);
	paramIO(filename, out, "MaxTemperatureColor", myParam.colorVisuals.tempColorMaxTemp);
	paramIO(filename, out, "MaxNeighbors", myParam.colorVisuals.maxNeighbors);

	// ----- Other visual sliders -----
	paramIO(filename, out, "MaxSizeForce", myParam.densitySize.sizeAcc);
	paramIO(filename, out, "Max Dynamic Size", myParam.densitySize.maxSize);
	paramIO(filename, out, "Min Dynamic Size", myParam.densitySize.minSize);
	paramIO(filename, out, "MaxSizeNeighbors", myParam.densitySize.maxNeighbors);
	paramIO(filename, out, "ParticleSizeMult", myVar.particleSizeMultiplier);
	paramIO(filename, out, "VisiblePAmountMult", myVar.particleAmountMultiplier);
	paramIO(filename, out, "DMPAmountMult", myVar.DMAmountMultiplier);
	paramIO(filename, out, "MassRandomMultiplier", myVar.massScatter);
	paramIO(filename, out, "MassMultiplierToggle", myParam.particlesSpawning.massMultiplierEnabled);

	// ----- Colors -----
	paramIO(filename, out, "pColorsR", myParam.colorVisuals.pColor.r);
	paramIO(filename, out, "pColorsG", myParam.colorVisuals.pColor.g);
	paramIO(filename, out, "pColorsB", myParam.colorVisuals.pColor.b);
	paramIO(filename, out, "pColorsA", myParam.colorVisuals.pColor.a);

	paramIO(filename, out, "sColorsR", myParam.colorVisuals.sColor.r);
	paramIO(filename, out, "sColorsG", myParam.colorVisuals.sColor.g);
	paramIO(filename, out, "sColorsB", myParam.colorVisuals.sColor.b);
	paramIO(filename, out, "sColorsA", myParam.colorVisuals.sColor.a);

	// ----- Misc Toggles -----
	paramIO(filename, out, "DarkMatter", myVar.isDarkMatterEnabled);
	paramIO(filename, out, "LoopingSpace", myVar.isPeriodicBoundaryEnabled);
	paramIO(filename, out, "SPHEnabled", myVar.isSPHEnabled);
	paramIO(filename, out, "DensitySize", myVar.isDensitySizeEnabled);
	paramIO(filename, out, "ForceSize", myVar.isForceSizeEnabled);
	paramIO(filename, out, "Glow", myVar.isGlowEnabled);
	paramIO(filename, out, "ShipGas", myVar.isShipGasEnabled);
	paramIO(filename, out, "Merger", myVar.isMergerEnabled);

	// ----- SPH Materials -----
	paramIO(filename, out, "SPHWater", myParam.brush.SPHWater);
	paramIO(filename, out, "SPHRock", myParam.brush.SPHRock);
	paramIO(filename, out, "SPHIron", myParam.brush.SPHIron);
	paramIO(filename, out, "SPHSand", myParam.brush.SPHSand);
	paramIO(filename, out, "SPHSoil", myParam.brush.SPHSoil);
	paramIO(filename, out, "SPHIce", myParam.brush.SPHIce);
	paramIO(filename, out, "SPHMud", myParam.brush.SPHMud);
	paramIO(filename, out, "SPHRubber", myParam.brush.SPHRubber);
	paramIO(filename, out, "SPHGas", myParam.brush.SPHGas);

	// ----- Physics params -----
	paramIO(filename, out, "Softening", myVar.softening);
	paramIO(filename, out, "Theta", myVar.theta);
	paramIO(filename, out, "TimeMult", myVar.timeStepMultiplier);
	paramIO(filename, out, "GravityMultiplier", myVar.gravityMultiplier);
	paramIO(filename, out, "HeavyParticlesMass", myVar.heavyParticleWeightMultiplier);
	paramIO(filename, out, "TemperatureSimulation", myVar.isTempEnabled);
	paramIO(filename, out, "AmbientTemperature", myVar.ambientTemp);
	paramIO(filename, out, "AmbientHeatRate", myVar.globalAmbientHeatRate);
	paramIO(filename, out, "HeatConductivityMultiplier", myVar.globalHeatConductivity);

	// ----- SPH -----
	paramIO(filename, out, "SPHGravity", sph.verticalGravity);
	paramIO(filename, out, "SPHRadiusMult", sph.radiusMultiplier);
	paramIO(filename, out, "SPHMass", sph.mass);
	paramIO(filename, out, "SPHViscosity", sph.viscosity);
	paramIO(filename, out, "SPHStiffness", sph.stiffMultiplier);
	paramIO(filename, out, "SPHCohesion", sph.cohesionCoefficient);
	paramIO(filename, out, "SPHGround", myVar.sphGround);
	paramIO(filename, out, "SPHDelta", sph.delta);
	paramIO(filename, out, "SPHMaxVel", myVar.sphMaxVel);

	// ----- Domain size -----
	paramIO(filename, out, "DomainWidth", myVar.domainSize.x);
	paramIO(filename, out, "DomainHeight", myVar.domainSize.y);

	// ----- Camera -----
	paramIO(filename, out, "CameraTargetX", myParam.myCamera.camera.target.x);
	paramIO(filename, out, "CameraTargetY", myParam.myCamera.camera.target.y);
	paramIO(filename, out, "CameraOffsetX", myParam.myCamera.camera.offset.x);
	paramIO(filename, out, "CameraOffsetY", myParam.myCamera.camera.offset.y);
	paramIO(filename, out, "CameraZoom", myParam.myCamera.camera.zoom);

	// ----- Constraints -----
	paramIO(filename, out, "ParticleConstraints", myVar.constraintsEnabled);
	paramIO(filename, out, "UnbreakableConstraints", myVar.unbreakableConstraints);
	paramIO(filename, out, "ConstraintAfterDrawing", myVar.constraintAfterDrawing);
	paramIO(filename, out, "VisualizeConstraints", myVar.drawConstraints);
	paramIO(filename, out, "VisualizeMesh", myVar.visualizeMesh);
	paramIO(filename, out, "ConstraintsStressColor", myVar.constraintStressColor);
	paramIO(filename, out, "MaxConstraintStress", myVar.constraintMaxStressColor);
	paramIO(filename, out, "ConstraintsStiffMultiplier", myVar.globalConstraintStiffnessMult);
	paramIO(filename, out, "ConstraintsResistMultiplier", myVar.globalConstraintResistance);

	// ----- Optics ----- 
	paramIO(filename, out, "Optics", myVar.isOpticsEnabled);

	// ----- Optics Colors ----- 
	paramIO(filename, out, "LColorR", lighting.lightColor.r);
	paramIO(filename, out, "LColorG", lighting.lightColor.g);
	paramIO(filename, out, "LColorB", lighting.lightColor.b);
	paramIO(filename, out, "LColorA", lighting.lightColor.a);

	paramIO(filename, out, "BaseColorR", lighting.wallBaseColor.r);
	paramIO(filename, out, "BaseColorG", lighting.wallBaseColor.g);
	paramIO(filename, out, "BaseColorB", lighting.wallBaseColor.b);
	paramIO(filename, out, "BaseColorA", lighting.wallBaseColor.a);

	paramIO(filename, out, "SpecularColorR", lighting.wallSpecularColor.r);
	paramIO(filename, out, "SpecularColorG", lighting.wallSpecularColor.g);
	paramIO(filename, out, "SpecularColorB", lighting.wallSpecularColor.b);
	paramIO(filename, out, "SpecularColorA", lighting.wallSpecularColor.a);

	paramIO(filename, out, "RefractionColorR", lighting.wallRefractionColor.r);
	paramIO(filename, out, "RefractionColorG", lighting.wallRefractionColor.g);
	paramIO(filename, out, "RefractionColorB", lighting.wallRefractionColor.b);
	paramIO(filename, out, "RefractionColorA", lighting.wallRefractionColor.a);

	paramIO(filename, out, "EmissionColorR", lighting.wallEmissionColor.r);
	paramIO(filename, out, "EmissionColorG", lighting.wallEmissionColor.g);
	paramIO(filename, out, "EmissionColorB", lighting.wallEmissionColor.b);
	paramIO(filename, out, "EmissionColorA", lighting.wallEmissionColor.a);

	// ----- Optics Lights ----- 
	paramIO(filename, out, "LightGain", lighting.lightGain);
	paramIO(filename, out, "LightSpread", lighting.lightSpread);

	// ----- Optics Walls ----- 
	paramIO(filename, out, "WallSpecularRough", lighting.wallSpecularRoughness);
	paramIO(filename, out, "WallRefractionRough", lighting.wallRefractionRoughness);
	paramIO(filename, out, "WallRefractionAmount", lighting.wallRefractionAmount);
	paramIO(filename, out, "WallIOR", lighting.wallIOR);
	paramIO(filename, out, "WallDispersion", lighting.wallDispersion);
	paramIO(filename, out, "WallEmissionGain", lighting.wallEmissionGain);

	// ----- Optics Shapes -----
	paramIO(filename, out, "ShapeRelaxIter", lighting.shapeRelaxIter);
	paramIO(filename, out, "ShapeRelaxFactor", lighting.shapeRelaxFactor);

	// ----- Optics Render -----
	paramIO(filename, out, "MaxSamples", lighting.maxSamples);
	paramIO(filename, out, "RaysPerSample", lighting.sampleRaysAmount);
	paramIO(filename, out, "MaxBounces", lighting.maxBounces);

	// ----- Optics Render Passes -----
	paramIO(filename, out, "GI", lighting.isDiffuseEnabled);
	paramIO(filename, out, "Specular", lighting.isSpecularEnabled);
	paramIO(filename, out, "Refraction", lighting.isRefractionEnabled);
	paramIO(filename, out, "Dispersion", lighting.isDispersionEnabled);
	paramIO(filename, out, "Emission", lighting.isEmissionEnabled);

	// ----- Optics Misc -----
	paramIO(filename, out, "SymmetricalLens", lighting.symmetricalLens);
	paramIO(filename, out, "ShowNormals", lighting.drawNormals);
	paramIO(filename, out, "RelaxMove", lighting.relaxMove);

	// ----- Gravity Field -----
	paramIO(filename, out, "GravityField", myVar.isGravityFieldEnabled);
	paramIO(filename, out, "GravityFieldDM", myVar.gravityFieldDMParticles);
	paramIO(filename, out, "GravityDisplayThreshold", field.gravityDisplayThreshold);
	paramIO(filename, out, "GravityDisplaySoftness", field.gravityDisplaySoftness);
	paramIO(filename, out, "GravityDisplayStretch", field.gravityStretchFactor);
	paramIO(filename, out, "GravityCustomColors", field.gravityCustomColors);
	paramIO(filename, out, "GravityCustomColExp", field.gravityExposure);

	// ----- Field Misc -----
	paramIO(filename, out, "FieldRes", field.res);

	/*out << YAML::EndMap;*/

	std::string yamlString = out.c_str();

	std::fstream file;
	if (saveFlag) {
		std::fstream file(filename, std::ios::out | std::ios::binary | std::ios::trunc);
		if (!file.is_open()) {
			std::cerr << "Failed to open file for writing: " << filename << "\n";
			return;
		}

		file.write(yamlString.c_str(), yamlString.size());

		const char* separator = "\n---PARTICLE BINARY DATA---\n";
		file.write(separator, strlen(separator));

		file.write(reinterpret_cast<const char*>(&currentVersion), sizeof(currentVersion));

		file.write(reinterpret_cast<const char*>(&globalId), sizeof(globalId));
		file.write(reinterpret_cast<const char*>(&globalShapeId), sizeof(globalShapeId));
		file.write(reinterpret_cast<const char*>(&globalWallId), sizeof(globalWallId));

		uint32_t particleCount = myParam.pParticles.size();
		file.write(reinterpret_cast<const char*>(&particleCount), sizeof(particleCount));

		for (size_t i = 0; i < myParam.pParticles.size(); i++) {
			const ParticlePhysics& p = myParam.pParticles[i];
			const ParticleRendering& r = myParam.rParticles[i];

			file.write(reinterpret_cast<const char*>(&p.pos), sizeof(p.pos));
			file.write(reinterpret_cast<const char*>(&p.predPos), sizeof(p.predPos));
			file.write(reinterpret_cast<const char*>(&p.vel), sizeof(p.vel));
			file.write(reinterpret_cast<const char*>(&p.prevVel), sizeof(p.prevVel));
			file.write(reinterpret_cast<const char*>(&p.predVel), sizeof(p.predVel));
			file.write(reinterpret_cast<const char*>(&p.acc), sizeof(p.acc));
			file.write(reinterpret_cast<const char*>(&p.mass), sizeof(p.mass));
			file.write(reinterpret_cast<const char*>(&p.press), sizeof(p.press));
			file.write(reinterpret_cast<const char*>(&p.pressTmp), sizeof(p.pressTmp));
			file.write(reinterpret_cast<const char*>(&p.pressF), sizeof(p.pressF));
			file.write(reinterpret_cast<const char*>(&p.dens), sizeof(p.dens));
			file.write(reinterpret_cast<const char*>(&p.predDens), sizeof(p.predDens));
			file.write(reinterpret_cast<const char*>(&p.sphMass), sizeof(p.sphMass));
			file.write(reinterpret_cast<const char*>(&p.restDens), sizeof(p.restDens));
			file.write(reinterpret_cast<const char*>(&p.stiff), sizeof(p.stiff));
			file.write(reinterpret_cast<const char*>(&p.visc), sizeof(p.visc));
			file.write(reinterpret_cast<const char*>(&p.cohesion), sizeof(p.cohesion));
			file.write(reinterpret_cast<const char*>(&p.temp), sizeof(p.temp));
			file.write(reinterpret_cast<const char*>(&p.ke), sizeof(p.ke));
			file.write(reinterpret_cast<const char*>(&p.prevKe), sizeof(p.prevKe));
			file.write(reinterpret_cast<const char*>(&p.mortonKey), sizeof(p.mortonKey));
			file.write(reinterpret_cast<const char*>(&p.id), sizeof(p.id));
			file.write(reinterpret_cast<const char*>(&p.isHotPoint), sizeof(p.isHotPoint));
			file.write(reinterpret_cast<const char*>(&p.hasSolidified), sizeof(p.hasSolidified));

			file.write(reinterpret_cast<const char*>(&r.color), sizeof(r.color));
			file.write(reinterpret_cast<const char*>(&r.pColor), sizeof(r.pColor));
			file.write(reinterpret_cast<const char*>(&r.sColor), sizeof(r.sColor));
			file.write(reinterpret_cast<const char*>(&r.sphColor), sizeof(r.sphColor));
			file.write(reinterpret_cast<const char*>(&r.size), sizeof(r.size));
			file.write(reinterpret_cast<const char*>(&r.uniqueColor), sizeof(r.uniqueColor));
			file.write(reinterpret_cast<const char*>(&r.isSolid), sizeof(r.isSolid));
			file.write(reinterpret_cast<const char*>(&r.canBeSubdivided), sizeof(r.canBeSubdivided));
			file.write(reinterpret_cast<const char*>(&r.canBeResized), sizeof(r.canBeResized));
			file.write(reinterpret_cast<const char*>(&r.isDarkMatter), sizeof(r.isDarkMatter));
			file.write(reinterpret_cast<const char*>(&r.isSPH), sizeof(r.isSPH));
			file.write(reinterpret_cast<const char*>(&r.isSelected), sizeof(r.isSelected));
			file.write(reinterpret_cast<const char*>(&r.isGrabbed), sizeof(r.isGrabbed));
			file.write(reinterpret_cast<const char*>(&r.previousSize), sizeof(r.previousSize));
			file.write(reinterpret_cast<const char*>(&r.neighbors), sizeof(r.neighbors));
			file.write(reinterpret_cast<const char*>(&r.totalRadius), sizeof(r.totalRadius));
			file.write(reinterpret_cast<const char*>(&r.lifeSpan), sizeof(r.lifeSpan));
			file.write(reinterpret_cast<const char*>(&r.sphLabel), sizeof(r.sphLabel));
			file.write(reinterpret_cast<const char*>(&r.isPinned), sizeof(r.isPinned));
			file.write(reinterpret_cast<const char*>(&r.isBeingDrawn), sizeof(r.isBeingDrawn));
			file.write(reinterpret_cast<const char*>(&r.spawnCorrectIter), sizeof(r.spawnCorrectIter));
			file.write(reinterpret_cast<const char*>(&r.turbulence), sizeof(r.turbulence));
		}

		uint32_t numConstraints = physics.particleConstraints.size();
		file.write(reinterpret_cast<const char*>(&numConstraints), sizeof(numConstraints));
		if (numConstraints > 0) {
			file.write(
				reinterpret_cast<const char*>(physics.particleConstraints.data()),
				numConstraints * sizeof(ParticleConstraint)
			);
		}

		uint32_t wallCount = lighting.walls.size();
		file.write(reinterpret_cast<const char*>(&wallCount), sizeof(wallCount));

		for (size_t i = 0; i < lighting.walls.size(); i++) {
			const Wall& w = lighting.walls[i];

			// Write all glm::vec2 fields
			file.write(reinterpret_cast<const char*>(&w.vA), sizeof(w.vA));
			file.write(reinterpret_cast<const char*>(&w.vB), sizeof(w.vB));
			file.write(reinterpret_cast<const char*>(&w.normal), sizeof(w.normal));
			file.write(reinterpret_cast<const char*>(&w.normalVA), sizeof(w.normalVA));
			file.write(reinterpret_cast<const char*>(&w.normalVB), sizeof(w.normalVB));

			// Write booleans
			file.write(reinterpret_cast<const char*>(&w.isBeingSpawned), sizeof(w.isBeingSpawned));
			file.write(reinterpret_cast<const char*>(&w.vAisBeingMoved), sizeof(w.vAisBeingMoved));
			file.write(reinterpret_cast<const char*>(&w.vBisBeingMoved), sizeof(w.vBisBeingMoved));

			// Write Color structs
			file.write(reinterpret_cast<const char*>(&w.apparentColor), sizeof(w.apparentColor));
			file.write(reinterpret_cast<const char*>(&w.baseColor), sizeof(w.baseColor));
			file.write(reinterpret_cast<const char*>(&w.specularColor), sizeof(w.specularColor));
			file.write(reinterpret_cast<const char*>(&w.refractionColor), sizeof(w.refractionColor));
			file.write(reinterpret_cast<const char*>(&w.emissionColor), sizeof(w.emissionColor));

			// Write float color values
			file.write(reinterpret_cast<const char*>(&w.baseColorVal), sizeof(w.baseColorVal));
			file.write(reinterpret_cast<const char*>(&w.specularColorVal), sizeof(w.specularColorVal));
			file.write(reinterpret_cast<const char*>(&w.refractionColorVal), sizeof(w.refractionColorVal));

			// Write material floats
			file.write(reinterpret_cast<const char*>(&w.specularRoughness), sizeof(w.specularRoughness));
			file.write(reinterpret_cast<const char*>(&w.refractionRoughness), sizeof(w.refractionRoughness));
			file.write(reinterpret_cast<const char*>(&w.refractionAmount), sizeof(w.refractionAmount));
			file.write(reinterpret_cast<const char*>(&w.IOR), sizeof(w.IOR));
			file.write(reinterpret_cast<const char*>(&w.dispersionStrength), sizeof(w.dispersionStrength));

			// Write shape metadata
			file.write(reinterpret_cast<const char*>(&w.isShapeWall), sizeof(w.isShapeWall));
			file.write(reinterpret_cast<const char*>(&w.isShapeClosed), sizeof(w.isShapeClosed));
			file.write(reinterpret_cast<const char*>(&w.shapeId), sizeof(w.shapeId));

			// Write wall ID and selection flag
			file.write(reinterpret_cast<const char*>(&w.id), sizeof(w.id));
			file.write(reinterpret_cast<const char*>(&w.isSelected), sizeof(w.isSelected));
		}

		uint32_t numShapes = lighting.shapes.size();
		file.write(reinterpret_cast<const char*>(&numShapes), sizeof(numShapes));
		if (numShapes > 0) {
			for (const Shape& s : lighting.shapes) {
				uint32_t wallIdCount = s.myWallIds.size();
				file.write(reinterpret_cast<const char*>(&wallIdCount), sizeof(wallIdCount));
				file.write(reinterpret_cast<const char*>(s.myWallIds.data()), wallIdCount * sizeof(uint32_t));

				uint32_t vertCount = s.polygonVerts.size();
				file.write(reinterpret_cast<const char*>(&vertCount), sizeof(vertCount));
				file.write(reinterpret_cast<const char*>(s.polygonVerts.data()), vertCount * sizeof(glm::vec2));

				uint32_t helpersCount = s.helpers.size();
				file.write(reinterpret_cast<const char*>(&helpersCount), sizeof(helpersCount));
				if (helpersCount > 0) {
					file.write(reinterpret_cast<const char*>(s.helpers.data()), helpersCount * sizeof(glm::vec2));
				}

				file.write(reinterpret_cast<const char*>(&s.baseColor), sizeof(s.baseColor));
				file.write(reinterpret_cast<const char*>(&s.specularColor), sizeof(s.specularColor));
				file.write(reinterpret_cast<const char*>(&s.refractionColor), sizeof(s.refractionColor));
				file.write(reinterpret_cast<const char*>(&s.emissionColor), sizeof(s.emissionColor));

				file.write(reinterpret_cast<const char*>(&s.specularRoughness), sizeof(s.specularRoughness));
				file.write(reinterpret_cast<const char*>(&s.refractionRoughness), sizeof(s.refractionRoughness));
				file.write(reinterpret_cast<const char*>(&s.refractionAmount), sizeof(s.refractionAmount));
				file.write(reinterpret_cast<const char*>(&s.IOR), sizeof(s.IOR));
				file.write(reinterpret_cast<const char*>(&s.dispersionStrength), sizeof(s.dispersionStrength));

				file.write(reinterpret_cast<const char*>(&s.id), sizeof(s.id));
				file.write(reinterpret_cast<const char*>(&s.h1), sizeof(s.h1));
				file.write(reinterpret_cast<const char*>(&s.h2), sizeof(s.h2));
				file.write(reinterpret_cast<const char*>(&s.isBeingSpawned), sizeof(s.isBeingSpawned));
				file.write(reinterpret_cast<const char*>(&s.isBeingMoved), sizeof(s.isBeingMoved));
				file.write(reinterpret_cast<const char*>(&s.isShapeClosed), sizeof(s.isShapeClosed));
				file.write(reinterpret_cast<const char*>(&s.shapeType), sizeof(s.shapeType));
				file.write(reinterpret_cast<const char*>(&s.drawHoverHelpers), sizeof(s.drawHoverHelpers));
				file.write(reinterpret_cast<const char*>(&s.oldDrawHelperPos), sizeof(s.oldDrawHelperPos));

				// Lens variables
				file.write(reinterpret_cast<const char*>(&s.secondHelper), sizeof(s.secondHelper));
				file.write(reinterpret_cast<const char*>(&s.thirdHelper), sizeof(s.thirdHelper));
				file.write(reinterpret_cast<const char*>(&s.fourthHelper), sizeof(s.fourthHelper));

				file.write(reinterpret_cast<const char*>(&s.Tempsh2Length), sizeof(s.Tempsh2Length));
				file.write(reinterpret_cast<const char*>(&s.Tempsh2LengthSymmetry), sizeof(s.Tempsh2LengthSymmetry));
				file.write(reinterpret_cast<const char*>(&s.tempDist), sizeof(s.tempDist));

				file.write(reinterpret_cast<const char*>(&s.moveH2), sizeof(s.moveH2));

				file.write(reinterpret_cast<const char*>(&s.isThirdBeingMoved), sizeof(s.isThirdBeingMoved));
				file.write(reinterpret_cast<const char*>(&s.isFourthBeingMoved), sizeof(s.isFourthBeingMoved));
				file.write(reinterpret_cast<const char*>(&s.isFifthBeingMoved), sizeof(s.isFifthBeingMoved));

				file.write(reinterpret_cast<const char*>(&s.isFifthFourthMoved), sizeof(s.isFifthFourthMoved));

				file.write(reinterpret_cast<const char*>(&s.symmetricalLens), sizeof(s.symmetricalLens));

				file.write(reinterpret_cast<const char*>(&s.wallAId), sizeof(s.wallAId));
				file.write(reinterpret_cast<const char*>(&s.wallBId), sizeof(s.wallBId));
				file.write(reinterpret_cast<const char*>(&s.wallCId), sizeof(s.wallCId));

				file.write(reinterpret_cast<const char*>(&s.lensSegments), sizeof(s.lensSegments));

				file.write(reinterpret_cast<const char*>(&s.startAngle), sizeof(s.startAngle));
				file.write(reinterpret_cast<const char*>(&s.endAngle), sizeof(s.endAngle));

				file.write(reinterpret_cast<const char*>(&s.startAngleSymmetry), sizeof(s.startAngleSymmetry));
				file.write(reinterpret_cast<const char*>(&s.endAngleSymmetry), sizeof(s.endAngleSymmetry));

				file.write(reinterpret_cast<const char*>(&s.center), sizeof(s.center));
				file.write(reinterpret_cast<const char*>(&s.radius), sizeof(s.radius));

				file.write(reinterpret_cast<const char*>(&s.centerSymmetry), sizeof(s.centerSymmetry));
				file.write(reinterpret_cast<const char*>(&s.radiusSymmetry), sizeof(s.radiusSymmetry));

				file.write(reinterpret_cast<const char*>(&s.arcEnd), sizeof(s.arcEnd));

				file.write(reinterpret_cast<const char*>(&s.globalLensPrev), sizeof(s.globalLensPrev));
			}
		}

		uint32_t pointLightCount = lighting.pointLights.size();
		file.write(reinterpret_cast<const char*>(&pointLightCount), sizeof(pointLightCount));

		for (const PointLight& pl : lighting.pointLights) {
			file.write(reinterpret_cast<const char*>(&pl.pos), sizeof(pl.pos));
			file.write(reinterpret_cast<const char*>(&pl.isBeingMoved), sizeof(pl.isBeingMoved));
			file.write(reinterpret_cast<const char*>(&pl.color), sizeof(pl.color));
			file.write(reinterpret_cast<const char*>(&pl.apparentColor), sizeof(pl.apparentColor));
			file.write(reinterpret_cast<const char*>(&pl.isSelected), sizeof(pl.isSelected));
		}

		uint32_t areaLightCount = lighting.areaLights.size();
		file.write(reinterpret_cast<const char*>(&areaLightCount), sizeof(areaLightCount));

		for (const AreaLight& al : lighting.areaLights) {
			file.write(reinterpret_cast<const char*>(&al.vA), sizeof(al.vA));
			file.write(reinterpret_cast<const char*>(&al.vB), sizeof(al.vB));
			file.write(reinterpret_cast<const char*>(&al.isBeingSpawned), sizeof(al.isBeingSpawned));
			file.write(reinterpret_cast<const char*>(&al.vAisBeingMoved), sizeof(al.vAisBeingMoved));
			file.write(reinterpret_cast<const char*>(&al.vBisBeingMoved), sizeof(al.vBisBeingMoved));
			file.write(reinterpret_cast<const char*>(&al.color), sizeof(al.color));
			file.write(reinterpret_cast<const char*>(&al.apparentColor), sizeof(al.apparentColor));
			file.write(reinterpret_cast<const char*>(&al.isSelected), sizeof(al.isSelected));
			file.write(reinterpret_cast<const char*>(&al.spread), sizeof(al.spread));
		}

		uint32_t coneLightCount = lighting.coneLights.size();
		file.write(reinterpret_cast<const char*>(&coneLightCount), sizeof(coneLightCount));

		for (const ConeLight& cl : lighting.coneLights) {
			file.write(reinterpret_cast<const char*>(&cl.vA), sizeof(cl.vA));
			file.write(reinterpret_cast<const char*>(&cl.vB), sizeof(cl.vB));
			file.write(reinterpret_cast<const char*>(&cl.isBeingSpawned), sizeof(cl.isBeingSpawned));
			file.write(reinterpret_cast<const char*>(&cl.vAisBeingMoved), sizeof(cl.vAisBeingMoved));
			file.write(reinterpret_cast<const char*>(&cl.vBisBeingMoved), sizeof(cl.vBisBeingMoved));
			file.write(reinterpret_cast<const char*>(&cl.color), sizeof(cl.color));
			file.write(reinterpret_cast<const char*>(&cl.apparentColor), sizeof(cl.apparentColor));
			file.write(reinterpret_cast<const char*>(&cl.isSelected), sizeof(cl.isSelected));
			file.write(reinterpret_cast<const char*>(&cl.spread), sizeof(cl.spread));
		}

		file.close();
	}

	deserializeParticleSystem(filename, yamlString, myVar, myParam, sph, physics, lighting, loadFlag);
}

void SaveSystem::saveLoadLogic(UpdateVariables& myVar, UpdateParameters& myParam, SPH& sph, Physics& physics, Lighting& lighting, Field& field) {
	if (saveFlag) {
		if (!std::filesystem::exists("Saves")) {
			std::filesystem::create_directory("Saves");
		}

		int nextAvailableIndex = 0;
		for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator("Saves")) {

			std::string filename = entry.path().filename().string();
			if (filename.rfind("Save_", 0) == 0 && filename.find(".bin") != std::string::npos) {

				size_t startPos = filename.find_last_of('_') + 1;
				size_t endPos = filename.find(".bin");
				int index = std::stoi(filename.substr(startPos, endPos - startPos));

				if (index >= nextAvailableIndex) {
					nextAvailableIndex = index + 1;
				}
			}
		}

		std::string savePath = "Saves/Save_" + std::to_string(nextAvailableIndex) + ".bin";

		saveSystem(savePath.c_str(), myVar, myParam, sph, physics, lighting, field);

		saveIndex++;

		saveFlag = false;
	}

	if (loadFlag) {

		int fileIndex = 1;

		filePaths.clear();

		if (!std::filesystem::exists("Saves")) {
			std::filesystem::create_directory("Saves");
			std::cout << "Created Saves directory as it did not exist" << std::endl;
			loadFlag = false;
			return;
		}

		std::vector<std::pair<std::string, std::string>> files;
		for (const auto& entry : std::filesystem::recursive_directory_iterator("Saves")) {
			if (entry.is_regular_file()) {
				const auto filename = entry.path().filename().string();
				const auto fullpath = entry.path().string();
				if (filename.rfind(".bin") != std::string::npos) {
					std::string relativePath = fullpath.substr(6);
					files.emplace_back(relativePath, fullpath);
				}
			}
		}

		if (files.empty()) {
			std::cout << "No .bin files found in Saves directory" << std::endl;
			loadFlag = false;
			return;
		}

		std::sort(files.begin(), files.end(),
			[](auto& A, auto& B) {
				const std::string& nameA = A.first;
				const std::string& nameB = B.first;

				auto isDefault = [](const std::string& s) {
					const std::string key = "DefaultSettings.bin";

					return s.size() >= key.size()
						&& s.compare(s.size() - key.size(), key.size(), key) == 0;
					};
				bool aIsDefault = isDefault(nameA);
				bool bIsDefault = isDefault(nameB);
				if (aIsDefault != bIsDefault) {
					return aIsDefault;
				}

				auto getDir = [](const std::string& s) {
					size_t pos = s.find_last_of("\\/");
					return (pos == std::string::npos) ? std::string{} : s.substr(0, pos);
					};
				std::string dirA = getDir(nameA);
				std::string dirB = getDir(nameB);
				if (dirA != dirB) {
					return dirA < dirB;
				}

				auto extractNumber = [](const std::string& s) -> int {
					size_t i = 0;
					while (i < s.size() && !std::isdigit(s[i])) ++i;
					size_t j = i;
					while (j < s.size() && std::isdigit(s[j])) ++j;
					if (i < j) {
						try { return std::stoi(s.substr(i, j - i)); }
						catch (...) {}
					}
					return -1;
					};
				int numA = extractNumber(nameA);
				int numB = extractNumber(nameB);
				if (numA >= 0 && numB >= 0 && numA != numB) {
					return numA < numB;
				}
				return nameA < nameB;
			}
		);

		ImGui::SetNextWindowSize(loadMenuSize, ImGuiCond_Once);
		ImGui::SetNextWindowPos(ImVec2(static_cast<float>(GetScreenWidth()) * 0.5f - loadMenuSize.x * 0.5f, 450.0f), ImGuiCond_Once);
		ImGui::Begin("Files");

		for (const auto& [filename, fullPath] : files) {

			bool placeHolder = false;
			bool enabled = true;

			if (UI::buttonHelper(fullPath.c_str(), "Select scene file", placeHolder, ImGui::GetContentRegionAvail().x, buttonHeight, enabled, enabled)) {
				saveSystem(fullPath.c_str(), myVar, myParam, sph, physics, lighting, field);
				loadFlag = false;
			}

			filePaths.push_back(fullPath);
			fileIndex++;
		}

		ImGui::End();
	}
}