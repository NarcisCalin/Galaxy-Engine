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

    template <typename T>
    void paramIO(const std::string& filename, YAML::Emitter& out, std::string key, T& value) {
        int ucToInt = 0;

        if constexpr (!std::is_same_v<T, unsigned char>) {
            if (saveFlag) {
                out << YAML::BeginMap;
                out << YAML::Key << key;
                out << YAML::Value << value;
                std::cout << "Parameter: " << key.c_str() << " | Value: " << value << std::endl;
                out << YAML::EndMap;
            }
        }
        else {
            ucToInt = static_cast<int>(value);
            if (saveFlag) {
                out << YAML::BeginMap;
                out << YAML::Key << key;
                out << YAML::Value << ucToInt;
                std::cout << "Parameter: " << key.c_str() << " | Value: " << ucToInt << std::endl;
                out << YAML::EndMap;
            }
        }

        if (loadFlag && !saveFlag) {
            std::ifstream inFile(filename, std::ios::in | std::ios::binary);
            if (!inFile.is_open()) {
                std::cerr << "Failed to open file for reading: " << filename << std::endl;
                return;
            }

            const std::string sepToken = "---PARTICLE BINARY DATA---";
            std::string   line;
            std::string   yamlText;
            bool          sawSeparator = false;

            while (std::getline(inFile, line)) {
                if (line.find(sepToken) != std::string::npos) {
                    sawSeparator = true;
                    break;
                }
                yamlText += line + "\n";
            }

            inFile.close();

            try {
                std::vector<YAML::Node> documents = YAML::LoadAll(yamlText);
                bool found = false;

                for (const auto& doc : documents) {
                    if (!doc || !doc[key]) continue;

                    if constexpr (!std::is_same_v<T, unsigned char>) {
                        value = doc[key].as<T>();
                        std::cout << "Loaded " << key << ": " << value << std::endl;
                    }
                    else {
                        int tempInt = doc[key].as<int>();
                        value = static_cast<unsigned char>(tempInt);
                        std::cout << "Loaded " << key << ": " << tempInt << std::endl;
                    }

                    found = true;
                    break;
                }

                if (!found) {
                    std::cerr << "No " << key << " found in any YAML document in the file!" << std::endl;
                }
            }
            catch (const YAML::Exception& e) {
                std::cerr << "YAML error while loading key \"" << key << "\": " << e.what() << std::endl;
            }
        }
    }

    void saveSystem(const std::string& filename, UpdateVariables& myVar, UpdateParameters& myParam, SPH& sph) {

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
        paramIO(filename, out, "Collisions", myVar.isCollisionsEnabled);
        paramIO(filename, out, "DensitySize", myVar.isDensitySizeEnabled);
        paramIO(filename, out, "ForceSize", myVar.isForceSizeEnabled);
        paramIO(filename, out, "Glow", myVar.isGlowEnabled);
        paramIO(filename, out, "ShipGas", myVar.isShipGasEnabled);

        // ----- SPH Materials -----
        paramIO(filename, out, "SPHWater", myParam.brush.SPHWater);
        paramIO(filename, out, "SPHRock", myParam.brush.SPHRock);
        paramIO(filename, out, "SPHSand", myParam.brush.SPHSand);
        paramIO(filename, out, "SPHSoil", myParam.brush.SPHSoil);
        paramIO(filename, out, "SPHIce", myParam.brush.SPHIce);
        paramIO(filename, out, "SPHMud", myParam.brush.SPHMud);

        // ----- Physics params -----
        paramIO(filename, out, "Softening", myVar.softening);
        paramIO(filename, out, "Theta", myVar.theta);
        paramIO(filename, out, "TimeMult", myVar.timeStepMultiplier);
        paramIO(filename, out, "CollisionSubsteps", myVar.substeps);
        paramIO(filename, out, "GravityMultiplier", myVar.gravityMultiplier);
        paramIO(filename, out, "CollisionBounciness", myVar.particleBounciness);
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

        /*out << YAML::EndMap;*/

        std::string yamlString = out.c_str();

        std::fstream file;
        if (saveFlag) {
            file.open(filename, std::ios::out | std::ios::binary | std::ios::trunc);
            if (!file.is_open()) {
                std::cerr << "Failed to open file for writing: " << filename << "\n";
                return;
            }

            file.write(yamlString.c_str(), yamlString.size());

            const char* separator = "\n---PARTICLE BINARY DATA---\n";
            file.write(separator, strlen(separator));

            for (size_t i = 0; i < myParam.pParticles.size(); i++) {
                const ParticlePhysics& p = myParam.pParticles[i];
                const ParticleRendering& r = myParam.rParticles[i];
                file.write(reinterpret_cast<const char*>(&p), sizeof(p));
                file.write(reinterpret_cast<const char*>(&r), sizeof(r));
            }

            file.close();
        }
        else if (loadFlag) {
            file.open(filename, std::ios::in | std::ios::binary);
            if (!file.is_open()) {
                std::cerr << "Failed to open file for reading: " << filename << "\n";
                return;
            }

            const std::string sepToken = "---PARTICLE BINARY DATA---";
            std::string line;
            bool foundSeparator = false;
            std::streampos binaryStartPos = 0;

            file.seekg(0, std::ios::beg);

            while (std::getline(file, line)) {
                if (line.find(sepToken) != std::string::npos) {
                    foundSeparator = true;

                    binaryStartPos = file.tellg();
                    break;
                }
            }

            if (!foundSeparator) {
                std::cerr << "Separator not found. Cannot load binary data.\n";
                file.close();
                return;
            }

            file.clear();
            file.seekg(binaryStartPos);

            myParam.pParticles.clear();
            myParam.rParticles.clear();

            while (true) {
                ParticlePhysics p;
                ParticleRendering r;

                file.read(reinterpret_cast<char*>(&p), sizeof(p));
                if (file.gcount() != sizeof(p)) {
                    break;
                }
                file.read(reinterpret_cast<char*>(&r), sizeof(r));
                if (file.gcount() != sizeof(r)) {
                    std::cerr << "Incomplete ParticleRendering read; file may be corrupted.\n";
                    break;
                }
                myParam.pParticles.push_back(p);
                myParam.rParticles.push_back(r);
            }

            file.close();
        }
    }


	void saveLoadLogic(UpdateVariables& myVar, UpdateParameters& myParam, SPH& sph) {
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

			saveSystem(savePath.c_str(), myVar, myParam, sph);

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

				if (ImGui::Button(fullPath.c_str(), ImVec2(ImGui::GetContentRegionAvail().x, buttonHeight))) {
					saveSystem(fullPath.c_str(), myVar, myParam, sph);
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