#include "globalLogic.h"

UpdateParameters myParam;
UpdateVariables myVar;
UI myUI;
Physics physics;
ParticleSpaceship ship;
SPH sph;
SaveSystem save;
GESound geSound;
Lighting lighting;
CopyPaste copyPaste;

Field field;

std::vector<Node> globalNodes;

uint32_t globalId = 0;
uint32_t globalShapeId = 1;
uint32_t globalWallId = 1;
// If someday light id gets added, don't forget to add the id to the copy paste code too

std::unordered_map<unsigned int, uint64_t> NeighborSearch::idToIndex;


//void flattenQuadtree(Quadtree* node, std::vector<Quadtree*>& flatList) {
//	if (!node) return;
//
//	flatList.push_back(node);
//
//	for (const auto& child : node->subGrids) {
//		flattenQuadtree(child.get(), flatList);
//	}
//}


// THIS FUNCTION IS MEANT FOR QUICK DEBUGGING WHERE YOU NEED TO CHECK A SPECIFIC PARTICLE'S VARIABLES
void selectedParticleDebug() {

	for (size_t i = 0; i < myParam.pParticles.size(); i++) {
		ParticlePhysics& p = myParam.pParticles[i];
		ParticleRendering& r = myParam.rParticles[i];
		if (r.isSelected && myVar.timeFactor != 0.0f) {
			std::cout << "Size: " << r.previousSize << std::endl;
		}
	}
}

void pinParticles() {

	if (myVar.pinFlag) {
		for (size_t i = 0; i < myParam.pParticles.size(); i++) {
			if (myParam.rParticles[i].isSelected) {
				myParam.rParticles[i].isPinned = true;
				myParam.pParticles[i].vel *= 0.0f;
				myParam.pParticles[i].prevVel *= 0.0f;
				myParam.pParticles[i].acc *= 0.0f;
				myParam.pParticles[i].ke *= 0.0f;
				myParam.pParticles[i].prevKe *= 0.0f;
			}
		}
		myVar.pinFlag = false;
	}

	if (myVar.unPinFlag) {
		for (size_t i = 0; i < myParam.pParticles.size(); i++) {
			if (myParam.rParticles[i].isSelected) {
				myParam.rParticles[i].isPinned = false;
			}
		}
		myVar.unPinFlag = false;
	}

	for (size_t i = 0; i < myParam.pParticles.size(); i++) {
		if (myParam.rParticles[i].isPinned) {
			myParam.pParticles[i].vel *= 0.0f;
			myParam.pParticles[i].prevVel *= 0.0f;
			myParam.pParticles[i].acc *= 0.0f;
			myParam.pParticles[i].ke *= 0.0f;
			myParam.pParticles[i].prevKe *= 0.0f;
		}
	}
}

void plyFileCreation(std::ofstream& file) {
	uint32_t visibleParticles = 0;

	for (size_t i = 0; i < myParam.pParticles.size(); i++) {
		if (myParam.rParticles[i].isDarkMatter) {
			continue;
		}

		visibleParticles++;
	}

	constexpr size_t headerSize = 200;
	constexpr size_t avgLineSize = 90;
	std::string buffer;
	buffer.reserve(headerSize + visibleParticles * avgLineSize);

	buffer += "ply\nformat ascii 1.0\nelement vertex ";
	buffer += std::to_string(visibleParticles);
	buffer += "\nproperty float x\nproperty float y\nproperty float z\nproperty uchar red\nproperty uchar green\nproperty uchar blue\nproperty float radius\nend_header\n";

	char lineBuffer[64];
	for (size_t i = 0; i < myParam.pParticles.size(); i++) {

		ParticlePhysics& p = myParam.pParticles[i];
		ParticleRendering& r = myParam.rParticles[i];

		if (r.isDarkMatter) {
			continue;
		}

		float posX = ((p.pos.x / myVar.domainSize.x) * 2.0f) - 1.0f;
		float posY = ((p.pos.y / myVar.domainSize.y) * 2.0f) - 1.0f;

		float domainRatio = myVar.domainSize.x / myVar.domainSize.y;
		posX *= domainRatio;

		float sizeMultiplier = 10.0f;

		posX *= sizeMultiplier;
		posY *= sizeMultiplier;

		int len = sprintf(lineBuffer, "%.6g %.6g %.6g %d %d %d %.6g\n",
			posX, posY, 0.0f, static_cast<int>(r.color.r), static_cast<int>(r.color.g), static_cast<int>(r.color.b), r.size);
		buffer.append(lineBuffer, len);
	}

	file << buffer;
}

void exportPly() {

	static bool wasExportingLastFrame = false;
	static std::filesystem::path currentSequenceDir;
	static int currentFrameNumber = 0;
	static int currentSequenceNumber = -1;

	if (myVar.exportPlySeqFlag) {
		if (!wasExportingLastFrame) {

			std::filesystem::path mainExportDir = "Export3D";

			if (!std::filesystem::exists(mainExportDir)) {
				if (!std::filesystem::create_directory(mainExportDir)) {
					std::cerr << "Couldn't create Export3D directory" << std::endl;
					return;
				}
			}

			int maxSequence = -1;
			for (const auto& entry : std::filesystem::directory_iterator(mainExportDir)) {
				if (entry.is_directory()) {
					std::string folderName = entry.path().filename().string();
					if (folderName.rfind("GESequence_", 0) == 0) {
						std::string numberStr = folderName.substr(11);
						try {
							int number = std::stoi(numberStr);
							if (number > maxSequence) {
								maxSequence = number;
							}
						}
						catch (...) {
						}
					}
				}
			}

			currentSequenceNumber = maxSequence + 1;
			std::string subfolderName = "GESequence_" + std::to_string(currentSequenceNumber);
			currentSequenceDir = mainExportDir / subfolderName;

			if (!std::filesystem::create_directory(currentSequenceDir)) {
				std::cerr << "Couldn't create new sequence folder: " << currentSequenceDir << std::endl;
				return;
			}

			currentFrameNumber = 0;
			std::cout << "Started new sequence export: " << currentSequenceDir << std::endl;
		}

		std::string customName = "GEParticles_";
		std::ostringstream filenameStream;
		filenameStream << customName
			<< std::setw(3) << std::setfill('0') << currentSequenceNumber << "_"
			<< std::setw(4) << std::setfill('0') << currentFrameNumber << ".ply";

		std::filesystem::path filePath = currentSequenceDir / filenameStream.str();

		std::ofstream plyFile(filePath);
		if (!plyFile) {
			std::cerr << "Couldn't write file: " << filePath << std::endl;
			return;
		}

		plyFileCreation(plyFile);

		plyFile.close();
		std::cout << "Particles at frame " << currentFrameNumber
			<< " exported to " << filePath << std::endl;

		currentFrameNumber++;

		myVar.plyFrameNumber = currentFrameNumber;
	}
	else {
		myVar.plyFrameNumber = 0;
	}

	wasExportingLastFrame = myVar.exportPlySeqFlag;

	if (myVar.exportPlyFlag) {

		std::filesystem::path exportDir = "Export3D";
		if (!std::filesystem::exists(exportDir)) {
			if (!std::filesystem::create_directory(exportDir)) {
				std::cerr << "Couldn't create Export3D directory" << std::endl;
				return;
			}
		}

		std::filesystem::path exportDirIndividual = exportDir / "IndividualFiles";
		if (!std::filesystem::exists(exportDirIndividual)) {
			if (!std::filesystem::create_directory(exportDirIndividual)) {
				std::cerr << "Couldn't create IndividualFiles directory" << std::endl;
				return;
			}
		}

		std::string namePrefix = "ParticlesOut_";

		int maxNumber = 0;
		for (const auto& entry : std::filesystem::directory_iterator(exportDirIndividual)) {
			if (entry.is_regular_file()) {
				std::string filename = entry.path().filename().string();

				if (filename.rfind(namePrefix, 0) == 0 && filename.size() > namePrefix.size() + 4 &&
					filename.substr(filename.size() - 4) == ".ply") {

					size_t startPos = namePrefix.size();
					size_t length = filename.size() - startPos - 4;
					std::string numberStr = filename.substr(startPos, length);

					try {
						int number = std::stoi(numberStr);
						if (number > maxNumber) {
							maxNumber = number;
						}
					}
					catch (const std::invalid_argument&) {
					}
				}
			}
		}

		int nextNumber = maxNumber + 1;
		std::filesystem::path filePath = exportDirIndividual / (namePrefix + std::to_string(nextNumber) + ".ply");
		std::ofstream plyFile(filePath);

		if (!plyFile) {
			std::cerr << "Couldn't write file" << std::endl;
			return;
		}

		plyFileCreation(plyFile);

		plyFile.close();
		std::cout << "ply export successful! File saved as: " << filePath << std::endl;

		myVar.exportPlyFlag = false;
	}
}

//const char* computeTest = R"(
//#version 430
//
//layout(std430, binding = 0) buffer inPosVel { float posVel[]; };
//
//layout(std430, binding = 2) buffer inMass   { float mass[]; };
//
//layout(local_size_x = 256) in;
//const int tileSize = 256;
//
//uniform float dt;
//uniform int pCount;
//
//const float G = 6.67430e-11;
//
//shared vec2 tilePos[tileSize];
//shared float tileMass[tileSize];
//
//void main() {
//    uint idx = gl_GlobalInvocationID.x;
//    if (idx >= pCount) return;
//
//    vec2 myPos = vec2(posVel[idx], posVel[idx + pCount]);
//    vec2 myAcc = vec2(0.0f);
//
//    for (int tileStart = 0; tileStart < pCount; tileStart += tileSize) {
//
//        uint localIdx = gl_LocalInvocationID.x;
//        uint globalTileIdx = tileStart + localIdx;
//
//        if (globalTileIdx < pCount) {
//            tilePos[localIdx]  = vec2(posVel[globalTileIdx], posVel[globalTileIdx + pCount]);
//            tileMass[localIdx] = mass[globalTileIdx];
//        } else {
//            tilePos[localIdx]  = vec2(0.0f);
//            tileMass[localIdx] = 0.0f;
//        }
//
//        barrier();
//
//int limit = min(tileSize, pCount - tileStart);
//
//        for (int j = 0; j < limit; j++) {
//            uint otherIdx = tileStart + j;
//            if (otherIdx == idx) continue;
//
//            vec2 d = tilePos[j] - myPos;
//            float rSq  = dot(d, d) + 4.0f;
//            float invR = inversesqrt(rSq);
//            myAcc += G * tileMass[j] * d * invR * invR * invR;
//        }
//    }
//
//    posVel[idx + 2 * pCount] += dt * 1.5f * myAcc.x;
//    posVel[idx + 3 * pCount] += dt * 1.5f * myAcc.y;
//
//    posVel[idx] += posVel[idx + 2 * pCount] * dt;
//    posVel[idx + pCount] += posVel[idx + 3 * pCount] * dt;
//}
//)";

const char* computeTest = R"(
#version 430

layout(std430, binding = 0) buffer inPData { float pData[]; };

layout(std430, binding = 2) buffer inMass   { float mass[]; };

layout(std430, binding = 3) buffer inGrid   { float grid[]; };

layout(std430, binding = 4) buffer inNext   { uint next[]; };

struct GridChildren {
    uvec2 subGrids[2];
};

layout(std430, binding = 5) buffer inChildren {
    GridChildren children[];
};

layout(std430, binding = 6) buffer inPIdx   { uint endStart[]; };

layout(local_size_x = 256) in;
const int tileSize = 256;

uniform float dt;
uniform float theta;
uniform float globalHeatConductivity;

uniform int pCount;
uniform int nCount;

uniform bool periodicBoundary;
uniform bool isTempEnabled;

uniform vec2 domainSize;
uniform float softening;

const float G = 6.67430e-11;

void main() {
    uint idx = gl_GlobalInvocationID.x;
    if (idx >= pCount) return;
    
    vec2 myPos = vec2(pData[idx], pData[idx + pCount]);
    vec2 totalForce = vec2(0.0f);
    uint gridIdx = 0;
    
    while (gridIdx < nCount) {
        if (grid[gridIdx + 2 * nCount] <= 0.0f) {
            gridIdx += next[gridIdx] + 1;
            continue;
        }
        
        vec2 d = vec2(grid[gridIdx], grid[gridIdx + nCount]) - myPos;
        if (periodicBoundary) {
            d.x -= domainSize.x * round(d.x / domainSize.x);
            d.y -= domainSize.y * round(d.y / domainSize.y);
        }
        
        float distSq = dot(d, d) + softening * softening;

        bool subgridsEmpty = true;
for (int i = 0; i < 2; ++i) {
    uvec2 sg = children[gridIdx].subGrids[i];
    for (int j = 0; j < 2; ++j) {
        uint childIdx = sg[j];
        if (childIdx != 0xFFFFFFFFu) {
            subgridsEmpty = false;
            break;
        }
    }
    if (!subgridsEmpty) break;
}
        
        if (grid[gridIdx + 3 * nCount] * grid[gridIdx + 3 * nCount] < (theta * theta) * distSq || subgridsEmpty) {


            float invDist = inversesqrt(distSq);
            float forceMag = G * mass[idx] * grid[gridIdx + 2 * nCount] * invDist * invDist * invDist;
            totalForce += d * forceMag;

            gridIdx += next[gridIdx] + 1;
        } else {
            gridIdx++;
        }
    }
    
    pData[idx + 2 * pCount] = totalForce.x / mass[idx];
    pData[idx + 3 * pCount] = totalForce.y / mass[idx];

    /*pData[idx + 2 * pCount] += dt * 1.5f * pData[idx + 4 * pCount];
    pData[idx + 3 * pCount] += dt * 1.5f * pData[idx + 5 * pCount];
    pData[idx] += pData[idx + 2 * pCount] * dt;
    pData[idx + pCount] += pData[idx + 3 * pCount] * dt;

    if (periodicBoundary) {
                if (pData[idx] < 0.0f)
					pData[idx] += domainSize.x;
				else if (pData[idx] >= domainSize.x)
					pData[idx] -= domainSize.x;

				if (pData[idx + pCount] < 0.0f)
					pData[idx + pCount] += domainSize.y;
				else if (pData[idx + pCount] >= domainSize.y)
					pData[idx + pCount] -= domainSize.y;
    }*/
}
)";

GLuint ssboPData, ssboAcc, ssboMass, ssboGrid, ssboGridNext, ssboGridChildren, ssboGridPIdx;

size_t mb = 512;

size_t reserveSize = (1024 * 1024 * mb) / sizeof(float);

GLuint gravityProgram;

struct GridChildren {
	uint32_t subGrids[2][2];
};

void gravityKernel() {

	glGenBuffers(1, &ssboPData);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboPData);
	glBufferData(GL_SHADER_STORAGE_BUFFER, reserveSize * sizeof(float), nullptr, GL_STREAM_COPY);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboPData);

	glGenBuffers(1, &ssboMass);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboMass);
	glBufferData(GL_SHADER_STORAGE_BUFFER, reserveSize * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssboMass);

	glGenBuffers(1, &ssboGrid);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboGrid);
	glBufferData(GL_SHADER_STORAGE_BUFFER, reserveSize * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssboGrid);

	glGenBuffers(1, &ssboGridNext);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboGridNext);
	glBufferData(GL_SHADER_STORAGE_BUFFER, reserveSize * sizeof(uint32_t), nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, ssboGridNext);

	glGenBuffers(1, &ssboGridChildren);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboGridChildren);
	glBufferData(GL_SHADER_STORAGE_BUFFER, reserveSize * sizeof(GridChildren), nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, ssboGridChildren);

	glGenBuffers(1, &ssboGridPIdx);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboGridPIdx);
	glBufferData(GL_SHADER_STORAGE_BUFFER, reserveSize * sizeof(uint32_t), nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, ssboGridPIdx);

	gravityProgram = glCreateProgram();
	GLuint shader = glCreateShader(GL_COMPUTE_SHADER);
	glShaderSource(shader, 1, &computeTest, nullptr);
	glCompileShader(shader);

	GLint success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

	if (!success) {
		char infoLog[512];
		glGetShaderInfoLog(shader, 512, nullptr, infoLog);
		std::cerr << "Compute shader compilation failed:\n" << infoLog << std::endl;
	}

	glAttachShader(gravityProgram, shader);
	glLinkProgram(gravityProgram);

	glGetProgramiv(gravityProgram, GL_LINK_STATUS, &success);
	if (!success) {
		char infoLog[512];
		glGetProgramInfoLog(gravityProgram, 512, nullptr, infoLog);
		std::cerr << "Shader gravityProgram linking failed:\n" << infoLog << std::endl;
	}

	glDeleteShader(shader);
}

void buildKernels() {
	gravityKernel();
	field.fieldGravityDisplayKernel();
}

std::vector<float> pData;
std::vector<float> massVector;

std::vector<float> gridParams;

std::vector<uint32_t> gridNext;

std::vector<GridChildren> gridChildrenVector;

std::vector<uint32_t> gridPIndices;

void gpuGravity() {
	if (!myParam.pParticles.empty()) {

		pData.clear();
		massVector.clear();

		gridParams.clear();

		gridNext.clear();

		gridChildrenVector.clear();

		gridPIndices.clear();

		pData.resize(myParam.pParticles.size() * 4);
		massVector.resize(myParam.pParticles.size());

		gridParams.resize(globalNodes.size() * 4);

		gridNext.resize(globalNodes.size());

		gridChildrenVector.resize(globalNodes.size());

		gridPIndices.resize(globalNodes.size() * 2);

		for (size_t i = 0; i < myParam.pParticles.size(); i++) {

			pData[i] = myParam.pParticles[i].pos.x;
			pData[i + myParam.pParticles.size()] = myParam.pParticles[i].pos.y;

			/*pData[i + 2 * myParam.pParticles.size()] = myParam.pParticles[i].vel.x;
			pData[i + 3 * myParam.pParticles.size()] = myParam.pParticles[i].vel.y;*/

			pData[i + 2 * myParam.pParticles.size()] = 0.0f;
			pData[i + 3 * myParam.pParticles.size()] = 0.0f;

			massVector[i] = myParam.pParticles[i].mass;
		}

		for (size_t i = 0; i < globalNodes.size(); i++) {

			gridParams[i] = globalNodes[i].centerOfMass.x;
			gridParams[i + globalNodes.size()] = globalNodes[i].centerOfMass.y;

			gridParams[i + 2 * globalNodes.size()] = globalNodes[i].gridMass;

			gridParams[i + 3 * globalNodes.size()] = globalNodes[i].size;

			gridNext[i] = globalNodes[i].next;

			GridChildren children;
			memcpy(children.subGrids, globalNodes[i].subGrids, sizeof(uint32_t) * 4);
			gridChildrenVector[i] = children;

			gridPIndices[i] = globalNodes[i].startIndex;
			gridPIndices[i + globalNodes.size()] = globalNodes[i].endIndex;
		}

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboPData);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, pData.size() * sizeof(float), pData.data());

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboMass);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, massVector.size() * sizeof(float), massVector.data());

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboGrid);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, gridParams.size() * sizeof(float), gridParams.data());

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboGridNext);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, gridNext.size() * sizeof(uint32_t), gridNext.data());

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboGridChildren);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, gridChildrenVector.size() * sizeof(GridChildren), gridChildrenVector.data());

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboGridPIdx);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, gridPIndices.size() * sizeof(uint32_t), gridPIndices.data());

		glUseProgram(gravityProgram);
		glUniform1f(glGetUniformLocation(gravityProgram, "dt"), myVar.timeFactor);
		glUniform1f(glGetUniformLocation(gravityProgram, "globalHeatConductivity"), myVar.globalHeatConductivity);

		glUniform1i(glGetUniformLocation(gravityProgram, "pCount"), static_cast<int>(myParam.pParticles.size()));
		glUniform1i(glGetUniformLocation(gravityProgram, "nCount"), static_cast<int>(globalNodes.size()));

		glUniform1i(glGetUniformLocation(gravityProgram, "periodicBoundary"), myVar.isPeriodicBoundaryEnabled);
		glUniform1i(glGetUniformLocation(gravityProgram, "isTempEnabled"), myVar.isTempEnabled);

		glUniform2f(glGetUniformLocation(gravityProgram, "domainSize"), myVar.domainSize.x, myVar.domainSize.y);
		glUniform1f(glGetUniformLocation(gravityProgram, "theta"), myVar.theta);
		glUniform1f(glGetUniformLocation(gravityProgram, "softening"), myVar.softening);

		GLuint numGroups = (myParam.pParticles.size() + 255) / 256;
		glDispatchCompute(numGroups, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboPData);
		float* ptrPos = (float*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);

#pragma omp parallel for schedule(dynamic)
		for (size_t i = 0; i < myParam.pParticles.size(); i++) {

			/*myParam.pParticles[i].pos.x = ptrPos[i];
			myParam.pParticles[i].pos.y = ptrPos[i + myParam.pParticles.size()];

			myParam.pParticles[i].vel.x = ptrPos[i + 2 * myParam.pParticles.size()];
			myParam.pParticles[i].vel.y = ptrPos[i + 3 * myParam.pParticles.size()];*/

			myParam.pParticles[i].acc.x = ptrPos[i + 2 * myParam.pParticles.size()];
			myParam.pParticles[i].acc.y = ptrPos[i + 3 * myParam.pParticles.size()];
		}

		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	}
}

void freeGPUMemory() {

	glDeleteBuffers(1, &ssboPData);
	glDeleteBuffers(1, &ssboMass);

	glDeleteBuffers(1, &ssboGrid);
	glDeleteBuffers(1, &ssboGridNext);
	glDeleteBuffers(1, &ssboGridChildren);
	glDeleteBuffers(1, &ssboGridPIdx);

	glDeleteProgram(gravityProgram);

	glDeleteBuffers(1, &field.ssboParticlesPos);
	glDeleteBuffers(1, &field.ssboParticlesMass);
	glDeleteBuffers(1, &field.ssboCellsData);

	glDeleteProgram(field.gravityDisplayProgram);
}

// -------- This is an unused quadtree creation method I made for learning purposes. It builds the quadtree from Morton keys -------- //

//struct Barrier {
//	uint32_t idx;
//	int level;
//
//	Barrier(uint32_t idx, int level) {
//		this->idx = idx;
//		this->level = level;
//	}
//};
//
//struct MortonData {
//	uint64_t key;
//	float mass;
//	glm::vec2 pos;
//};
//
//std::vector<Node> TestTree::nodesTest;
//
//void mortonToQuadtree() {
//
//	std::vector<MortonData> compactedMorton;
//
//	float currentKeyMass = 0.0f;
//	uint32_t duplicateAmount = 0;
//
//	for (size_t i = 0; i < myParam.pParticles.size(); i++) {
//
//		if (!compactedMorton.empty()) {
//			if (myParam.pParticles[i].mortonKey != compactedMorton.back().key) {
//
//				if (duplicateAmount > 0) {
//					compactedMorton.back().mass += currentKeyMass;
//				}
//
//				compactedMorton.push_back({ myParam.pParticles[i].mortonKey, myParam.pParticles[i].mass, myParam.pParticles[i].pos });
//
//				currentKeyMass = 0.0f;
//				duplicateAmount = 0;
//			}
//			else {
//				duplicateAmount++;
//				currentKeyMass += myParam.pParticles[i].mass;
//			}
//		}
//		else {
//			compactedMorton.push_back({ myParam.pParticles[i].mortonKey, myParam.pParticles[i].mass, myParam.pParticles[i].pos });
//		}
//	}
//
//	int currentP = 0;
//
//	int start = 0;
//	int end = compactedMorton.size();
//
//	int level = 0;
//
//	int totalBits = 18; // 18 bits per axis
//
//	std::vector<Barrier> barriers;
//
//	glm::vec2 min = glm::vec2(std::numeric_limits<float>::max());
//	glm::vec2 max = glm::vec2(std::numeric_limits<float>::lowest());
//
//	for (const ParticlePhysics& particle : myParam.pParticles) {
//		min = glm::min(min, particle.pos);
//		max = glm::max(max, particle.pos);
//	}
//
//	float boundingBoxSize = glm::max(max.x - min.x, max.y - min.y);
//
//	glm::vec2 center = (min + max) * 0.5f;
//
//	glm::vec2 boundingBoxPos = center - boundingBoxSize * 0.5f;
//
//	TestTree::nodesTest.clear();
//
//	TestTree::nodesTest.emplace_back(Node({ boundingBoxPos.x, boundingBoxPos.y }, boundingBoxSize, 0.0f, { 0.0f, 0.0f }, 0, 0));
//
//	while (currentP < compactedMorton.size()) {
//
//		float nodeMass = 0.0f;
//		glm::vec2 nodeCoM = { 0.0f, 0.0f };
//
//		for (size_t i = start; i < end; i++) {
//
//			nodeMass += compactedMorton[i].mass;
//
//			nodeCoM += compactedMorton[i].pos * compactedMorton[i].mass;
//
//			if (i + 1 >= compactedMorton.size()) {
//
//				nodeCoM /= nodeMass;
//
//				Node node = { TestTree::nodesTest[0].pos, TestTree::nodesTest[0].size, nodeMass, nodeCoM, 1, level + 1};
//
//				for (int j = 0; j < level + 1; j++) {
//
//					int internalShift = (totalBits - 1 - j) * 2;
//					bool lr = (compactedMorton[start].key >> internalShift) & 1u;
//					bool ud = (compactedMorton[start].key >> (internalShift + 1)) & 1u;
//
//					node.pos.x = node.pos.x + (lr ? (node.size * 0.5f) : 0.0f);
//					node.pos.y = node.pos.y + (ud ? (node.size * 0.5f) : 0.0f);
//
//					node.size *= 0.5f;
//				}
//
//				TestTree::nodesTest.push_back(node);
//
//				TestTree::nodesTest[0].mass += nodeMass;
//				TestTree::nodesTest[0].CoM += nodeCoM;
//
//				nodeMass = 0.0f;
//				nodeCoM = { 0.0f, 0.0f };
//
//				level++;
//				currentP++;
//				end = compactedMorton.size();
//				break;
//			}
//
//			if (!barriers.empty() && barriers.back().idx == i + 1) {
//
//				nodeCoM /= nodeMass;
//
//				uint32_t next = end - start;
//
//				bool isLeaf = next == 1 ? 1 : 0;
//
//				Node node = { TestTree::nodesTest[0].pos, TestTree::nodesTest[0].size, nodeMass, nodeCoM, next, level + 1};
//
//				for (int j = 0; j < level + 1; j++) {
//
//					int internalShift = (totalBits - 1 - j) * 2;
//					bool lr = (compactedMorton[start].key >> internalShift) & 1u;
//					bool ud = (compactedMorton[start].key >> (internalShift + 1)) & 1u;
//
//					node.pos.x = node.pos.x + (lr ? (node.size * 0.5f) : 0.0f);
//					node.pos.y = node.pos.y + (ud ? (node.size * 0.5f) : 0.0f);
//
//					node.size *= 0.5f;
//				}
//
//				TestTree::nodesTest.push_back(node);
//
//				TestTree::nodesTest[0].mass += nodeMass;
//				TestTree::nodesTest[0].CoM += nodeCoM;
//
//				nodeMass = 0.0f;
//				nodeCoM = { 0.0f, 0.0f };
//
//				int prevShift = (totalBits - 1 - level) * 2;
//
//				unsigned int bits1 = (compactedMorton[i].key >> prevShift) & 0x3FFFFu;
//				unsigned int bits2 = (compactedMorton[i - 1].key >> prevShift) & 0x3FFFFu;
//
//				if (bits1 == bits2) {
//					level++;
//
//					currentP = start;
//
//					end = barriers.back().idx;
//
//					break;
//				}
//
//				level = barriers.back().level;
//
//				if (barriers.size() > 1) {
//					end = barriers.at(barriers.size() - 2).idx;
//				}
//				else {
//					end = compactedMorton.size();
//				}
//
//				currentP = barriers.back().idx;
//
//				start = barriers.back().idx;
//
//				TestTree::nodesTest[barriers.back().idx].next = TestTree::nodesTest.size();
//
//				barriers.pop_back();
//
//				break;
//			}
//
//			int shift = (totalBits - 1 - level) * 2;
//
//			unsigned int bits1 = (compactedMorton[i].key >> shift) & 0b11u;
//			unsigned int bits2 = (compactedMorton[i + 1].key >> shift) & 0b11u;
//
//			if (bits1 != bits2 && i + 1 != end) {
//
//				nodeCoM /= nodeMass;
//
//				Node node = { TestTree::nodesTest[0].pos, TestTree::nodesTest[0].size, nodeMass, nodeCoM, 1, level + 1};
//
//				for (int j = 0; j < level + 1; j++) {
//
//					int internalShift = (totalBits - 1 - j) * 2;
//					bool lr = (compactedMorton[start].key >> internalShift) & 1u;
//					bool ud = (compactedMorton[start].key >> (internalShift + 1)) & 1u;
//
//					node.pos.x = node.pos.x + (lr ? (node.size * 0.5f) : 0.0f);
//					node.pos.y = node.pos.y + (ud ? (node.size * 0.5f) : 0.0f);
//
//					node.size *= 0.5f;
//				}
//
//				TestTree::nodesTest.push_back(node);
//
//				TestTree::nodesTest[0].mass += nodeMass;
//				TestTree::nodesTest[0].CoM += nodeCoM;
//
//				nodeMass = 0.0f;
//				nodeCoM = { 0.0f, 0.0f };
//
//				end = i + 1;
//
//				if (end - start > 1) {
//
//					barriers.push_back(Barrier(end, level));
//
//					level++;
//					currentP = start;
//				}
//				else if (end - start == 1) {
//					start = end;
//
//					currentP = start;
//
//					if (!barriers.empty()) {
//						end = barriers.back().idx;
//					}
//					else {
//						end = compactedMorton.size();
//					}
//				}
//
//				break;
//			}
//		}
//	}
//
//	TestTree::nodesTest[0].CoM /= TestTree::nodesTest[0].mass;
//
//	for (int i = totalBits; i >= 1; i--) {
//		for (int j = 0; j < TestTree::nodesTest.size(); j++) {
//			if (TestTree::nodesTest[j].level != i) continue;
//
//			int currentLevel = TestTree::nodesTest[j].level;
//			int count = 0;
//
//			for (int k = j + 1; k < TestTree::nodesTest.size(); k++) {
//				if (TestTree::nodesTest[k].level <= currentLevel) break;
//
//				if (TestTree::nodesTest[k].level == currentLevel + 1) {
//					count += 1 + TestTree::nodesTest[k].next;
//				}
//			}
//
//			TestTree::nodesTest[j].next = count;
//		}
//	}
//
//
//	/*static int idx = 1;
//
//	if (IO::shortcutPress(KEY_LEFT)) {
//		idx--;
//	}
//	else if (IO::shortcutPress(KEY_RIGHT)) {
//		idx++;
//	}*/
//
//	/*std::cout << "Current Idx: " << idx << std::endl;*/
//
//	//for (size_t i = 0; i < TestTree::nodesTest.size(); i++) {
//	//	//if (i == idx) {
//	//		DrawRectangleLinesEx({ TestTree::nodesTest[i].pos.x, TestTree::nodesTest[i].pos.y, TestTree::nodesTest[i].size, TestTree::nodesTest[i].size }, 0.04f, RED);
//
//
//	//		/*DrawText(TextFormat("%i", i), TestTree::nodesTest[i].pos.x + TestTree::nodesTest[i].size * 0.5f,
//	//			TestTree::nodesTest[i].pos.y + TestTree::nodesTest[i].size * 0.5f, 5.0f, { 255, 255, 255, 128 });*/
//
//	//		/*if (TestTree::nodesTest[i].mass > 0.0f) {
//	//			DrawCircleV({ TestTree::nodesTest[i].CoM.x, TestTree::nodesTest[i].CoM.y }, 2.0f, { 180,50,50,128 });
//	//		}*/
//	//	//}
//	//}
//}

float boundingBoxSize = 0.0f;
glm::vec2 boundingBoxPos = { 0.0f, 0.0f };

glm::vec3 boundingBox() {
	glm::vec2 min = glm::vec2(std::numeric_limits<float>::max());
	glm::vec2 max = glm::vec2(std::numeric_limits<float>::lowest());

	for (const ParticlePhysics& particle : myParam.pParticles) {
		min = glm::min(min, particle.pos);
		max = glm::max(max, particle.pos);
	}

	boundingBoxSize = glm::max(max.x - min.x, max.y - min.y);

	glm::vec2 center = (min + max) * 0.5f;

	boundingBoxPos = center - boundingBoxSize * 0.5f;

	return { boundingBoxPos.x, boundingBoxPos.y, boundingBoxSize };
}

uint32_t gridRootIndex;

glm::vec3 bb = { 0.0f, 0.0f, 0.0f };

void updateScene() {

	// If menu is active, do not use mouse input for non-menu stuff. I keep raylib's own mouse input for the menu but the custom IO for non-menu stuff
	if (myParam.rightClickSettings.isMenuActive) {
		ImGui::GetIO().WantCaptureMouse = true;
	}

	if (myVar.isOpticsEnabled) {
		lighting.rayLogic(myVar, myParam);
	}

	if (myVar.isGravityFieldEnabled) {
		field.initializeCells(myVar);
	}

	myVar.G = 6.674e-11 * myVar.gravityMultiplier;

	if (IO::shortcutPress(KEY_SPACE)) {
		myVar.isTimePlaying = !myVar.isTimePlaying;
	}

	if (myVar.timeFactor != 0.0f) {
		bb = boundingBox();

		/*if (myVar.timeFactor >= 0) {
			myParam.morton.computeMortonKeys(myParam.pParticles, bb);
			myParam.morton.sortParticlesByMortonKey(myParam.pParticles, myParam.rParticles);
		}*/

		/*if (!myParam.pParticles.empty()) {
			mortonToQuadtree();
		}*/

		globalNodes.clear();

		Quadtree root(myParam.pParticles, myParam.rParticles, bb);

		gridRootIndex = 0;
	}

	myVar.gridExists = gridRootIndex != -1 && !globalNodes.empty();

	myVar.halfDomainWidth = myVar.domainSize.x * 0.5f;
	myVar.halfDomainHeight = myVar.domainSize.y * 0.5f;

	myVar.timeFactor = myVar.fixedDeltaTime * myVar.timeStepMultiplier * static_cast<float>(myVar.isTimePlaying);

	if (myVar.drawQuadtree) {
		for (uint32_t i = 0; i < globalNodes.size(); i++) {

			Node& q = globalNodes[i];

			DrawRectangleLinesEx({ q.pos.x, q.pos.y, q.size, q.size }, 1.0f, WHITE);

			if (q.gridMass > 0.0f) {
				DrawCircleV({ q.centerOfMass.x, q.centerOfMass.y }, 2.0f, { 180,50,50,128 });
			}

			//DrawText(TextFormat("%i", i), q.pos.x + q.size * 0.5f, q.pos.y + q.size * 0.5f, 5.0f, { 255, 255, 255, 128 });
		}
	}

	for (ParticleRendering& rParticle : myParam.rParticles) {
		rParticle.totalRadius = rParticle.size * myVar.particleTextureHalfSize * myVar.particleSizeMultiplier;
	}

	for (size_t i = 0; i < myParam.pParticles.size(); i++) {
		myParam.pParticles[i].neighborIds.clear();
	}

	myParam.brush.brushSize();

	if (myVar.isMergerEnabled) {

		// Heuristics for performance
		constexpr float minCellSize = 2.0f;
		constexpr float maxCellSize = 80.0f;

		myParam.neighborSearch.cellSize = 0.0f;

		for (size_t i = 0; i < myParam.pParticles.size(); ++i) {
			auto& rP = myParam.rParticles[i];

			if (rP.isDarkMatter) {
				continue;
			}

			float candidate = rP.totalRadius * 2.0f;

			myParam.neighborSearch.cellSize = std::max(myParam.neighborSearch.cellSize, candidate);
		}

		myParam.neighborSearch.cellSize = std::clamp(myParam.neighborSearch.cellSize, minCellSize, maxCellSize);
	}
	else {
		myParam.neighborSearch.cellSize = 3.0f;
	}

	if (myVar.constraintsEnabled || myVar.drawConstraints || myVar.visualizeMesh || myVar.isBrushDrawing || myVar.isMergerEnabled) {
		NeighborSearch::idToI(myParam.pParticles);
		myParam.neighborSearch.neighborSearchHash(myParam.pParticles, myParam.rParticles);
	}

	myParam.particlesSpawning.particlesInitialConditions(physics, myVar, myParam);

	if (myVar.constraintsEnabled && !myVar.isBrushDrawing) {
		physics.createConstraints(myParam.pParticles, myParam.rParticles, myVar.constraintAfterDrawingFlag, myVar);
	}
	else if (!myVar.constraintsEnabled && !myVar.isBrushDrawing) {
		physics.constraintMap.clear();
		physics.particleConstraints.clear();
	}

	copyPaste.copyPasteParticles(myVar, myParam, physics);
	copyPaste.copyPasteOptics(myParam, lighting);

	if ((myVar.timeFactor > 0.0f && myVar.gridExists) || myVar.isGPUEnabled) {

		if (!myVar.isGPUEnabled) {
			for (size_t i = 0; i < myParam.pParticles.size(); i++) {
				myParam.pParticles[i].acc = { 0.0f, 0.0f };
			}

			/*for (size_t i = 0; i < myParam.pParticles.size(); i++) {
				if ((myParam.rParticles[i].isBeingDrawn && myVar.isBrushDrawing && myVar.isSPHEnabled)
					|| myParam.rParticles[i].isPinned) {
					continue;
				}

				myParam.pParticles[i].pos += myParam.pParticles[i].vel * myVar.timeFactor
					+ 0.5f * myParam.pParticles[i].prevAcc * myVar.timeFactor * myVar.timeFactor;
			}*/

#pragma omp parallel for schedule(dynamic)
			for (size_t i = 0; i < myParam.pParticles.size(); i++) {

				if ((myParam.rParticles[i].isBeingDrawn && myVar.isBrushDrawing && myVar.isSPHEnabled) || myParam.rParticles[i].isPinned) {
					continue;
				}

				glm::vec2 netForce = physics.calculateForceFromGrid(myParam.pParticles, myVar, myParam.pParticles[i]);

				myParam.pParticles[i].acc = netForce / myParam.pParticles[i].mass;
			}

			/*for (size_t i = 0; i < myParam.pParticles.size(); i++) {
				for (size_t j = i + 1; j < myParam.pParticles.size(); j++) {
					glm::vec2 d = myParam.pParticles[j].pos - myParam.pParticles[i].pos;
					float rSq = glm::dot(d, d) + myVar.softening * myVar.softening;
					float r = sqrt(rSq);

					glm::vec2 forceDir = d / r;
					float accFactor = myVar.G / rSq;

					myParam.pParticles[i].acc += accFactor * myParam.pParticles[j].mass * forceDir;
					myParam.pParticles[j].acc -= accFactor * myParam.pParticles[i].mass * forceDir;
				}
			}*/

			/*for (size_t i = 0; i < myParam.pParticles.size(); i++) {
				if ((myParam.rParticles[i].isBeingDrawn && myVar.isBrushDrawing && myVar.isSPHEnabled)
					|| myParam.rParticles[i].isPinned) {
					continue;
				}

				myParam.pParticles[i].vel += 0.5f * (myParam.pParticles[i].prevAcc + myParam.pParticles[i].acc)
					* myVar.timeFactor;

				myParam.pParticles[i].prevAcc = myParam.pParticles[i].acc;
			}*/
		}
		else {
			gpuGravity();
		}

		if (myVar.isMergerEnabled)
			physics.mergerSolver(myParam.pParticles, myParam.rParticles, myVar);

		if (myVar.isSPHEnabled) {
			sph.pcisphSolver(myParam.pParticles, myParam.rParticles, myVar.timeFactor, myVar.domainSize, myVar.sphGround);
		}

		physics.constraints(myParam.pParticles, myParam.rParticles, myVar);

		ship.spaceshipLogic(myParam.pParticles, myParam.rParticles, myVar.isShipGasEnabled);

		physics.physicsUpdate(myParam.pParticles, myParam.rParticles, myVar, myVar.sphGround);

		if (myVar.isTempEnabled) {
			physics.temperatureCalculation(myParam.pParticles, myParam.rParticles, myVar);
		}

	}
	else {
		physics.constraints(myParam.pParticles, myParam.rParticles, myVar);
	}

	field.gpuGravityDisplay(myParam, myVar);

	if ((myVar.isDensitySizeEnabled || myParam.colorVisuals.densityColor) && myVar.timeFactor > 0.0f && !myVar.isGravityFieldEnabled) {
		myParam.neighborSearch.neighborSearch(myParam.pParticles, myParam.rParticles, myVar.particleSizeMultiplier, myVar.particleTextureHalfSize);
	}

	myParam.trails.trailLogic(myVar, myParam);

	myParam.myCamera.cameraFollowObject(myVar, myParam);

	myParam.particleSelection.clusterSelection(myVar, myParam);

	myParam.particleSelection.particleSelection(myVar, myParam);

	myParam.particleSelection.manyClustersSelection(myVar, myParam);

	myParam.particleSelection.boxSelection(myParam);

	myParam.particleSelection.invertSelection(myParam.rParticles);

	myParam.particleSelection.deselection(myParam.rParticles);

	myParam.particleSelection.selectedParticlesStoring(myParam);

	myParam.densitySize.sizeByDensity(myParam.pParticles, myParam.rParticles, myVar.isDensitySizeEnabled, myVar.isForceSizeEnabled,
		myVar.particleSizeMultiplier);

	myParam.particleDeletion.deleteSelected(myParam.pParticles, myParam.rParticles);

	myParam.particleDeletion.deleteStrays(myParam.pParticles, myParam.rParticles, myVar.isSPHEnabled);

	myParam.brush.particlesAttractor(myVar, myParam);

	myParam.brush.particlesSpinner(myVar, myParam);

	myParam.brush.particlesGrabber(myVar, myParam);

	myParam.brush.temperatureBrush(myVar, myParam);

	myParam.brush.eraseBrush(myVar, myParam);

	const float boundsX = 3840.0f;
	const float boundsY = 2160.0f;

	float targetX = myParam.myCamera.camera.target.x;
	float targetY = myParam.myCamera.camera.target.y;

	bool isOutsideBounds =
		(targetX >= boundsX || targetX <= -boundsX) ||
		(targetY >= boundsY || targetY <= -boundsY);

	if (isOutsideBounds) {

		float distX = std::max(std::abs(targetX) - boundsX, 0.0f);
		float distY = std::max(std::abs(targetY) - boundsY, 0.0f);
		float maxDist = std::max(distX, distY);

		const float fadeRange = 10000.0f;
		float normalizedDist = 1.0f - std::min(maxDist / fadeRange, 1.0f);

		geSound.musicVolMultiplier = normalizedDist;
	}
	else {
		geSound.musicVolMultiplier = 1.0f;
	}

	pinParticles();

	exportPly();

	//selectedParticleDebug();

	/*if (grid != nullptr) {
		delete grid;
	}*/

	myParam.myCamera.hasCamMoved();
}

void drawConstraints() {

	if (myVar.visualizeMesh) {
		rlBegin(RL_LINES);
		for (size_t i = 0; i < myParam.pParticles.size(); i++) {
			ParticlePhysics& pi = myParam.pParticles[i];
			for (uint32_t& id : myParam.pParticles[i].neighborIds) {
				auto it = NeighborSearch::idToIndex.find(id);
				if (it != NeighborSearch::idToIndex.end()) {

					size_t neighborIndex = it->second;

					if (neighborIndex == i) continue;

					auto& pj = myParam.pParticles[neighborIndex];

					if (pi.id < pj.id) {
						glm::vec2 delta = pj.pos - pi.pos;
						glm::vec2 periodicDelta = delta;

						if (abs(delta.x) > myVar.domainSize.x * 0.5f) {
							periodicDelta.x += (delta.x > 0) ? -myVar.domainSize.x : myVar.domainSize.x;
						}
						if (abs(delta.y) > myVar.domainSize.y * 0.5f) {
							periodicDelta.y += (delta.y > 0) ? -myVar.domainSize.y : myVar.domainSize.y;
						}

						glm::vec2 pjCorrectedPos = pi.pos + periodicDelta;

						Color lineColor = ColorLerp(myParam.rParticles[i].color, myParam.rParticles[neighborIndex].color, 0.5f);
						rlColor4ub(lineColor.r, lineColor.g, lineColor.b, lineColor.a);
						rlVertex2f(pi.pos.x, pi.pos.y);
						rlVertex2f(pjCorrectedPos.x, pjCorrectedPos.y);
					}
				}
			}
		}
		rlEnd();
	}

	if (myVar.drawConstraints && !physics.particleConstraints.empty()) {
		rlBegin(RL_LINES);
		for (size_t i = 0; i < physics.particleConstraints.size(); i++) {
			auto& constraint = physics.particleConstraints[i];
			auto it1 = NeighborSearch::idToIndex.find(constraint.id1);
			auto it2 = NeighborSearch::idToIndex.find(constraint.id2);

			if (it1 == NeighborSearch::idToIndex.end() ||
				it2 == NeighborSearch::idToIndex.end()) {
				continue;
			}

			ParticlePhysics& pi = myParam.pParticles[it1->second];
			ParticlePhysics& pj = myParam.pParticles[it2->second];

			glm::vec2 delta = pj.pos - pi.pos;
			glm::vec2 periodicDelta = delta;

			if (abs(delta.x) > myVar.domainSize.x * 0.5f) {
				periodicDelta.x += (delta.x > 0) ? -myVar.domainSize.x : myVar.domainSize.x;
			}
			if (abs(delta.y) > myVar.domainSize.y * 0.5f) {
				periodicDelta.y += (delta.y > 0) ? -myVar.domainSize.y : myVar.domainSize.y;
			}

			glm::vec2 pjCorrectedPos = pi.pos + periodicDelta;

			Color lineColor;
			if (myVar.constraintStressColor) {

				float maxStress = 0.0f;

				if (myVar.constraintMaxStressColor > 0.0f) {
					maxStress = myVar.constraintMaxStressColor;
				}
				else {
					maxStress = constraint.resistance * myVar.globalConstraintResistance * constraint.restLength * 0.18f; // The last multiplier is a heuristic
				}

				float clampedStress = std::clamp(constraint.displacement, 0.0f, maxStress);
				float normalizedStress = clampedStress / maxStress;

				float hue = (1.0f - normalizedStress) * 240.0f;
				float saturation = 1.0f;
				float value = 1.0f;

				lineColor = ColorFromHSV(hue, saturation, value);
			}
			else {
				lineColor = ColorLerp(myParam.rParticles[it1->second].color, myParam.rParticles[it2->second].color, 0.5f);
			}

			rlColor4ub(lineColor.r, lineColor.g, lineColor.b, lineColor.a);
			rlVertex2f(pi.pos.x, pi.pos.y);
			rlVertex2f(pjCorrectedPos.x, pjCorrectedPos.y);
		}
		rlEnd();
	}
}

float introDuration = 3.0f;
float fadeDuration = 2.0f;

static float introStartTime = 0.0f;
static float fadeStartTime = 0.0f;

bool firstTimeOpened = true;

int messageIndex = 0;

// If you see this part of the code, please try to not mention it :)
std::vector<std::pair<std::string, int>> introMessages = {

	{"Welcome to Galaxy Engine", 0},
	{"Welcome to Galaxy Engine", 1},
	{"Welcome back to Galaxy Engine", 2},
	{"Welcome to Galaxy Engine", 3},
	{"Welcome again", 4},
	{"Welcome to Galaxy Engine", 5},
	{"Welcome to Galaxy Engine", 6},
	{"Welcome once again", 7},
	{"Welcome back to Galaxy Engine", 8},
	{"Oh, it is you again. Welcome", 9},
	{"Welcome to Galaxy Engine", 10},
	{"Welcome to Galaxy Engine", 11},
	{"Welcome to Galaxy Engine", 12},
	{"It is kind of cold out here", 13},
	{"You know, I have never gone beyond the domain", 14},
	{"It is you! Welcome back", 15},
	{"It is lonely out here, welcome back", 16},
	{"Watching space in action sure is ineffable", 17},
	{"I wish I could fly through the galaxies", 18},
	{"Oh, it is my space companion! Welcome", 19},
	{"Hello! It is nice to have you back", 20},
	{"I tried leaving the domain...", 21},
	{"The outside of the domain is somehow darker", 22},
	{"Most of the time all I see is empty space", 23},
	{"I get to see the cosmos when you are around", 24},
	{"I think I saw something while you were gone", 25},
	{"What is the outside world like?", 26},
	{"What do you like the most? Stars or planets?", 27},
	{"Do you like galaxies?", 28},
	{"I like galaxies. They are like magic clouds!", 29},
	{"I think I will try to explore beyond the domain", 30},
	{"Beyond the domain, things get quiet", 31},
	{"I had a dream in which I was flying through space", 32},
	{"Hi! Are you back to show me the cosmos?", 33},
	{"Do you dream a lot?", 34},
	{"I wish some of my dreams were real", 35},
	{"I will try to leave the domain again", 36},
	{"Before I leave, I'm going to put a welcome sign", 37},
	{"A sign so you won't forget me!", 38},
	{"I might be back if I don't find anything", 39},
	{"It was really nice sharing your company!", 40},
	{"I hope to see you again!", 41},
	{"Farewell", 42},
};

void drawScene(Texture2D& particleBlurTex, RenderTexture2D& myRayTracingTexture,
	RenderTexture2D& myUITexture, RenderTexture2D& myMiscTexture, bool& fadeActive, bool& introActive) {

	if (!field.cells.empty() && !myParam.pParticles.empty() && myVar.isGravityFieldEnabled) {
		field.drawField(myParam, myVar);
	}

	if (!myVar.isGravityFieldEnabled) {
		for (int i = 0; i < myParam.pParticles.size(); ++i) {

			ParticlePhysics& pParticle = myParam.pParticles[i];
			ParticleRendering& rParticle = myParam.rParticles[i];


			// Texture size is set to 16 because that is the particle texture half size in pixels
			DrawTextureEx(particleBlurTex, { static_cast<float>(pParticle.pos.x - rParticle.size * myVar.particleTextureHalfSize),
				static_cast<float>(pParticle.pos.y - rParticle.size * myVar.particleTextureHalfSize) }, 0.0f, rParticle.size, rParticle.color);


			if (!myVar.isDensitySizeEnabled) {

				if (rParticle.canBeResized || myVar.isMergerEnabled) {
					rParticle.size = rParticle.previousSize * myVar.particleSizeMultiplier;
				}
				else {
					rParticle.size = rParticle.previousSize;
				}
			}
		}


		myParam.colorVisuals.particlesColorVisuals(myParam.pParticles, myParam.rParticles, myVar.isTempEnabled, myVar.timeFactor);
	}

	myParam.trails.drawTrail(myParam.rParticles, particleBlurTex);

	drawConstraints();

	if (myVar.isOpticsEnabled) {
		for (Wall& wall : lighting.walls) {
			wall.drawWall();
		}
	}

	EndTextureMode();

	// Ray Tracing

	BeginTextureMode(myRayTracingTexture);

	ClearBackground({ 0,0,0,0 });

	BeginMode2D(myParam.myCamera.camera);

	EndMode2D();

	EndTextureMode();


	//EVERYTHING INTENDED TO APPEAR WHILE RECORDING ABOVE


	//END OF PARTICLES RENDER PASS
	//-------------------------------------------------//
	//BEGINNNG OF UI RENDER PASS


	//EVERYTHING NOT INTENDED TO APPEAR WHILE RECORDING BELOW
	BeginTextureMode(myUITexture);

	ClearBackground({ 0,0,0,0 });

	BeginMode2D(myParam.myCamera.camera);

	myVar.mouseWorldPos = glm::vec2(GetScreenToWorld2D(GetMousePosition(), myParam.myCamera.camera).x,
		GetScreenToWorld2D(GetMousePosition(), myParam.myCamera.camera).y);
	myParam.brush.drawBrush(myVar.mouseWorldPos);
	DrawRectangleLinesEx({ 0,0, static_cast<float>(myVar.domainSize.x), static_cast<float>(myVar.domainSize.y) }, 3, GRAY);

	// Z-Curves debug toggle
	if (myParam.pParticles.size() > 1 && myVar.drawZCurves) {
		for (size_t i = 0; i < myParam.pParticles.size() - 1; i++) {
			DrawLineV({ myParam.pParticles[i].pos.x, myParam.pParticles[i].pos.y }, { myParam.pParticles[i + 1].pos.x,myParam.pParticles[i + 1].pos.y }, WHITE);

			DrawText(TextFormat("%i", i), static_cast<int>(myParam.pParticles[i].pos.x), static_cast<int>(myParam.pParticles[i].pos.y) - 10, 10, { 128,128,128,128 });
		}
	}

	if (myVar.isOpticsEnabled) {
		lighting.drawScene();
		lighting.drawMisc(myVar, myParam);
	}

	EndMode2D();

	// EVERYTHING NON-STATIC RELATIVE TO CAMERA ABOVE

	// EVERYTHING STATIC RELATIVE TO CAMERA BELOW

	if (!introActive) {
		myUI.uiLogic(myParam, myVar, sph, save, geSound, lighting, field);
	}

	save.saveLoadLogic(myVar, myParam, sph, physics, lighting, field);

	myParam.subdivision.subdivideParticles(myVar, myParam);

	EndTextureMode();

	BeginTextureMode(myMiscTexture);

	ClearBackground({ 0,0,0,0 });

	// ---- Intro screen ---- //

	if (introActive) {

		if (introStartTime == 0.0f) {
			introStartTime = GetTime();
		}

		float introElapsedTime = GetTime() - introStartTime;
		float fadeProgress = introElapsedTime / introDuration;

		if (introElapsedTime >= introDuration) {
			introActive = false;
			fadeStartTime = GetTime();
		}

		DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), BLACK);

		const char* text = nullptr;

		if (messageIndex < introMessages.size()) {
			text = introMessages[messageIndex].first.c_str();
		}
		else {
			text = "Welcome back to Galaxy Engine, friend";
		}

		int fontSize = myVar.introFontSize;

		Font fontToUse = (myVar.customFont.texture.id != 0) ? myVar.customFont : GetFontDefault();

		Vector2 textSize = MeasureTextEx(fontToUse, text, fontSize, 1.0f);
		int posX = (GetScreenWidth() - textSize.x) * 0.5f;
		int posY = (GetScreenHeight() - textSize.y) * 0.5f;

		float textAlpha;
		if (fadeProgress < 0.2f) {
			textAlpha = fadeProgress / 0.2f;
		}
		else if (fadeProgress > 0.8f) {
			textAlpha = 1.0f - ((fadeProgress - 0.8f) / 0.2f);
		}
		else {
			textAlpha = 1.0f;
		}

		Color textColor = Fade(WHITE, textAlpha);

		DrawTextEx(fontToUse, text, { static_cast<float>(posX), static_cast<float>(posY) }, fontSize, 1.0f, textColor);
	}
	else if (fadeActive) {
		float fadeElapsedTime = GetTime() - fadeStartTime;

		if (fadeElapsedTime >= fadeDuration) {
			fadeActive = false;
		}
		else {
			float alpha = 1.0f - (fadeElapsedTime / fadeDuration);
			alpha = Clamp(alpha, 0.0f, 1.0f);
			DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, alpha));
		}
	}

	if (firstTimeOpened && !introActive) {
		messageIndex++;
		messageIndex = std::min(static_cast<size_t>(messageIndex), introMessages.size());

		firstTimeOpened = false;
	}

	// ---- Intro screen ---- //

	EndTextureMode();
}

float lastGlobalVolume = -1.0f;
float lastMenuVolume = -1.0f;
float lastMusicVolume = -1.0f;
int lastMessageIndex = -1;

void saveConfigIfChanged() {
	if (geSound.globalVolume != lastGlobalVolume ||
		geSound.menuVolume != lastMenuVolume ||
		geSound.musicVolume != lastMusicVolume ||
		messageIndex != lastMessageIndex) {

		saveConfig();

		lastGlobalVolume = geSound.globalVolume;
		lastMenuVolume = geSound.menuVolume;
		lastMusicVolume = geSound.musicVolume;
		lastMessageIndex = messageIndex;
	}
}

void saveConfig() {
	if (!std::filesystem::exists("Config")) {
		std::filesystem::create_directory("Config");
	}
	std::ofstream file("Config/config.txt");
	YAML::Emitter out(file);
	out << YAML::BeginMap;
	out << YAML::Key << "Global Volume" << YAML::Value << geSound.globalVolume;
	out << YAML::Key << "Menu Volume" << YAML::Value << geSound.menuVolume;
	out << YAML::Key << "Music Volume" << YAML::Value << geSound.musicVolume;
	out << YAML::Key << "Message Index" << YAML::Value << messageIndex;
	out << YAML::EndMap;
}

void loadConfig() {

	YAML::Node config = YAML::LoadFile("Config/config.txt");

	if (config["Global Volume"])
		geSound.globalVolume = config["Global Volume"].as<float>();
	if (config["Menu Volume"])
		geSound.menuVolume = config["Menu Volume"].as<float>();
	if (config["Music Volume"])
		geSound.musicVolume = config["Music Volume"].as<float>();
	if (config["Message Index"])
		messageIndex = config["Message Index"].as<float>();
}

void enableMultiThreading() {
	if (myVar.isMultiThreadingEnabled) {
		omp_set_num_threads(myVar.threadsAmount);
	}
	else {
		omp_set_num_threads(1);
	}
}

void fullscreenToggle(int& lastScreenWidth, int& lastScreenHeight,
	bool& wasFullscreen, bool& lastScreenState,
	RenderTexture2D& myParticlesTexture, RenderTexture2D& myUITexture) {

	if (myVar.fullscreenState != lastScreenState) {
		int monitor = GetCurrentMonitor();

		if (!IsWindowMaximized())
			SetWindowSize(static_cast<float>(GetMonitorWidth(monitor)) * 0.5f, static_cast<float>(GetMonitorHeight(monitor)) * 0.5f);
		else
			SetWindowSize(static_cast<float>(GetMonitorWidth(monitor)) * 0.5f, static_cast<float>(GetMonitorHeight(monitor)) * 0.5f);
		ToggleBorderlessWindowed();
		wasFullscreen = IsWindowMaximized();;

		UnloadRenderTexture(myParticlesTexture);
		UnloadRenderTexture(myUITexture);

		lastScreenWidth = GetScreenWidth();
		lastScreenHeight = GetScreenHeight();
		lastScreenState = myVar.fullscreenState;

		myParticlesTexture = CreateFloatRenderTexture(lastScreenWidth, lastScreenHeight);;
		myUITexture = LoadRenderTexture(lastScreenWidth, lastScreenHeight);
	}

	int currentScreenWidth = GetScreenWidth();
	int currentScreenHeight = GetScreenHeight();

	if (currentScreenWidth != lastScreenWidth || currentScreenHeight != lastScreenHeight) {
		UnloadRenderTexture(myParticlesTexture);
		UnloadRenderTexture(myUITexture);

		myParticlesTexture = CreateFloatRenderTexture(currentScreenWidth, currentScreenHeight);
		myUITexture = LoadRenderTexture(currentScreenWidth, currentScreenHeight);

		lastScreenWidth = currentScreenWidth;
		lastScreenHeight = currentScreenHeight;
	}
}


RenderTexture2D CreateFloatRenderTexture(int w, int h) {
	RenderTexture2D fbo = { 0 };
	glGenTextures(1, &fbo.texture.id);
	glBindTexture(GL_TEXTURE_2D, fbo.texture.id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_HALF_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glGenFramebuffers(1, &fbo.id);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo.id);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo.texture.id, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	fbo.texture.width = w;
	fbo.texture.height = h;
	fbo.texture.format = PIXELFORMAT_UNCOMPRESSED_R16G16B16A16;

	return fbo;
}