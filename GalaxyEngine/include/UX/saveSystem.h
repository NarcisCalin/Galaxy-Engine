#pragma once

#include "Particles/particle.h"
#include "Physics/light.h"
#include "Physics/field.h"

#include "Physics/SPH.h"
#include "Physics/physics.h"
#include "Physics/physics3D.h"
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
	const uint32_t version170 = 170;
	const uint32_t version171 = 171;
	const uint32_t version172 = 172;
	const uint32_t version173 = 173;
	const uint32_t version180 = 180;
	const uint32_t version190 = 190;
	const uint32_t currentVersion = version190; // VERY IMPORTANT. CHANGE THIS IF YOU MAKE ANY CHANGES TO THE SAVE SYSTEM. VERSION "1.6.0" = 160, VERSION "1.6.12" = 1612

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

	void saveSystem(const std::string& filename, UpdateVariables& myVar, UpdateParameters& myParam, SPH& sph, Physics& physics, Physics3D& physics3D,
		Lighting& lighting, Field& field);

	bool deserializeParticleSystem(const std::string& filename,
		std::string& yamlString,
		UpdateVariables& myVar,
		UpdateParameters& myParam,
		SPH& sph,
		Physics& physics,
		Physics3D& physics3D,
		Lighting& lighting,
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
			std::cerr << "Loaded file from older version! Expected version: " << currentVersion << " Loaded version: " << loadedVersion << std::endl;
		}
		else {
			std::cout << "File version: " << loadedVersion << std::endl;
		}

		if (loadedVersion == currentVersion) {
			deserializeVersion190(file, myParam, physics, physics3D, lighting, myVar.is3DMode);
		}
		else if (loadedVersion == version180) {
			deserializeVersion190(file, myParam, physics, physics3D, lighting, myVar.is3DMode);
		}

		if (myVar.isOpticsEnabled) {
			lighting.shouldRender = true;
		}

		lighting.wallPointers.clear();
		for (Wall& wall : lighting.walls) {
			lighting.wallPointers.push_back(&wall);
		}
		lighting.bvh.build(lighting.wallPointers);
		
		myVar.longExposureFlag = false;

		myVar.loadDropDownMenus = true;

		file.close();

		uint32_t maxId = 0;
		for (const auto& particle : myParam.pParticles) {
			if (particle.id > maxId) maxId = particle.id;
		}
		globalId = maxId + 1;

		return true;
	}

	bool deserializeVersion190(std::istream& file, UpdateParameters& myParam, Physics& physics, Physics3D& physics3D, Lighting& lighting, bool& is3DMode) {

		file.read(reinterpret_cast<char*>(&globalId), sizeof(globalId));
		file.read(reinterpret_cast<char*>(&globalShapeId), sizeof(globalShapeId));
		file.read(reinterpret_cast<char*>(&globalWallId), sizeof(globalWallId));

		myParam.pParticles.clear();
		myParam.rParticles.clear();

		myParam.pParticles3D.clear();
		myParam.rParticles3D.clear();

		myParam.pParticlesSelected.clear();
		myParam.rParticlesSelected.clear();

		myParam.pParticlesSelected3D.clear();
		myParam.rParticlesSelected3D.clear();

		if (!is3DMode) {
			uint32_t particleCount;
			file.read(reinterpret_cast<char*>(&particleCount), sizeof(particleCount));

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
				file.read(reinterpret_cast<char*>(&r.turbulence), sizeof(r.turbulence));

				myParam.pParticles.push_back(p);
				myParam.rParticles.push_back(r);
			}
		}
		else {
			uint32_t particleCount;
			file.read(reinterpret_cast<char*>(&particleCount), sizeof(particleCount));

			myParam.pParticles3D.reserve(particleCount);
			myParam.rParticles3D.reserve(particleCount);

			for (uint32_t i = 0; i < particleCount; i++) {
				ParticlePhysics3D p;
				ParticleRendering3D r;

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
				file.read(reinterpret_cast<char*>(&r.turbulence), sizeof(r.turbulence));

				myParam.pParticles3D.push_back(p);
				myParam.rParticles3D.push_back(r);
			}
		}

		if (!is3DMode) {

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
		}
		else {

			physics3D.particleConstraints.clear();

			uint32_t numConstraints = 0;
			file.read(reinterpret_cast<char*>(&numConstraints), sizeof(numConstraints));
			if (numConstraints > 0) {

				physics3D.particleConstraints.resize(numConstraints);
				file.read(
					reinterpret_cast<char*>(physics3D.particleConstraints.data()),
					numConstraints * sizeof(ParticleConstraint)
				);

				physics3D.constraintMap.clear();
				for (auto& constraint : physics3D.particleConstraints) {
					uint64_t key = physics3D.makeKey(constraint.id1, constraint.id2);
					physics3D.constraintMap[key] = &constraint;
				}
			}
		}

		uint32_t wallCount = 0;
		file.read(reinterpret_cast<char*>(&wallCount), sizeof(wallCount));
		lighting.walls.clear();
		lighting.walls.reserve(wallCount);

		for (uint32_t i = 0; i < wallCount; i++) {
			Wall w;

			file.read(reinterpret_cast<char*>(&w.vA), sizeof(w.vA));
			file.read(reinterpret_cast<char*>(&w.vB), sizeof(w.vB));
			file.read(reinterpret_cast<char*>(&w.normal), sizeof(w.normal));
			file.read(reinterpret_cast<char*>(&w.normalVA), sizeof(w.normalVA));
			file.read(reinterpret_cast<char*>(&w.normalVB), sizeof(w.normalVB));

			file.read(reinterpret_cast<char*>(&w.isBeingSpawned), sizeof(w.isBeingSpawned));
			file.read(reinterpret_cast<char*>(&w.vAisBeingMoved), sizeof(w.vAisBeingMoved));
			file.read(reinterpret_cast<char*>(&w.vBisBeingMoved), sizeof(w.vBisBeingMoved));

			file.read(reinterpret_cast<char*>(&w.apparentColor), sizeof(w.apparentColor));
			file.read(reinterpret_cast<char*>(&w.baseColor), sizeof(w.baseColor));
			file.read(reinterpret_cast<char*>(&w.specularColor), sizeof(w.specularColor));
			file.read(reinterpret_cast<char*>(&w.refractionColor), sizeof(w.refractionColor));
			file.read(reinterpret_cast<char*>(&w.emissionColor), sizeof(w.emissionColor));

			file.read(reinterpret_cast<char*>(&w.baseColorVal), sizeof(w.baseColorVal));
			file.read(reinterpret_cast<char*>(&w.specularColorVal), sizeof(w.specularColorVal));
			file.read(reinterpret_cast<char*>(&w.refractionColorVal), sizeof(w.refractionColorVal));

			file.read(reinterpret_cast<char*>(&w.specularRoughness), sizeof(w.specularRoughness));
			file.read(reinterpret_cast<char*>(&w.refractionRoughness), sizeof(w.refractionRoughness));
			file.read(reinterpret_cast<char*>(&w.refractionAmount), sizeof(w.refractionAmount));
			file.read(reinterpret_cast<char*>(&w.IOR), sizeof(w.IOR));
			file.read(reinterpret_cast<char*>(&w.dispersionStrength), sizeof(w.dispersionStrength));

			file.read(reinterpret_cast<char*>(&w.isShapeWall), sizeof(w.isShapeWall));
			file.read(reinterpret_cast<char*>(&w.isShapeClosed), sizeof(w.isShapeClosed));
			file.read(reinterpret_cast<char*>(&w.shapeId), sizeof(w.shapeId));

			file.read(reinterpret_cast<char*>(&w.id), sizeof(w.id));
			file.read(reinterpret_cast<char*>(&w.isSelected), sizeof(w.isSelected));

			lighting.walls.push_back(w);
		}

		uint32_t numShapes = 0;
		file.read(reinterpret_cast<char*>(&numShapes), sizeof(numShapes));

		lighting.shapes.clear();
		lighting.shapes.reserve(numShapes);

		if (numShapes > 0) {

			for (uint32_t i = 0; i < numShapes; i++) {
				Shape s;

				uint32_t wallIdCount = 0;
				file.read(reinterpret_cast<char*>(&wallIdCount), sizeof(wallIdCount));
				s.myWallIds.resize(wallIdCount);
				file.read(reinterpret_cast<char*>(s.myWallIds.data()), wallIdCount * sizeof(uint32_t));

				uint32_t vertCount = 0;
				file.read(reinterpret_cast<char*>(&vertCount), sizeof(vertCount));
				s.polygonVerts.resize(vertCount);
				file.read(reinterpret_cast<char*>(s.polygonVerts.data()), vertCount * sizeof(glm::vec2));

				uint32_t helpersCount = 0;
				file.read(reinterpret_cast<char*>(&helpersCount), sizeof(helpersCount));
				s.helpers.resize(helpersCount);
				file.read(reinterpret_cast<char*>(s.helpers.data()), helpersCount * sizeof(glm::vec2));


				file.read(reinterpret_cast<char*>(&s.baseColor), sizeof(s.baseColor));
				file.read(reinterpret_cast<char*>(&s.specularColor), sizeof(s.specularColor));
				file.read(reinterpret_cast<char*>(&s.refractionColor), sizeof(s.refractionColor));
				file.read(reinterpret_cast<char*>(&s.emissionColor), sizeof(s.emissionColor));

				file.read(reinterpret_cast<char*>(&s.specularRoughness), sizeof(s.specularRoughness));
				file.read(reinterpret_cast<char*>(&s.refractionRoughness), sizeof(s.refractionRoughness));
				file.read(reinterpret_cast<char*>(&s.refractionAmount), sizeof(s.refractionAmount));
				file.read(reinterpret_cast<char*>(&s.IOR), sizeof(s.IOR));
				file.read(reinterpret_cast<char*>(&s.dispersionStrength), sizeof(s.dispersionStrength));

				file.read(reinterpret_cast<char*>(&s.id), sizeof(s.id));

				file.read(reinterpret_cast<char*>(&s.h1), sizeof(s.h1));
				file.read(reinterpret_cast<char*>(&s.h2), sizeof(s.h2));

				file.read(reinterpret_cast<char*>(&s.isBeingSpawned), sizeof(s.isBeingSpawned));
				file.read(reinterpret_cast<char*>(&s.isBeingMoved), sizeof(s.isBeingMoved));
				file.read(reinterpret_cast<char*>(&s.isShapeClosed), sizeof(s.isShapeClosed));

				file.read(reinterpret_cast<char*>(&s.shapeType), sizeof(s.shapeType));

				file.read(reinterpret_cast<char*>(&s.drawHoverHelpers), sizeof(s.drawHoverHelpers));

				file.read(reinterpret_cast<char*>(&s.oldDrawHelperPos), sizeof(s.oldDrawHelperPos));

				// Lens variables
				file.read(reinterpret_cast<char*>(&s.secondHelper), sizeof(s.secondHelper));
				file.read(reinterpret_cast<char*>(&s.thirdHelper), sizeof(s.thirdHelper));
				file.read(reinterpret_cast<char*>(&s.fourthHelper), sizeof(s.fourthHelper));

				file.read(reinterpret_cast<char*>(&s.Tempsh2Length), sizeof(s.Tempsh2Length));
				file.read(reinterpret_cast<char*>(&s.Tempsh2LengthSymmetry), sizeof(s.Tempsh2LengthSymmetry));
				file.read(reinterpret_cast<char*>(&s.tempDist), sizeof(s.tempDist));

				file.read(reinterpret_cast<char*>(&s.moveH2), sizeof(s.moveH2));

				file.read(reinterpret_cast<char*>(&s.isThirdBeingMoved), sizeof(s.isThirdBeingMoved));
				file.read(reinterpret_cast<char*>(&s.isFourthBeingMoved), sizeof(s.isFourthBeingMoved));
				file.read(reinterpret_cast<char*>(&s.isFifthBeingMoved), sizeof(s.isFifthBeingMoved));

				file.read(reinterpret_cast<char*>(&s.isFifthFourthMoved), sizeof(s.isFifthFourthMoved));

				file.read(reinterpret_cast<char*>(&s.symmetricalLens), sizeof(s.symmetricalLens));

				file.read(reinterpret_cast<char*>(&s.wallAId), sizeof(s.wallAId));
				file.read(reinterpret_cast<char*>(&s.wallBId), sizeof(s.wallBId));
				file.read(reinterpret_cast<char*>(&s.wallCId), sizeof(s.wallCId));

				file.read(reinterpret_cast<char*>(&s.lensSegments), sizeof(s.lensSegments));

				file.read(reinterpret_cast<char*>(&s.startAngle), sizeof(s.startAngle));
				file.read(reinterpret_cast<char*>(&s.endAngle), sizeof(s.endAngle));

				file.read(reinterpret_cast<char*>(&s.startAngleSymmetry), sizeof(s.startAngleSymmetry));
				file.read(reinterpret_cast<char*>(&s.endAngleSymmetry), sizeof(s.endAngleSymmetry));

				file.read(reinterpret_cast<char*>(&s.center), sizeof(s.center));
				file.read(reinterpret_cast<char*>(&s.radius), sizeof(s.radius));

				file.read(reinterpret_cast<char*>(&s.centerSymmetry), sizeof(s.centerSymmetry));
				file.read(reinterpret_cast<char*>(&s.radiusSymmetry), sizeof(s.radiusSymmetry));

				file.read(reinterpret_cast<char*>(&s.arcEnd), sizeof(s.arcEnd));

				file.read(reinterpret_cast<char*>(&s.globalLensPrev), sizeof(s.globalLensPrev));

				s.walls = &lighting.walls;

				lighting.shapes.push_back(s);
			}
		}

		uint32_t pointLightCount = 0;
		file.read(reinterpret_cast<char*>(&pointLightCount), sizeof(pointLightCount));

		lighting.pointLights.clear();
		lighting.pointLights.reserve(pointLightCount);

		for (uint32_t i = 0; i < pointLightCount; i++) {

			PointLight p = lighting.pointLights[i];

			file.read(reinterpret_cast<char*>(&p.pos), sizeof(p.pos));
			file.read(reinterpret_cast<char*>(&p.isBeingMoved), sizeof(p.isBeingMoved));
			file.read(reinterpret_cast<char*>(&p.color), sizeof(p.color));
			file.read(reinterpret_cast<char*>(&p.apparentColor), sizeof(p.apparentColor));
			file.read(reinterpret_cast<char*>(&p.isSelected), sizeof(p.isSelected));

			lighting.pointLights.push_back(p);
		}

		uint32_t areaLightCount = 0;
		file.read(reinterpret_cast<char*>(&areaLightCount), sizeof(areaLightCount));

		lighting.areaLights.clear();
		lighting.areaLights.reserve(areaLightCount);

		for (uint32_t i = 0; i < areaLightCount; i++) {

			AreaLight a = lighting.areaLights[i];

			file.read(reinterpret_cast<char*>(&a.vA), sizeof(a.vA));
			file.read(reinterpret_cast<char*>(&a.vB), sizeof(a.vB));
			file.read(reinterpret_cast<char*>(&a.isBeingSpawned), sizeof(a.isBeingSpawned));
			file.read(reinterpret_cast<char*>(&a.vAisBeingMoved), sizeof(a.vAisBeingMoved));
			file.read(reinterpret_cast<char*>(&a.vBisBeingMoved), sizeof(a.vBisBeingMoved));
			file.read(reinterpret_cast<char*>(&a.color), sizeof(a.color));
			file.read(reinterpret_cast<char*>(&a.apparentColor), sizeof(a.apparentColor));
			file.read(reinterpret_cast<char*>(&a.isSelected), sizeof(a.isSelected));
			file.read(reinterpret_cast<char*>(&a.spread), sizeof(a.spread));

			lighting.areaLights.push_back(a);
		}

		uint32_t coneLightCount = 0;
		file.read(reinterpret_cast<char*>(&coneLightCount), sizeof(coneLightCount));

		lighting.coneLights.clear();
		lighting.coneLights.reserve(coneLightCount);

		for (uint32_t i = 0; i < coneLightCount; i++) {

			ConeLight l = lighting.coneLights[i];

			file.read(reinterpret_cast<char*>(&l.vA), sizeof(l.vA));
			file.read(reinterpret_cast<char*>(&l.vB), sizeof(l.vB));
			file.read(reinterpret_cast<char*>(&l.isBeingSpawned), sizeof(l.isBeingSpawned));
			file.read(reinterpret_cast<char*>(&l.vAisBeingMoved), sizeof(l.vAisBeingMoved));
			file.read(reinterpret_cast<char*>(&l.vBisBeingMoved), sizeof(l.vBisBeingMoved));
			file.read(reinterpret_cast<char*>(&l.color), sizeof(l.color));
			file.read(reinterpret_cast<char*>(&l.apparentColor), sizeof(l.apparentColor));
			file.read(reinterpret_cast<char*>(&l.isSelected), sizeof(l.isSelected));
			file.read(reinterpret_cast<char*>(&l.spread), sizeof(l.spread));

			lighting.coneLights.push_back(l);
		}

		return true;
	}


	void saveLoadLogic(UpdateVariables& myVar, UpdateParameters& myParam, SPH& sph, Physics& physics, Physics3D& physics3D, Lighting& lighting, Field& field);

private:

	ImVec2 loadMenuSize = { 600.0f, 500.0f };
	float buttonHeight = 30.0f;

	std::vector<std::string> filePaths;

	int saveIndex = 0;
};