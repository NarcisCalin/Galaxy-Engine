#pragma once
#include "../raylib/raylib.h"
#include "../Particles/particle.h"
#include "../parameters.h"
#include "../Physics/SPH.h"
#include "../../external/imgui/imgui.h"
#include "../../external/imgui/rlImGui.h"
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <filesystem>

class SaveSystem {

public:

	bool saveFlag = false;
	bool loadFlag = false;

	void saveSimulation(const std::string& filename, const UpdateVariables& myVar, const UpdateParameters& myParam, const SPH& sph) {
		std::ofstream out(filename);
		if (!out) return;

#define WRITE(k, v) out << #k << " " << v << "\n"

		WRITE(GlobalTrails, myVar.isGlobalTrailsEnabled);
		WRITE(SelectedTrails, myVar.isSelectedTrailsEnabled);
		WRITE(LocalTrails, myVar.isLocalTrailsEnabled);
		WRITE(WhiteTrails, myParam.trails.whiteTrails);
		WRITE(SolidColor, myParam.colorVisuals.solidColor);
		WRITE(DensityColor, myParam.colorVisuals.densityColor);
		WRITE(ForceColor, myParam.colorVisuals.forceColor);
		WRITE(VelocityColor, myParam.colorVisuals.velocityColor);
		WRITE(DeltaVColor, myParam.colorVisuals.deltaVColor);
		WRITE(PressureColor, myParam.colorVisuals.pressureColor);
		WRITE(SPHColor, myParam.colorVisuals.SPHColor);
		WRITE(SelectedColor, myParam.colorVisuals.selectedColor);
		WRITE(DarkMatter, myVar.isDarkMatterEnabled);
		WRITE(ShowDarkMatter, myParam.colorVisuals.showDarkMatterEnabled);
		WRITE(PrevDarkMatter, myParam.colorVisuals.previousDarkMatterState);
		WRITE(LoopingSpace, myVar.isPeriodicBoundaryEnabled);
		WRITE(SPHEnabled, myVar.isSPHEnabled);
		WRITE(Collisions, myVar.isCollisionsEnabled);
		WRITE(DensitySize, myVar.isDensitySizeEnabled);
		WRITE(ForceSize, myVar.isForceSizeEnabled);
		WRITE(Glow, myVar.isGlowEnabled);
		WRITE(ShipGas, myVar.isShipGasEnabled);
		WRITE(SPHWater, myParam.brush.SPHWater);
		WRITE(SPHRock, myParam.brush.SPHRock);
		WRITE(SPHSand, myParam.brush.SPHSand);
		WRITE(SPHMud, myParam.brush.SPHMud);
		WRITE(PrimaryR, myParam.colorVisuals.primaryR);
		WRITE(PrimaryG, myParam.colorVisuals.primaryG);
		WRITE(PrimaryB, myParam.colorVisuals.primaryB);
		WRITE(PrimaryA, myParam.colorVisuals.primaryA);
		WRITE(SecondaryR, myParam.colorVisuals.secondaryR);
		WRITE(SecondaryG, myParam.colorVisuals.secondaryG);
		WRITE(SecondaryB, myParam.colorVisuals.secondaryB);
		WRITE(SecondaryA, myParam.colorVisuals.secondaryA);
		WRITE(DensityRadius, myParam.neighborSearch.densityRadius);
		WRITE(MaxNeighbors, myParam.colorVisuals.maxNeighbors);
		WRITE(MaxColorForce, myParam.colorVisuals.maxColorAcc);
		WRITE(MaxSizeForce, myParam.densitySize.sizeAcc);
		WRITE(DeltaVMaxAcc, myParam.colorVisuals.deltaVMaxAccel);
		WRITE(TrailsMaxLength, myVar.trailMaxLength);
		WRITE(TrailsThickness, myParam.trails.trailThickness);
		WRITE(ParticleSizeMult, myVar.particleSizeMultiplier);
		WRITE(VisiblePAmountMult, myParam.particlesSpawning.particleAmountMultiplier);
		WRITE(DMPAmountMult, myParam.particlesSpawning.DMAmountMultiplier);
		WRITE(Softening, myVar.softening);
		WRITE(Theta, myVar.theta);
		WRITE(TimeMult, myVar.timeStepMultiplier);
		WRITE(CollisionSubsteps, myVar.substeps);
		WRITE(GravityMultiplier, myVar.gravityMultiplier);
		WRITE(HeavyParticlesMass, myParam.particlesSpawning.heavyParticleWeightMultiplier);
		WRITE(CollisionBounciness, myVar.particleBounciness);
		WRITE(SPHRestDens, sph.restDensity);
		WRITE(SPHRadiusMult, sph.radiusMultiplier);
		WRITE(SPHMass, sph.mass);
		WRITE(SPHViscosity, sph.viscosity);
		WRITE(SPHCohesion, sph.cohesionCoefficient);
		WRITE(SPHGround, myVar.sphGround);
		WRITE(SPHDelta, sph.delta);
		WRITE(SPHMaxVel, myVar.sphMaxVel);
		WRITE(DomainWidth, myVar.domainSize.x);
		WRITE(DomainHeight, myVar.domainSize.y);
		WRITE(CameraTarget, myParam.myCamera.camera.target);
		WRITE(CameraOffset, myParam.myCamera.camera.offset);
		WRITE(CameraZoom, myParam.myCamera.camera.zoom);
		WRITE(ColorMaxVel, myParam.colorVisuals.maxVel);
		WRITE(ColorMaxPressure, myParam.colorVisuals.maxPress);

#undef WRITE

		out << "numParticles " << myParam.pParticles.size() << "\n";
		for (auto const& p : myParam.pParticles) out << "P " << p << "\n";
		for (auto const& r : myParam.rParticles) out << "R " << r << "\n";
	}

	void loadSimulation(const std::string& filename, UpdateVariables& myVar, UpdateParameters& myParam, SPH& sph) {
		std::ifstream in(filename);
		if (!in) {
			std::cout << "Failed to open file for loading: " << filename << std::endl;
			return;
		}

		std::cout << "Loading from: " << filename << std::endl;

		myParam.pParticles.clear();
		myParam.rParticles.clear();

		std::string key;
		while (in >> key) {
#define READ(k, v) else if (key == #k) in >> v
			if (false);
			READ(GlobalTrails, myVar.isGlobalTrailsEnabled);
			READ(SelectedTrails, myVar.isSelectedTrailsEnabled);
			READ(LocalTrails, myVar.isLocalTrailsEnabled);
			READ(WhiteTrails, myParam.trails.whiteTrails);
			READ(SolidColor, myParam.colorVisuals.solidColor);
			READ(DensityColor, myParam.colorVisuals.densityColor);
			READ(ForceColor, myParam.colorVisuals.forceColor);
			READ(VelocityColor, myParam.colorVisuals.velocityColor);
			READ(DeltaVColor, myParam.colorVisuals.deltaVColor);
			READ(PressureColor, myParam.colorVisuals.pressureColor);
			READ(SPHColor, myParam.colorVisuals.SPHColor);
			READ(SelectedColor, myParam.colorVisuals.selectedColor);
			READ(DarkMatter, myVar.isDarkMatterEnabled);
			READ(ShowDarkMatter, myParam.colorVisuals.showDarkMatterEnabled);
			READ(PrevDarkMatter, myParam.colorVisuals.previousDarkMatterState);
			READ(LoopingSpace, myVar.isPeriodicBoundaryEnabled);
			READ(SPHEnabled, myVar.isSPHEnabled);
			READ(Collisions, myVar.isCollisionsEnabled);
			READ(DensitySize, myVar.isDensitySizeEnabled);
			READ(ForceSize, myVar.isForceSizeEnabled);
			READ(Glow, myVar.isGlowEnabled);
			READ(ShipGas, myVar.isShipGasEnabled);
			READ(SPHWater, myParam.brush.SPHWater);
			READ(SPHRock, myParam.brush.SPHRock);
			READ(SPHSand, myParam.brush.SPHSand);
			READ(SPHMud, myParam.brush.SPHMud);
			READ(PrimaryR, myParam.colorVisuals.primaryR);
			READ(PrimaryG, myParam.colorVisuals.primaryG);
			READ(PrimaryB, myParam.colorVisuals.primaryB);
			READ(PrimaryA, myParam.colorVisuals.primaryA);
			READ(SecondaryR, myParam.colorVisuals.secondaryR);
			READ(SecondaryG, myParam.colorVisuals.secondaryG);
			READ(SecondaryB, myParam.colorVisuals.secondaryB);
			READ(SecondaryA, myParam.colorVisuals.secondaryA);
			READ(DensityRadius, myParam.neighborSearch.densityRadius);
			READ(MaxNeighbors, myParam.colorVisuals.maxNeighbors);
			READ(MaxColorForce, myParam.colorVisuals.maxColorAcc);
			READ(MaxSizeForce, myParam.densitySize.sizeAcc);
			READ(DeltaVMaxAcc, myParam.colorVisuals.deltaVMaxAccel);
			READ(TrailsMaxLength, myVar.trailMaxLength);
			READ(TrailsThickness, myParam.trails.trailThickness);
			READ(ParticleSizeMult, myVar.particleSizeMultiplier);
			READ(VisiblePAmountMult, myParam.particlesSpawning.particleAmountMultiplier);
			READ(DMPAmountMult, myParam.particlesSpawning.DMAmountMultiplier);
			READ(Softening, myVar.softening);
			READ(Theta, myVar.theta);
			READ(TimeMult, myVar.timeStepMultiplier);
			READ(CollisionSubsteps, myVar.substeps);
			READ(GravityMultiplier, myVar.gravityMultiplier);
			READ(HeavyParticlesMass, myParam.particlesSpawning.heavyParticleWeightMultiplier);
			READ(CollisionBounciness, myVar.particleBounciness);
			READ(SPHRestDens, sph.restDensity);
			READ(SPHRadiusMult, sph.radiusMultiplier);
			READ(SPHMass, sph.mass);
			READ(SPHViscosity, sph.viscosity);
			READ(SPHCohesion, sph.cohesionCoefficient);
			READ(SPHGround, myVar.sphGround);
			READ(SPHDelta, sph.delta);
			READ(SPHMaxVel, myVar.sphMaxVel);
			READ(DomainWidth, myVar.domainSize.x);
			READ(DomainHeight, myVar.domainSize.y);
			READ(CameraTarget, myParam.myCamera.camera.target);
			READ(CameraOffset, myParam.myCamera.camera.offset);
			READ(CameraZoom, myParam.myCamera.camera.zoom);
			READ(ColorMaxVel, myParam.colorVisuals.maxVel);
			READ(ColorMaxPressure, myParam.colorVisuals.maxPress);
			
#undef READ
			else if (key == "numParticles") {
				size_t n;
				in >> n;
				myParam.pParticles.reserve(n);
				myParam.rParticles.reserve(n);
			}
			else if (key == "P") {
				ParticlePhysics p;
				in >> p;
				myParam.pParticles.push_back(p);
			}
			else if (key == "R") {
				ParticleRendering r;
				in >> r;
				myParam.rParticles.push_back(r);
			}
		}
		std::cout << "Successfully loaded " << myParam.pParticles.size() << " particles" << std::endl;
	}

	void saveLoadLogic(UpdateVariables& myVar, UpdateParameters& myParam, SPH& sph) {
		if (saveFlag) {
			if (!std::filesystem::exists("Saves")) {
				std::filesystem::create_directory("Saves");
			}

			int nextAvailableIndex = 0;
			for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator("Saves")) {

				std::string filename = entry.path().filename().string();
				if (filename.rfind("Save_", 0) == 0 && filename.find(".galaxsim") != std::string::npos) {

					size_t startPos = filename.find_last_of('_') + 1;
					size_t endPos = filename.find(".galaxsim");
					int index = std::stoi(filename.substr(startPos, endPos - startPos));

					if (index >= nextAvailableIndex) {
						nextAvailableIndex = index + 1;
					}
				}
			}

			std::string savePath = "Saves/Save_" + std::to_string(nextAvailableIndex) + ".galaxsim";

			saveSimulation(savePath.c_str(), myVar, myParam, sph);

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
					if (filename.rfind(".galaxsim") != std::string::npos) {
						std::string relativePath = fullpath.substr(6);
						files.emplace_back(relativePath, fullpath);
					}
				}
			}

			if (files.empty()) {
				std::cout << "No .galaxsim files found in Saves directory" << std::endl;
				loadFlag = false;
				return;
			}

			std::sort(files.begin(), files.end(),
				[](auto& A, auto& B) {
					const std::string& nameA = A.first;
					const std::string& nameB = B.first;

					auto isDefault = [](const std::string& s) {
						const std::string key = "DefaultSettings.galaxsim";

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

			float filesAmount = static_cast<float>(files.size());

			ImGui::SetNextWindowSize(loadMenuSize, ImGuiCond_Once);
			ImGui::SetNextWindowPos(ImVec2(static_cast<float>(GetScreenWidth()) * 0.5f - loadMenuSize.x * 0.5f, 450.0f), ImGuiCond_Once);
			ImGui::Begin("Files");

			for (const auto& [filename, fullPath] : files) {
				
				if (ImGui::Button(fullPath.c_str(), ImVec2(ImGui::GetContentRegionAvail().x, buttonHeight))) {
					loadSimulation(fullPath.c_str(), myVar, myParam, sph);
					loadFlag = false;
				}

				filePaths.push_back(fullPath);
				fileIndex++;
			}

			ImGui::End();
		}
	}

private:

	ImVec2 loadMenuSize = { 600.0f, 500.0f };
	float buttonHeight = 30.0f;

	float buttonsGap = 5.0f;

	std::vector<std::string> filePaths;

	int saveIndex = 0;
};