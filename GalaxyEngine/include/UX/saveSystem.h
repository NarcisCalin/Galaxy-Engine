#pragma once

#include "Particles/particle.h"

#include "Physics/SPH.h"
#include "Physics/physics.h"
#include "UI/UI.h"

#include "parameters.h"

struct ParticleConstraint;

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

	const uint32_t version160 = 160;
	const uint32_t currentVersion = version160; // VERY IMPORTANT. CHANGE THIS IF YOU MAKE ANY CHANGES TO THE SAVE SYSTEM. VERSION "1.6.0" = 160, VERSION "1.6.12" = 1612

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

	void saveSystem(const std::string& filename, UpdateVariables& myVar, UpdateParameters& myParam, SPH& sph, Physics& physics);

	bool deserializeParticleSystem(const std::string& filename,
		std::string& yamlString,
		UpdateVariables& myVar,
		UpdateParameters& myParam,
		SPH& sph,
		Physics& physics,
		bool loadFlag) {
		if (!loadFlag) return false;

		std::fstream file(filename, std::ios::in | std::ios::binary);
		if (!file.is_open()) {
			std::cerr << "Failed to open file for reading: " << filename << std::endl;
			return false;
		}

		const std::string sepToken = "---PARTICLE BINARY DATA---";
		std::string line;
		bool foundSeparator = false;
		std::streampos binaryStartPos = 0;
		yamlString.clear();

		while (std::getline(file, line)) {
			if (line.find(sepToken) != std::string::npos) {
				foundSeparator = true;
				binaryStartPos = file.tellg();
				break;
			}
			yamlString += line + "\n";
		}

		if (!foundSeparator) {
			std::cerr << "Separator not found. Cannot load binary data.\n";
			file.close();
			return false;
		}

		file.seekg(binaryStartPos);

		uint32_t loadedVersion;
		file.read(reinterpret_cast<char*>(&loadedVersion), sizeof(loadedVersion));
		if (loadedVersion != currentVersion) {
			std::cerr << "Version mismatch! Expected version: " << currentVersion << " Loaded version: " << loadedVersion << std::endl;
		}
		else {
			std::cout << "File version: " << loadedVersion << std::endl;
		}

		if (loadedVersion == currentVersion) {
			deserializeVersion160(file, myParam);
		}

		physics.particleConstraints.clear();
		uint32_t numConstraints = 0;
		file.read(reinterpret_cast<char*>(&numConstraints), sizeof(numConstraints));
		if (numConstraints > 0) {
			
			physics.particleConstraints.resize(numConstraints);
			file.read(
				reinterpret_cast<char*>(physics.particleConstraints.data()),
				numConstraints * sizeof(ParticleConstraint)
			);

			physics.constraintMap.clear();
			for (auto& constraint : physics.particleConstraints) {
				uint64_t key = physics.makeKey(constraint.id1, constraint.id2);
				physics.constraintMap[key] = &constraint;
			}
		}

		file.close();

		uint32_t maxId = 0;
		for (const auto& particle : myParam.pParticles) {
			if (particle.id > maxId) maxId = particle.id;
		}
		globalId = maxId + 1;

		return true;
	}

	bool deserializeVersion160(std::istream& file, UpdateParameters& myParam) {

		uint32_t particleCount;
		file.read(reinterpret_cast<char*>(&particleCount), sizeof(particleCount));

		myParam.pParticles.clear();
		myParam.rParticles.clear();
		myParam.pParticles.reserve(particleCount);
		myParam.rParticles.reserve(particleCount);

		for (uint32_t i = 0; i < particleCount; i++) {
			ParticlePhysics p;
			ParticleRendering r;

			file.read(reinterpret_cast<char*>(&p.pos), sizeof(p.pos));
			file.read(reinterpret_cast<char*>(&p.predPos), sizeof(p.predPos));
			file.read(reinterpret_cast<char*>(&p.vel), sizeof(p.vel));
			file.read(reinterpret_cast<char*>(&p.prevVel), sizeof(p.prevVel));
			file.read(reinterpret_cast<char*>(&p.predVel), sizeof(p.predVel));
			file.read(reinterpret_cast<char*>(&p.acc), sizeof(p.acc));
			file.read(reinterpret_cast<char*>(&p.mass), sizeof(p.mass));
			file.read(reinterpret_cast<char*>(&p.press), sizeof(p.press));
			file.read(reinterpret_cast<char*>(&p.pressTmp), sizeof(p.pressTmp));
			file.read(reinterpret_cast<char*>(&p.pressF), sizeof(p.pressF));
			file.read(reinterpret_cast<char*>(&p.dens), sizeof(p.dens));
			file.read(reinterpret_cast<char*>(&p.predDens), sizeof(p.predDens));
			file.read(reinterpret_cast<char*>(&p.sphMass), sizeof(p.sphMass));
			file.read(reinterpret_cast<char*>(&p.restDens), sizeof(p.restDens));
			file.read(reinterpret_cast<char*>(&p.stiff), sizeof(p.stiff));
			file.read(reinterpret_cast<char*>(&p.visc), sizeof(p.visc));
			file.read(reinterpret_cast<char*>(&p.cohesion), sizeof(p.cohesion));
			file.read(reinterpret_cast<char*>(&p.temp), sizeof(p.temp));
			file.read(reinterpret_cast<char*>(&p.ke), sizeof(p.ke));
			file.read(reinterpret_cast<char*>(&p.prevKe), sizeof(p.prevKe));
			file.read(reinterpret_cast<char*>(&p.mortonKey), sizeof(p.mortonKey));
			file.read(reinterpret_cast<char*>(&p.id), sizeof(p.id));
			file.read(reinterpret_cast<char*>(&p.isHotPoint), sizeof(p.isHotPoint));
			file.read(reinterpret_cast<char*>(&p.hasSolidified), sizeof(p.hasSolidified));

			uint32_t numNeighbors = 0;
			file.read(reinterpret_cast<char*>(&numNeighbors), sizeof(numNeighbors));
			if (numNeighbors > 0) {
				p.neighborIds.resize(numNeighbors);
				file.read(reinterpret_cast<char*>(p.neighborIds.data()),
					numNeighbors * sizeof(uint32_t));
			}

			file.read(reinterpret_cast<char*>(&r.color), sizeof(r.color));
			file.read(reinterpret_cast<char*>(&r.pColor), sizeof(r.pColor));
			file.read(reinterpret_cast<char*>(&r.sColor), sizeof(r.sColor));
			file.read(reinterpret_cast<char*>(&r.sphColor), sizeof(r.sphColor));
			file.read(reinterpret_cast<char*>(&r.size), sizeof(r.size));
			file.read(reinterpret_cast<char*>(&r.uniqueColor), sizeof(r.uniqueColor));
			file.read(reinterpret_cast<char*>(&r.isSolid), sizeof(r.isSolid));
			file.read(reinterpret_cast<char*>(&r.canBeSubdivided), sizeof(r.canBeSubdivided));
			file.read(reinterpret_cast<char*>(&r.canBeResized), sizeof(r.canBeResized));
			file.read(reinterpret_cast<char*>(&r.isDarkMatter), sizeof(r.isDarkMatter));
			file.read(reinterpret_cast<char*>(&r.isSPH), sizeof(r.isSPH));
			file.read(reinterpret_cast<char*>(&r.isSelected), sizeof(r.isSelected));
			file.read(reinterpret_cast<char*>(&r.isGrabbed), sizeof(r.isGrabbed));
			file.read(reinterpret_cast<char*>(&r.previousSize), sizeof(r.previousSize));
			file.read(reinterpret_cast<char*>(&r.neighbors), sizeof(r.neighbors));
			file.read(reinterpret_cast<char*>(&r.totalRadius), sizeof(r.totalRadius));
			file.read(reinterpret_cast<char*>(&r.lifeSpan), sizeof(r.lifeSpan));
			file.read(reinterpret_cast<char*>(&r.sphLabel), sizeof(r.sphLabel));
			file.read(reinterpret_cast<char*>(&r.isPinned), sizeof(r.isPinned));
			file.read(reinterpret_cast<char*>(&r.isBeingDrawn), sizeof(r.isBeingDrawn));
			file.read(reinterpret_cast<char*>(&r.spawnCorrectIter), sizeof(r.spawnCorrectIter));

			myParam.pParticles.push_back(p);
			myParam.rParticles.push_back(r);
		}
		return true;
	}


	void saveLoadLogic(UpdateVariables& myVar, UpdateParameters& myParam, SPH& sph, Physics& physics);

private:

	ImVec2 loadMenuSize = { 600.0f, 500.0f };
	float buttonHeight = 30.0f;

	std::vector<std::string> filePaths;

	int saveIndex = 0;
};