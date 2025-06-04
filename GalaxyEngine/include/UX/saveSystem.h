#pragma once

#include "Particles/particle.h"

#include "Physics/SPH.h"

#include "parameters.h"

inline std::ostream& operator<<(std::ostream& os, const Vector2& vec) {
	return os << vec.x << " " << vec.y;
}

inline std::istream& operator>>(std::istream& is, Vector2& vec) {
	return is >> vec.x >> vec.y;
}

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
		WRITE(DeltaVColor, myParam.colorVisuals.shockwaveColor);
		WRITE(PressureColor, myParam.colorVisuals.pressureColor);
		WRITE(SPHColor, myParam.colorVisuals.SPHColor);
		WRITE(SelectedColor, myParam.colorVisuals.selectedColor);
		WRITE(DarkMatter, myVar.isDarkMatterEnabled);
		WRITE(ShowDarkMatter, myParam.colorVisuals.showDarkMatterEnabled);
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
		WRITE(SPHSoil, myParam.brush.SPHSoil);
		WRITE(pColors, myParam.colorVisuals.pColor);
		WRITE(sColors, myParam.colorVisuals.sColor);
		WRITE(DensityRadius, myParam.neighborSearch.densityRadius);
		WRITE(MaxNeighbors, myParam.colorVisuals.maxNeighbors);
		WRITE(MaxColorForce, myParam.colorVisuals.maxColorAcc);
		WRITE(MaxSizeForce, myParam.densitySize.sizeAcc);
		WRITE(DeltaVMaxAcc, myParam.colorVisuals.ShockwaveMaxAcc);
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
		WRITE(TemperatureSimulation, myVar.isTempEnabled);
		WRITE(TemperatureColor, myParam.colorVisuals.temperatureColor);
		WRITE(TemperatureGasColor, myParam.colorVisuals.gasTempColor);
		WRITE(MaxTemperatureColor, myParam.colorVisuals.tempColorMaxTemp);
		WRITE(AmbientTemperature, myVar.ambientTemp);
		WRITE(AmbientHeatRate, myVar.globalAmbientHeatRate);
		WRITE(HeatConductivityMultiplier, myVar.globalHeatConductivity);

#undef WRITE

		out << "numParticles " << myParam.pParticles.size() << "\n";
		for (auto const& p : myParam.pParticles) out << "P " << p << "\n";
		for (auto const& r : myParam.rParticles) out << "R " << r << "\n";
	}


	template <typename T>
	bool READ(std::ifstream& in, std::string key, std::string paramName, T& param) {
		if (key == paramName) {
			if (in >> param) {
				std::cout << "Successfully read " << paramName << ": " << param << std::endl;
				return true;
			}
			else {
				std::cout << "Error reading parameter: " << paramName << std::endl;
				in.clear(); // Clear error flags

				if constexpr (std::is_same_v<T, float>) {
					// This entire block is only *compiled* when T is float.
					param = 20.0f;
				}
				// Skip the rest of the current line to avoid infinite loop
				std::string line;
				std::getline(in, line);

				return false;
			}
		}
		return false;
	}

	void loadSimulation(const std::string& filename, UpdateVariables& myVar, UpdateParameters& myParam, SPH& sph) {
		std::ifstream in(filename);
		if (!in) {
			std::cout << "Failed to open file for loading: " << filename << std::endl;
			return;
		}

		myParam.pParticles.clear();
		myParam.rParticles.clear();

		std::string key;
		while (in >> key) {
			READ(in, key, "GlobalTrails", myVar.isGlobalTrailsEnabled);
			READ(in, key, "SelectedTrails", myVar.isSelectedTrailsEnabled);
			READ(in, key, "LocalTrails", myVar.isLocalTrailsEnabled);
			READ(in, key, "WhiteTrails", myParam.trails.whiteTrails);
			READ(in, key, "SolidColor", myParam.colorVisuals.solidColor);
			READ(in, key, "DensityColor", myParam.colorVisuals.densityColor);
			READ(in, key, "ForceColor", myParam.colorVisuals.forceColor);
			READ(in, key, "VelocityColor", myParam.colorVisuals.velocityColor);
			READ(in, key, "DeltaVColor", myParam.colorVisuals.shockwaveColor);
			READ(in, key, "PressureColor", myParam.colorVisuals.pressureColor);
			READ(in, key, "SPHColor", myParam.colorVisuals.SPHColor);
			READ(in, key, "SelectedColor", myParam.colorVisuals.selectedColor);
			READ(in, key, "DarkMatter", myVar.isDarkMatterEnabled);
			READ(in, key, "ShowDarkMatter", myParam.colorVisuals.showDarkMatterEnabled);
			READ(in, key, "LoopingSpace", myVar.isPeriodicBoundaryEnabled);
			READ(in, key, "SPHEnabled", myVar.isSPHEnabled);
			READ(in, key, "Collisions", myVar.isCollisionsEnabled);
			READ(in, key, "DensitySize", myVar.isDensitySizeEnabled);
			READ(in, key, "ForceSize", myVar.isForceSizeEnabled);
			READ(in, key, "Glow", myVar.isGlowEnabled);
			READ(in, key, "ShipGas", myVar.isShipGasEnabled);
			READ(in, key, "SPHWater", myParam.brush.SPHWater);
			READ(in, key, "SPHRock", myParam.brush.SPHRock);
			READ(in, key, "SPHSand", myParam.brush.SPHSand);
			READ(in, key, "SPHSoil", myParam.brush.SPHSoil);
			READ(in, key, "SPHIce", myParam.brush.SPHIce);
			READ(in, key, "SPHMud", myParam.brush.SPHMud);
			READ(in, key, "pColors", myParam.colorVisuals.pColor);
			READ(in, key, "sColors", myParam.colorVisuals.sColor);
			READ(in, key, "DensityRadius", myParam.neighborSearch.densityRadius);
			READ(in, key, "MaxNeighbors", myParam.colorVisuals.maxNeighbors);
			READ(in, key, "MaxColorForce", myParam.colorVisuals.maxColorAcc);
			READ(in, key, "MaxSizeForce", myParam.densitySize.sizeAcc);
			READ(in, key, "DeltaVMaxAcc", myParam.colorVisuals.ShockwaveMaxAcc);
			READ(in, key, "TrailsMaxLength", myVar.trailMaxLength);
			READ(in, key, "TrailsThickness", myParam.trails.trailThickness);
			READ(in, key, "ParticleSizeMult", myVar.particleSizeMultiplier);
			READ(in, key, "VisiblePAmountMult", myParam.particlesSpawning.particleAmountMultiplier);
			READ(in, key, "DMPAmountMult", myParam.particlesSpawning.DMAmountMultiplier);
			READ(in, key, "Softening", myVar.softening);
			READ(in, key, "Theta", myVar.theta);
			READ(in, key, "TimeMult", myVar.timeStepMultiplier);
			READ(in, key, "CollisionSubsteps", myVar.substeps);
			READ(in, key, "GravityMultiplier", myVar.gravityMultiplier);
			READ(in, key, "HeavyParticlesMass", myParam.particlesSpawning.heavyParticleWeightMultiplier);
			READ(in, key, "CollisionBounciness", myVar.particleBounciness);
			READ(in, key, "SPHRadiusMult", sph.radiusMultiplier);
			READ(in, key, "SPHMass", sph.mass);
			READ(in, key, "SPHViscosity", sph.viscosity);
			READ(in, key, "SPHCohesion", sph.cohesionCoefficient);
			READ(in, key, "SPHGround", myVar.sphGround);
			READ(in, key, "SPHDelta", sph.delta);
			READ(in, key, "SPHMaxVel", myVar.sphMaxVel);
			READ(in, key, "DomainWidth", myVar.domainSize.x);
			READ(in, key, "DomainHeight", myVar.domainSize.y);
			READ(in, key, "CameraTarget", myParam.myCamera.camera.target);
			READ(in, key, "CameraOffset", myParam.myCamera.camera.offset);
			READ(in, key, "CameraZoom", myParam.myCamera.camera.zoom);
			READ(in, key, "ColorMaxVel", myParam.colorVisuals.maxVel);
			READ(in, key, "ColorMaxPressure", myParam.colorVisuals.maxPress);
			READ(in, key, "TemperatureSimulation", myVar.isTempEnabled);
			READ(in, key, "TemperatureColor", myParam.colorVisuals.temperatureColor);
			READ(in, key, "TemperatureGasColor", myParam.colorVisuals.gasTempColor);
			READ(in, key, "MaxTemperatureColor", myParam.colorVisuals.tempColorMaxTemp);
			READ(in, key, "AmbientTemperature", myVar.ambientTemp);
			READ(in, key, "AmbientHeatRate", myVar.globalAmbientHeatRate);
			READ(in, key, "HeatConductivityMultiplier", myVar.globalHeatConductivity);


			if (key == "numParticles") {
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

	std::vector<std::string> filePaths;

	int saveIndex = 0;
};