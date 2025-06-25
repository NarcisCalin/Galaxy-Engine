#include "UX/saveSystem.h"

void SaveSystem::saveSystem(const std::string& filename, UpdateVariables& myVar, UpdateParameters& myParam, SPH& sph, Physics& physics) {

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
	paramIO(filename, out, "PressureColor", myParam.colorVisuals.pressureColor);
	paramIO(filename, out, "SPHColor", myParam.colorVisuals.SPHColor);
	paramIO(filename, out, "SelectedColor", myParam.colorVisuals.selectedColor);
	paramIO(filename, out, "ShowDarkMatter", myParam.colorVisuals.showDarkMatterEnabled);

	// ----- Color sliders -----
	paramIO(filename, out, "ColorMaxVel", myParam.colorVisuals.maxVel);
	paramIO(filename, out, "ColorMaxPressure", myParam.colorVisuals.maxPress);
	paramIO(filename, out, "MaxColorForce", myParam.colorVisuals.maxColorAcc);
	paramIO(filename, out, "ShockwaveMaxAcc", myParam.colorVisuals.ShockwaveMaxAcc);
	paramIO(filename, out, "TemperatureColor", myParam.colorVisuals.temperatureColor);
	paramIO(filename, out, "TemperatureGasColor", myParam.colorVisuals.gasTempColor);
	paramIO(filename, out, "MaxTemperatureColor", myParam.colorVisuals.tempColorMaxTemp);
	paramIO(filename, out, "MaxNeighbors", myParam.colorVisuals.maxNeighbors);

	// ----- Other visual sliders -----
	paramIO(filename, out, "DensityRadius", myParam.neighborSearch.densityRadius);
	paramIO(filename, out, "MaxSizeForce", myParam.densitySize.sizeAcc);
	paramIO(filename, out, "ParticleSizeMult", myVar.particleSizeMultiplier);
	paramIO(filename, out, "VisiblePAmountMult", myParam.particlesSpawning.particleAmountMultiplier);
	paramIO(filename, out, "DMPAmountMult", myParam.particlesSpawning.DMAmountMultiplier);
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
	paramIO(filename, out, "HeavyParticlesMass", myParam.particlesSpawning.heavyParticleWeightMultiplier);
	paramIO(filename, out, "TemperatureSimulation", myVar.isTempEnabled);
	paramIO(filename, out, "AmbientTemperature", myVar.ambientTemp);
	paramIO(filename, out, "AmbientHeatRate", myVar.globalAmbientHeatRate);
	paramIO(filename, out, "HeatConductivityMultiplier", myVar.globalHeatConductivity);

	// ----- SPH -----
	paramIO(filename, out, "SPHGravity", sph.verticalGravity);
	paramIO(filename, out, "SPHRadiusMult", sph.radiusMultiplier);
	paramIO(filename, out, "SPHMass", sph.mass);
	paramIO(filename, out, "SPHViscosity", sph.viscosity);
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

			uint32_t numNeighbors = p.neighborIds.size();
			file.write(reinterpret_cast<const char*>(&numNeighbors), sizeof(numNeighbors));
			if (numNeighbors > 0) {
				file.write(reinterpret_cast<const char*>(p.neighborIds.data()),
					numNeighbors * sizeof(uint32_t));
			}

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
		}

		uint32_t numConstraints = physics.particleConstraints.size();
		file.write(reinterpret_cast<const char*>(&numConstraints), sizeof(numConstraints));
		if (numConstraints > 0) {
			file.write(
				reinterpret_cast<const char*>(physics.particleConstraints.data()),
				numConstraints * sizeof(ParticleConstraint)
			);
		}

		file.close();
	}

	deserializeParticleSystem(filename, yamlString, myVar, myParam, sph, physics, loadFlag);
}

void SaveSystem::saveLoadLogic(UpdateVariables& myVar, UpdateParameters& myParam, SPH& sph, Physics& physics) {
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

		saveSystem(savePath.c_str(), myVar, myParam, sph, physics);

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
				saveSystem(fullPath.c_str(), myVar, myParam, sph, physics);
				loadFlag = false;
			}

			filePaths.push_back(fullPath);
			fileIndex++;
		}

		ImGui::End();
	}
}