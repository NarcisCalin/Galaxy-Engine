#pragma once

#include "Particles/particle.h"

#include "parameters.h"
#include "Particles/QueryNeighbors.h"

struct UpdateVariables;
struct UpdateVariables;

struct GridCell {
	std::vector<size_t> particleIndices;
};

class SPH {
public:
	float radiusMultiplier = 3.0f;
	const float boundDamping = -0.1f;

	float densTolerance = 0.08f;

	int maxIter = 1; // I keep only 1 iteration when I don't use the density error condition
	int iter = 0;

	float rhoError = 0.0f;

	float cellSize;

	SPH() : cellSize(radiusMultiplier) {}

	void computeViscCohesionForcesWithCache(
		std::vector<ParticlePhysics>& pParticles,
		std::vector<ParticleRendering>& rParticles,
		std::vector<glm::vec2>& forces,
		const std::vector<std::vector<size_t>>& neighborCache,
		size_t N);

	float smoothingKernel(float dst, float radiusMultiplier) {
		if (dst >= radiusMultiplier) return 0.0f;

		float volume = (PI * pow(radiusMultiplier, 4.0f)) / 6.0f;
		return (radiusMultiplier - dst) * (radiusMultiplier - dst) / volume;
	}

	float spikyKernelDerivative(float dst, float radiusMultiplier) {
		if (dst >= radiusMultiplier) return 0.0f;

		float scale = -45.0f / (PI * pow(radiusMultiplier, 6.0f));
		return scale * pow(radiusMultiplier - dst, 2.0f);
	}

	float smoothingKernelLaplacian(float dst, float radiusMultiplier) {
		if (dst >= radiusMultiplier) return 0.0f;

		float scale = 45.0f / (PI * pow(radiusMultiplier, 6.0f));
		return scale;
	}

	float smoothingKernelCohesion(float r, float h) {
		if (r >= h) return 0.0f;

		float q = r / h;
		return (1.0f - q) * (0.5f - q) * (0.5f - q) * 30.0f / (PI * h * h);
	}

	// Currently unused
	float computeDelta(const std::vector<glm::vec2>& kernelGradients, float dt, float mass, float restDensity) {
		float beta = (dt * dt * mass * mass) / (restDensity * restDensity);

		glm::vec2 sumGradW = { 0.0f, 0.0f };
		float sumGradW_dot = 0.0f;

		for (const glm::vec2& gradW : kernelGradients) {
			sumGradW.x += gradW.x;
			sumGradW.y += gradW.y;

			sumGradW_dot += gradW.x * gradW.x + gradW.y * gradW.y;
		}

		float sumDot = sumGradW.x * sumGradW.x + sumGradW.y * sumGradW.y;

		float delta = -1.0f / (beta * (-sumDot - sumGradW_dot));

		return delta;
	}

	// New Grid Search

	struct EntryArrays {
		std::vector<uint32_t> cellKeys;
		std::vector<uint32_t> particleIndices;
		std::vector<int> cellXs;
		std::vector<int> cellYs;
		size_t size;
	};

	const uint32_t hashTableSize = 16384;
	EntryArrays entries;
	std::vector<uint32_t> startIndices;

	const char* neighborSearchCompute = R"(
#version 430

layout(local_size_x = 256) in;

layout(std430, binding = 10) buffer ParticlePositions {
    vec2 pos[];
};

layout(std430, binding = 11) buffer CellKeys {
    uint cellKeys[];
};

layout(std430, binding = 12) buffer ParticleIndices {
    uint particleIndices[];
};

layout(std430, binding = 13) buffer CellXs {
    int cellXs[];
};

layout(std430, binding = 14) buffer CellYs {
    int cellYs[];
};

uniform float cellSize;
uniform uint hashTableSize;
uniform uint numParticles;

uvec2 posToCellCoord(vec2 pos) {
    int cellX = int(floor(pos.x / cellSize));
    int cellY = int(floor(pos.y / cellSize));
    return uvec2(cellX, cellY);
}

uint hashCell(int cellX, int cellY) {
    uint h = uint(cellX * 73856093) ^ uint(cellY * 19349663);
    return h % hashTableSize;
}

void main() {
    uint i = gl_GlobalInvocationID.x;
    if (i >= numParticles) return;

    uvec2 cellCoord = posToCellCoord(pos[i]);
    uint cellKey = hashCell(int(cellCoord.x), int(cellCoord.y));

    cellKeys[i] = cellKey;
    particleIndices[i] = i;
    cellXs[i] = int(cellCoord.x);
    cellYs[i] = int(cellCoord.y);
}
)";

	GLuint ssboPPos, ssboCellKeys, ssboParticleIndices, ssboCellXs, ssboCellYs;

	size_t mb = 512;

	size_t reserveSize = (1024 * 1024 * mb) / sizeof(glm::vec2);

	GLuint neighborSearchProgram;

	void neighborSearchKernel() {

		glGenBuffers(1, &ssboPPos);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboPPos);
		glBufferData(GL_SHADER_STORAGE_BUFFER, reserveSize * sizeof(glm::vec2), nullptr, GL_STREAM_COPY);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, ssboPPos);

		glGenBuffers(1, &ssboCellKeys);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboCellKeys);
		glBufferData(GL_SHADER_STORAGE_BUFFER, 100000 * sizeof(uint32_t), nullptr, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, ssboCellKeys);

		glGenBuffers(1, &ssboParticleIndices);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboParticleIndices);
		glBufferData(GL_SHADER_STORAGE_BUFFER, 100000 * sizeof(uint32_t), nullptr, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 12, ssboParticleIndices);

		glGenBuffers(1, &ssboCellXs);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboCellXs);
		glBufferData(GL_SHADER_STORAGE_BUFFER, 100000 * sizeof(int32_t), nullptr, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 13, ssboCellXs);

		glGenBuffers(1, &ssboCellYs);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboCellYs);
		glBufferData(GL_SHADER_STORAGE_BUFFER, 100000 * sizeof(int32_t), nullptr, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 14, ssboCellYs);

		neighborSearchProgram = glCreateProgram();
		GLuint shader = glCreateShader(GL_COMPUTE_SHADER);
		glShaderSource(shader, 1, &neighborSearchCompute, nullptr);
		glCompileShader(shader);

		GLint success;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

		if (!success) {
			char infoLog[512];
			glGetShaderInfoLog(shader, 512, nullptr, infoLog);
			std::cerr << "Compute shader compilation failed:\n" << infoLog << std::endl;
		}

		glAttachShader(neighborSearchProgram, shader);
		glLinkProgram(neighborSearchProgram);

		glGetProgramiv(neighborSearchProgram, GL_LINK_STATUS, &success);
		if (!success) {
			char infoLog[512];
			glGetProgramInfoLog(neighborSearchProgram, 512, nullptr, infoLog);
			std::cerr << "Shader neighborSearchProgram linking failed:\n" << infoLog << std::endl;
		}

		glDeleteShader(shader);
	}

	std::vector<glm::vec2> pPos;

	void gpuNeighborSearch(std::vector<ParticlePhysics>& pParticles) {
		if (pParticles.empty()) return;
		size_t n = pParticles.size();

		entries.cellKeys.resize(n);
		entries.particleIndices.resize(n);
		entries.cellXs.resize(n);
		entries.cellYs.resize(n);
		pPos.resize(n);

		for (size_t i = 0; i < n; i++) {
			pPos[i] = pParticles[i].pos;
		}

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboPPos);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, n * sizeof(glm::vec2), pPos.data());

		glUseProgram(neighborSearchProgram);
		glUniform1f(glGetUniformLocation(neighborSearchProgram, "cellSize"), cellSize);
		glUniform1ui(glGetUniformLocation(neighborSearchProgram, "hashTableSize"), hashTableSize);
		glUniform1ui(glGetUniformLocation(neighborSearchProgram, "numParticles"), (GLuint)n);

		GLuint numGroups = (GLuint)((n + 255) / 256);
		glDispatchCompute(numGroups, 1, 1);

		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}

	const char* bitonicSortCompute = R"(
#version 430
layout(local_size_x = 256) in;

layout(std430, binding = 11) buffer CellKeys {
    uint cellKeys[];
};

layout(std430, binding = 12) buffer ParticleIndices {
    uint particleIndices[];
};

layout(std430, binding = 13) buffer CellXs {
    int cellXs[];
};

layout(std430, binding = 14) buffer CellYs {
    int cellYs[];
};

uniform uint numValues;
uniform uint groupWidth;
uniform uint groupHeight;
uniform uint stepIndex;

void main() {
    uint i = gl_GlobalInvocationID.x;

    uint hIndex = i & (groupWidth - 1);
    uint indexLeft = hIndex + (groupHeight + 1) * (i / groupWidth);
    uint rightStepSize = stepIndex == 0 ? groupHeight - 2 * hIndex : (groupHeight + 1) / 2;
    uint indexRight = indexLeft + rightStepSize;

    if (indexRight >= numValues) return;

    uint valueLeft = cellKeys[indexLeft];
    uint valueRight = cellKeys[indexRight];

    if (valueLeft > valueRight)
    {
        uint tempCellKey = cellKeys[indexLeft];
        cellKeys[indexLeft] = cellKeys[indexRight];
        cellKeys[indexRight] = tempCellKey;

        uint tempIndices = particleIndices[indexLeft];
        particleIndices[indexLeft] = particleIndices[indexRight];
        particleIndices[indexRight] = tempIndices;

        int tempX = cellXs[indexLeft];
        cellXs[indexLeft] = cellXs[indexRight];
        cellXs[indexRight] = tempX;

        int tempY = cellYs[indexLeft];
        cellYs[indexLeft] = cellYs[indexRight];
        cellYs[indexRight] = tempY;
    }
}
)";

	GLuint bitonicSortProgram;

	void bitonicSortKernel() {

		bitonicSortProgram = glCreateProgram();
		GLuint shader = glCreateShader(GL_COMPUTE_SHADER);
		glShaderSource(shader, 1, &bitonicSortCompute, nullptr);
		glCompileShader(shader);

		GLint success;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

		if (!success) {
			char infoLog[512];
			glGetShaderInfoLog(shader, 512, nullptr, infoLog);
			std::cerr << "Compute shader compilation failed:\n" << infoLog << std::endl;
		}

		glAttachShader(bitonicSortProgram, shader);
		glLinkProgram(bitonicSortProgram);

		glGetProgramiv(bitonicSortProgram, GL_LINK_STATUS, &success);
		if (!success) {
			char infoLog[512];
			glGetProgramInfoLog(bitonicSortProgram, 512, nullptr, infoLog);
			std::cerr << "Shader bitonicSortProgram linking failed:\n" << infoLog << std::endl;
		}

		glDeleteShader(shader);
	}

	void gpuBitonicSort() {
		const size_t n = entries.cellKeys.size();
		if (n == 0) return;

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, ssboCellKeys);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 12, ssboParticleIndices);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 13, ssboCellXs);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 14, ssboCellYs);

		glUseProgram(bitonicSortProgram);
		glUniform1ui(glGetUniformLocation(bitonicSortProgram, "numValues"), n);

		int numStages = static_cast<int>(log2(nextPowerOfTwo(n)));

		for (int stageIndex = 0; stageIndex < numStages; stageIndex++) {
			for (int stepIndex = 0; stepIndex < stageIndex + 1; stepIndex++) {

				int groupWidth = 1 << (stageIndex - stepIndex);
				int groupHeight = 2 * groupWidth - 1;

				glUniform1ui(glGetUniformLocation(bitonicSortProgram, "groupWidth"), static_cast<uint32_t>(groupWidth));
				glUniform1ui(glGetUniformLocation(bitonicSortProgram, "groupHeight"), static_cast<uint32_t>(groupHeight));
				glUniform1ui(glGetUniformLocation(bitonicSortProgram, "stepIndex"), static_cast<uint32_t>(stepIndex));

				GLuint numThreads = nextPowerOfTwo(n) / 2;
				GLuint numGroups = (numThreads + 255) / 256;
				glDispatchCompute(numGroups, 1, 1);
				glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
			}
		}
	}

	const char* offsetCompute = R"(
#version 430

layout(local_size_x = 256) in;

layout(std430, binding = 11) buffer CellKeys {
    uint cellKeys[];
};


layout(std430, binding = 15) buffer Offsets {
    uint offsets[];
};

uniform uint numEntries;
uniform uint hashTableSize;

void main() {

    uint i = gl_GlobalInvocationID.x; 
    if (i >= numEntries) return;

    uint null = hashTableSize;

    uint key = cellKeys[i]; 

    uint keyPrev = (i == 0) ? null : cellKeys[i - 1];

    if (key != keyPrev && key < hashTableSize) {
        offsets[key] = i;
    }
}
)";

	GLuint ssboOffsets;

	GLuint offsetProgram;

	void offsetKernel() {

		glGenBuffers(1, &ssboOffsets);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboOffsets);
		glBufferData(GL_SHADER_STORAGE_BUFFER,
			hashTableSize * sizeof(uint32_t),
			nullptr,
			GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 15, ssboOffsets);

		offsetProgram = glCreateProgram();
		GLuint shader = glCreateShader(GL_COMPUTE_SHADER);
		glShaderSource(shader, 1, &offsetCompute, nullptr);
		glCompileShader(shader);

		GLint success;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

		if (!success) {
			char infoLog[512];
			glGetShaderInfoLog(shader, 512, nullptr, infoLog);
			std::cerr << "Compute shader compilation failed:\n" << infoLog << std::endl;
		}

		glAttachShader(offsetProgram, shader);
		glLinkProgram(offsetProgram);

		glGetProgramiv(offsetProgram, GL_LINK_STATUS, &success);
		if (!success) {
			char infoLog[512];
			glGetProgramInfoLog(offsetProgram, 512, nullptr, infoLog);
			std::cerr << "Shader offsetProgram linking failed:\n" << infoLog << std::endl;
		}

		glDeleteShader(shader);
	}

	unsigned int nextPowerOfTwo(uint32_t n) {
		if (n == 0) return 1;
		n--;
		n |= n >> 1;
		n |= n >> 2;
		n |= n >> 4;
		n |= n >> 8;
		n |= n >> 16;
		return n + 1;
	}

	const char* offsetsResetCompute = R"(
#version 430
layout(local_size_x = 256) in;

layout(std430, binding = 15) buffer Offsets {
    uint offsets[];
};

uniform uint hashTableSize;

void main() {
    uint i = gl_GlobalInvocationID.x;
    if (i >= hashTableSize) return;
    offsets[i] = 4294967295; // UINT_MAX
}
)";

	GLuint offsetsResetProgram;

	void offsetResetKernel() {

		offsetsResetProgram = glCreateProgram();
		GLuint shader = glCreateShader(GL_COMPUTE_SHADER);
		glShaderSource(shader, 1, &offsetsResetCompute, nullptr);
		glCompileShader(shader);

		GLint success;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

		if (!success) {
			char infoLog[512];
			glGetShaderInfoLog(shader, 512, nullptr, infoLog);
			std::cerr << "Compute shader compilation failed:\n" << infoLog << std::endl;
		}

		glAttachShader(offsetsResetProgram, shader);
		glLinkProgram(offsetsResetProgram);

		glGetProgramiv(offsetsResetProgram, GL_LINK_STATUS, &success);
		if (!success) {
			char infoLog[512];
			glGetProgramInfoLog(offsetsResetProgram, 512, nullptr, infoLog);
			std::cerr << "Shader offsetsResetProgram linking failed:\n" << infoLog << std::endl;
		}

		glDeleteShader(shader);
	}

	void gpuOffsets() {
		const size_t n = entries.cellKeys.size();
		if (n == 0) return;

		glUseProgram(offsetsResetProgram);

		glUniform1ui(glGetUniformLocation(offsetsResetProgram, "hashTableSize"), hashTableSize);

		glDispatchCompute((hashTableSize + 255) / 256, 1, 1);

		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, ssboCellKeys);

		glUseProgram(offsetProgram);
		glUniform1ui(glGetUniformLocation(offsetProgram, "numEntries"), (uint32_t)n);
		glUniform1ui(glGetUniformLocation(offsetProgram, "hashTableSize"), hashTableSize);

		glDispatchCompute((n + 255) / 256, 1, 1);

		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboOffsets);
		uint32_t* ptrOffsets = (uint32_t*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
		if (ptrOffsets) {
			memcpy(startIndices.data(), ptrOffsets, hashTableSize * sizeof(uint32_t));
			glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		}
	}

	void newGridGPU(std::vector<ParticlePhysics>& pParticles) {
		const size_t n = pParticles.size();
		if (n == 0) return;

		entries.size = n;
		startIndices.assign(hashTableSize, UINT32_MAX);

		gpuNeighborSearch(pParticles);

		gpuBitonicSort();

		/*if (n > 0) {
			startIndices[entries.cellKeys[0]] = 0;
		}
		for (size_t i = 1; i < n; i++) {
			if (entries.cellKeys[i - 1] != entries.cellKeys[i]) {
				startIndices[entries.cellKeys[i]] = i;
			}
		}*/

		gpuOffsets();

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboCellKeys);

		uint32_t* ptrCellKeys = (uint32_t*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);

		memcpy(entries.cellKeys.data(), ptrCellKeys, entries.cellKeys.size() * sizeof(uint32_t));

		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);


		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboParticleIndices);

		uint32_t* ptrIndices = (uint32_t*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);

		memcpy(entries.particleIndices.data(), ptrIndices, entries.particleIndices.size() * sizeof(uint32_t));

		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);


		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboCellXs);

		uint32_t* ptrCellX = (uint32_t*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);

		memcpy(entries.cellXs.data(), ptrCellX, entries.cellXs.size() * sizeof(int32_t));

		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);


		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboCellYs);

		uint32_t* ptrCellY = (uint32_t*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);

		memcpy(entries.cellYs.data(), ptrCellY, entries.cellYs.size() * sizeof(int32_t));

		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	}

	std::vector<float> posX;
	std::vector<float> posY;
	std::vector<float> predPosX;
	std::vector<float> predPosY;
	std::vector<float> accX;
	std::vector<float> accY;
	std::vector<float> velX;
	std::vector<float> velY;
	std::vector<float> prevVelX;
	std::vector<float> prevVelY;
	std::vector<float> sphMass;
	std::vector<float> press;
	std::vector<float> pressFX;
	std::vector<float> pressFY;
	std::vector<float> stiff;
	std::vector<float> visc;
	std::vector<float> dens;
	std::vector<float> predDens;
	std::vector<float> restDens;

	void flattenParticles(std::vector<ParticlePhysics>& pParticles);

	void readFlattenBack(std::vector<ParticlePhysics>& pParticles);

	void computeViscCohesionForces(UpdateVariables& myVar, UpdateParameters& myParam, std::vector<glm::vec2>& sphForce, size_t& N);

	void groundModeBoundary(std::vector<ParticlePhysics>& pParticles,
		std::vector<ParticleRendering>& rParticles,
		glm::vec2 domainSize, UpdateVariables& myVar);

	void PCISPH(UpdateVariables& myVar, UpdateParameters& myParam);

	void pcisphSolver(UpdateVariables& myVar, UpdateParameters& myParam) {

		//newGridGPU(pParticles);

		PCISPH(myVar, myParam);

		if (myVar.sphGround) {
			groundModeBoundary(myParam.pParticles, myParam.rParticles, myVar.domainSize, myVar);
		}
	}
};