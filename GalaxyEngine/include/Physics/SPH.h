#pragma once

#include "Particles/particle.h"

#include "parameters.h"

struct UpdateVariables;
struct UpdateVariables;

struct GridCell {
	std::vector<size_t> particleIndices;
};

class SPH {
public:
	float radiusMultiplier = 3.0f;
	float mass = 0.03f;
	float stiffMultiplier = 1.0f;
	float viscosity = 0.3f;
	float cohesionCoefficient = 1.0f;
	const float boundDamping = -0.1f;
	float delta = 19000.0f;
	float verticalGravity = 3.0f;
	float boundaryFriction = 0.8f;

	float densTolerance = 0.08f;

	int maxIter = 1; // I keep only 1 iteration when I don't use the density error condition
	int iter = 0;

	float rhoError = 0.0f;

	float cellSize;

	SPH() : cellSize(radiusMultiplier) {}

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

	std::pair<int, int> posToCellCoord(glm::vec2& pos) {
		int cellX = static_cast<int>(floor(pos.x / cellSize));
		int cellY = static_cast<int>(floor(pos.y / cellSize));
		return { cellX, cellY };
	}

	uint32_t hashCell(int cellX, int cellY) {
		uint32_t h = (uint32_t)((cellX * 73856093) ^ (cellY * 19349663));
		return h % hashTableSize;
	}

	uint32_t getKey(size_t hash) {
		if (entries.size == 0) return 0;
		uint32_t cellKey = hash % hashTableSize;
		return cellKey;
	}

	__attribute__((target("avx2")))
		void newGridAVX2(std::vector<ParticlePhysics>& pParticles) {
		const size_t n = pParticles.size();
		if (n == 0) return;

		entries.cellKeys.resize(n);
		entries.particleIndices.resize(n);
		entries.cellXs.resize(n);
		entries.cellYs.resize(n);
		entries.size = n;
		startIndices.assign(hashTableSize, UINT32_MAX);

#pragma omp parallel for
		for (size_t i = 0; i < n; i++) {
			auto cellCoord = posToCellCoord(pParticles[i].pos);
			uint32_t cellKey = hashCell(cellCoord.first, cellCoord.second);
			entries.cellKeys[i] = cellKey;
			entries.particleIndices[i] = i;
			entries.cellXs[i] = cellCoord.first;
			entries.cellYs[i] = cellCoord.second;
		}

		std::vector<uint32_t> indices(n);
		std::iota(indices.begin(), indices.end(), 0);

		std::sort(std::execution::par_unseq, indices.begin(), indices.end(),
			[&](uint32_t a, uint32_t b) {
				return entries.cellKeys[a] < entries.cellKeys[b];
			}
		);

		EntryArrays sorted;
		sorted.cellKeys.resize(n);
		sorted.particleIndices.resize(n);
		sorted.cellXs.resize(n);
		sorted.cellYs.resize(n);
		sorted.size = n;

		size_t i = 0;

#pragma omp parallel for
		for (i = 0; i < (n & ~7); i += 8) {
			uint32_t idx0 = indices[i];
			uint32_t idx1 = indices[i + 1];
			uint32_t idx2 = indices[i + 2];
			uint32_t idx3 = indices[i + 3];
			uint32_t idx4 = indices[i + 4];
			uint32_t idx5 = indices[i + 5];
			uint32_t idx6 = indices[i + 6];
			uint32_t idx7 = indices[i + 7];

			sorted.cellKeys[i] = entries.cellKeys[idx0];
			sorted.cellKeys[i + 1] = entries.cellKeys[idx1];
			sorted.cellKeys[i + 2] = entries.cellKeys[idx2];
			sorted.cellKeys[i + 3] = entries.cellKeys[idx3];
			sorted.cellKeys[i + 4] = entries.cellKeys[idx4];
			sorted.cellKeys[i + 5] = entries.cellKeys[idx5];
			sorted.cellKeys[i + 6] = entries.cellKeys[idx6];
			sorted.cellKeys[i + 7] = entries.cellKeys[idx7];

			sorted.particleIndices[i] = entries.particleIndices[idx0];
			sorted.particleIndices[i + 1] = entries.particleIndices[idx1];
			sorted.particleIndices[i + 2] = entries.particleIndices[idx2];
			sorted.particleIndices[i + 3] = entries.particleIndices[idx3];
			sorted.particleIndices[i + 4] = entries.particleIndices[idx4];
			sorted.particleIndices[i + 5] = entries.particleIndices[idx5];
			sorted.particleIndices[i + 6] = entries.particleIndices[idx6];
			sorted.particleIndices[i + 7] = entries.particleIndices[idx7];

			__m256i cellXVec = _mm256_set_epi32(
				entries.cellXs[idx7], entries.cellXs[idx6],
				entries.cellXs[idx5], entries.cellXs[idx4],
				entries.cellXs[idx3], entries.cellXs[idx2],
				entries.cellXs[idx1], entries.cellXs[idx0]
			);
			_mm256_storeu_si256((__m256i*) & sorted.cellXs[i], cellXVec);

			__m256i cellYVec = _mm256_set_epi32(
				entries.cellYs[idx7], entries.cellYs[idx6],
				entries.cellYs[idx5], entries.cellYs[idx4],
				entries.cellYs[idx3], entries.cellYs[idx2],
				entries.cellYs[idx1], entries.cellYs[idx0]
			);
			_mm256_storeu_si256((__m256i*) & sorted.cellYs[i], cellYVec);
		}

		for (; i < n; i++) {
			size_t idx = indices[i];
			sorted.cellKeys[i] = entries.cellKeys[idx];
			sorted.particleIndices[i] = entries.particleIndices[idx];
			sorted.cellXs[i] = entries.cellXs[idx];
			sorted.cellYs[i] = entries.cellYs[idx];
		}

		entries = std::move(sorted);

		if (n > 0) {
			startIndices[entries.cellKeys[0]] = 0;
		}
		for (size_t i = 1; i < n; i++) {
			if (entries.cellKeys[i - 1] != entries.cellKeys[i]) {
				startIndices[entries.cellKeys[i]] = i;
			}
		}
	}

	void newGrid(std::vector<ParticlePhysics>& pParticles) {
		if (hasAVX2Support()) {
			newGridAVX2(pParticles);
		}
		else {
			newGridScalar(pParticles);
		}
	}

	void newGridScalar(std::vector<ParticlePhysics>& pParticles) {
		const size_t n = pParticles.size();
		if (n == 0) return;

		entries.cellKeys.resize(n);
		entries.particleIndices.resize(n);
		entries.cellXs.resize(n);
		entries.cellYs.resize(n);
		entries.size = n;
		startIndices.assign(hashTableSize, UINT32_MAX);

#pragma omp parallel for
		for (size_t i = 0; i < n; i++) {
			auto cellCoord = posToCellCoord(pParticles[i].pos);
			uint32_t cellKey = hashCell(cellCoord.first, cellCoord.second);
			entries.cellKeys[i] = cellKey;
			entries.particleIndices[i] = i;
			entries.cellXs[i] = cellCoord.first;
			entries.cellYs[i] = cellCoord.second;
		}

		std::vector<uint32_t> indices(n);
		std::iota(indices.begin(), indices.end(), 0);

		std::sort(std::execution::par_unseq, indices.begin(), indices.end(),
			[&](uint32_t a, uint32_t b) {
				return entries.cellKeys[a] < entries.cellKeys[b];
			}
		);

		EntryArrays sorted;
		sorted.cellKeys.resize(n);
		sorted.particleIndices.resize(n);
		sorted.cellXs.resize(n);
		sorted.cellYs.resize(n);
		sorted.size = n;

		size_t i = 0;

#pragma omp parallel for
		for (size_t i = 0; i < (n & ~7); i += 8) {
			size_t idx0 = indices[i];
			size_t idx1 = indices[i + 1];
			size_t idx2 = indices[i + 2];
			size_t idx3 = indices[i + 3];
			size_t idx4 = indices[i + 4];
			size_t idx5 = indices[i + 5];
			size_t idx6 = indices[i + 6];
			size_t idx7 = indices[i + 7];

			sorted.cellKeys[i] = entries.cellKeys[idx0];
			sorted.cellKeys[i + 1] = entries.cellKeys[idx1];
			sorted.cellKeys[i + 2] = entries.cellKeys[idx2];
			sorted.cellKeys[i + 3] = entries.cellKeys[idx3];
			sorted.cellKeys[i + 4] = entries.cellKeys[idx4];
			sorted.cellKeys[i + 5] = entries.cellKeys[idx5];
			sorted.cellKeys[i + 6] = entries.cellKeys[idx6];
			sorted.cellKeys[i + 7] = entries.cellKeys[idx7];

			sorted.particleIndices[i] = entries.particleIndices[idx0];
			sorted.particleIndices[i + 1] = entries.particleIndices[idx1];
			sorted.particleIndices[i + 2] = entries.particleIndices[idx2];
			sorted.particleIndices[i + 3] = entries.particleIndices[idx3];
			sorted.particleIndices[i + 4] = entries.particleIndices[idx4];
			sorted.particleIndices[i + 5] = entries.particleIndices[idx5];
			sorted.particleIndices[i + 6] = entries.particleIndices[idx6];
			sorted.particleIndices[i + 7] = entries.particleIndices[idx7];

			sorted.cellXs[i] = entries.cellXs[idx0];
			sorted.cellXs[i + 1] = entries.cellXs[idx1];
			sorted.cellXs[i + 2] = entries.cellXs[idx2];
			sorted.cellXs[i + 3] = entries.cellXs[idx3];
			sorted.cellXs[i + 4] = entries.cellXs[idx4];
			sorted.cellXs[i + 5] = entries.cellXs[idx5];
			sorted.cellXs[i + 6] = entries.cellXs[idx6];
			sorted.cellXs[i + 7] = entries.cellXs[idx7];

			sorted.cellYs[i] = entries.cellYs[idx0];
			sorted.cellYs[i + 1] = entries.cellYs[idx1];
			sorted.cellYs[i + 2] = entries.cellYs[idx2];
			sorted.cellYs[i + 3] = entries.cellYs[idx3];
			sorted.cellYs[i + 4] = entries.cellYs[idx4];
			sorted.cellYs[i + 5] = entries.cellYs[idx5];
			sorted.cellYs[i + 6] = entries.cellYs[idx6];
			sorted.cellYs[i + 7] = entries.cellYs[idx7];
		}

		for (; i < n; i++) {
			uint32_t idx = indices[i];
			sorted.cellKeys[i] = entries.cellKeys[idx];
			sorted.particleIndices[i] = entries.particleIndices[idx];
			sorted.cellXs[i] = entries.cellXs[idx];
			sorted.cellYs[i] = entries.cellYs[idx];
		}

		entries = std::move(sorted);

		if (n > 0) {
			startIndices[entries.cellKeys[0]] = 0;
		}
		for (size_t i = 1; i < n; i++) {
			if (entries.cellKeys[i - 1] != entries.cellKeys[i]) {
				startIndices[entries.cellKeys[i]] = i;
			}
		}
	}

#ifdef _MSC_VER
#include <intrin.h>
#else
#include <cpuid.h>
#endif

	bool hasAVX2Support() {
		static int cached = -1;
		if (cached != -1) return cached;

		int cpuInfo[4];
#ifdef _MSC_VER
		__cpuidex(cpuInfo, 7, 0);
#else
		__cpuid_count(7, 0, cpuInfo[0], cpuInfo[1], cpuInfo[2], cpuInfo[3]);
#endif
		cached = (cpuInfo[1] & (1 << 5)) != 0;
		return cached;
	}

	__attribute__((target("avx2")))
		std::vector<size_t> queryNeighborsAVX2(glm::vec2& pos, std::vector<ParticlePhysics>& pParticles) {
		std::vector<size_t> neighbors;
		neighbors.reserve(64);

		auto [cellX, cellY] = posToCellCoord(pos);

		for (int dx = -1; dx <= 1; dx++) {
			for (int dy = -1; dy <= 1; dy++) {
				int neighborX = cellX + dx;
				int neighborY = cellY + dy;
				uint32_t cellKey = hashCell(neighborX, neighborY);
				uint32_t start = startIndices[cellKey];
				if (start == UINT32_MAX) continue;

				__m256i targetX = _mm256_set1_epi32(neighborX);
				__m256i targetY = _mm256_set1_epi32(neighborY);

				size_t i = start;

				for (; i + 8 <= entries.size && entries.cellKeys[i] == cellKey; i += 8) {
					bool sameCellKey = true;
					for (int k = 0; k < 8; k++) {
						if (entries.cellKeys[i + k] != cellKey) {
							sameCellKey = false;
							break;
						}
					}
					if (!sameCellKey) break;

					__m256i cellXs = _mm256_loadu_si256((__m256i*) & entries.cellXs[i]);
					__m256i cellYs = _mm256_loadu_si256((__m256i*) & entries.cellYs[i]);

					__m256i matchX = _mm256_cmpeq_epi32(cellXs, targetX);
					__m256i matchY = _mm256_cmpeq_epi32(cellYs, targetY);
					__m256i match = _mm256_and_si256(matchX, matchY);

					int mask = _mm256_movemask_ps(_mm256_castsi256_ps(match));

					for (int k = 0; k < 8; k++) {
						if (mask & (1 << k)) {
							neighbors.push_back(entries.particleIndices[i + k]);
						}
					}
				}

				for (; i < entries.size && entries.cellKeys[i] == cellKey; i++) {
					if (entries.cellXs[i] == neighborX && entries.cellYs[i] == neighborY) {
						neighbors.push_back(entries.particleIndices[i]);
					}
				}
			}
		}
		return neighbors;
	}

	std::vector<size_t> queryNeighborsScalar(glm::vec2& pos, std::vector<ParticlePhysics>& pParticles) {
		std::vector<size_t> neighbors;
		neighbors.reserve(64);

		auto [cellX, cellY] = posToCellCoord(pos);

		for (int dx = -1; dx <= 1; dx++) {
			for (int dy = -1; dy <= 1; dy++) {
				int neighborX = cellX + dx;
				int neighborY = cellY + dy;
				size_t cellKey = hashCell(neighborX, neighborY);
				size_t start = startIndices[cellKey];
				if (start == UINT32_MAX) continue;

				for (size_t i = start; i < entries.size && entries.cellKeys[i] == cellKey; i++) {
					if (entries.cellXs[i] == neighborX && entries.cellYs[i] == neighborY) {
						neighbors.push_back(entries.particleIndices[i]);
					}
				}
			}
		}
		return neighbors;
	}

	std::vector<size_t> queryNeighbors(glm::vec2& pos, std::vector<ParticlePhysics>& pParticles) {
		if (hasAVX2Support()) {
			return queryNeighborsAVX2(pos, pParticles);
		}
		else {
			return queryNeighborsScalar(pos, pParticles);
		}
	}

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

	GLuint ssboPPos, ssboCellKeys, ssboParticleIndices, ssboCellX, ssboCellY;

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
		glBufferData(GL_SHADER_STORAGE_BUFFER, reserveSize * sizeof(uint32_t), nullptr, GL_STREAM_COPY);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, ssboCellKeys);

		glGenBuffers(1, &ssboParticleIndices);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboParticleIndices);
		glBufferData(GL_SHADER_STORAGE_BUFFER, reserveSize * sizeof(uint32_t), nullptr, GL_STREAM_COPY);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 12, ssboParticleIndices);

		glGenBuffers(1, &ssboCellX);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboCellX);
		glBufferData(GL_SHADER_STORAGE_BUFFER, reserveSize * sizeof(int), nullptr, GL_STREAM_COPY);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 13, ssboCellX);

		glGenBuffers(1, &ssboCellY);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboCellY);
		glBufferData(GL_SHADER_STORAGE_BUFFER, reserveSize * sizeof(int), nullptr, GL_STREAM_COPY);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 14, ssboCellY);

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

		pPos.resize(n);
		entries.cellKeys.resize(n);
		entries.particleIndices.resize(n);
		entries.cellXs.resize(n);
		entries.cellYs.resize(n);

		for (size_t i = 0; i < n; i++) {
			pPos[i] = pParticles[i].pos;
		}

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboPPos);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, n * sizeof(glm::vec2), pPos.data());

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboCellKeys);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, n * sizeof(uint32_t), entries.cellKeys.data());

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboParticleIndices);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, n * sizeof(uint32_t), entries.particleIndices.data());

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboCellX);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, n * sizeof(int), entries.cellXs.data());

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboCellY);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, n * sizeof(int), entries.cellYs.data());

		glUseProgram(neighborSearchProgram);

		glUniform1f(glGetUniformLocation(neighborSearchProgram, "cellSize"), cellSize);
		glUniform1ui(glGetUniformLocation(neighborSearchProgram, "hashTableSize"), hashTableSize);
		glUniform1ui(glGetUniformLocation(neighborSearchProgram, "numParticles"), (GLuint)n);

		GLuint numGroups = (GLuint)((n + 255) / 256);
		glDispatchCompute(numGroups, 1, 1);

		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboCellKeys);
		uint32_t* ptrCellKeys = (uint32_t*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboParticleIndices);
		uint32_t* ptrParticleIndices = (uint32_t*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboCellX);
		int* ptrCellX = (int*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboCellY);
		int* ptrCellY = (int*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);

		for (size_t i = 0; i < n; i++) { 
			entries.cellKeys[i] = ptrCellKeys[i]; 
			entries.particleIndices[i] = ptrParticleIndices[i];
			entries.cellXs[i] = ptrCellX[i]; 
			entries.cellYs[i] = ptrCellY[i];
		}

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboCellKeys);
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboParticleIndices);
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboCellX);
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboCellY);
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	}

		struct Entry {
		uint32_t cellKey;
		uint32_t particleIndex;
		int32_t cellX;
		int32_t cellY;
	};

	const char* bitonicSortCompute = R"(
#version 430
layout(local_size_x = 256) in;

struct Entry {
    uint cellKey;
    uint particleIndex;
    int cellX;
    int cellY;
};

layout(std430, binding = 16) buffer Entries {
    Entry entries[];
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

    uint valueLeft = entries[indexLeft].cellKey;
    uint valueRight = entries[indexRight].cellKey;

    if (valueLeft > valueRight)
    {
        Entry temp = entries[indexLeft];
        entries[indexLeft] = entries[indexRight];
        entries[indexRight] = temp;
    }
}
)";

	GLuint bitonicSortProgram;

	GLuint ssboEntries;

	std::vector<Entry> entriesBuffer;

	void bitonicSortKernel() {

		glGenBuffers(1, &ssboEntries);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboEntries);
		glBufferData(GL_SHADER_STORAGE_BUFFER, 100000 * sizeof(Entry), nullptr, GL_DYNAMIC_DRAW);

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

		entriesBuffer.resize(n);
		for (size_t i = 0; i < n; i++) {
			entriesBuffer[i].cellKey = entries.cellKeys[i];
			entriesBuffer[i].particleIndex = entries.particleIndices[i];
			entriesBuffer[i].cellX = entries.cellXs[i];
			entriesBuffer[i].cellY = entries.cellYs[i];
		}

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboEntries);
		glBufferData(GL_SHADER_STORAGE_BUFFER, n * sizeof(Entry), entriesBuffer.data(), GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 16, ssboEntries);

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

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboEntries);
		Entry* ptrEntries = (Entry*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);

		for (size_t i = 0; i < n; i++) {
			entries.cellKeys[i] = ptrEntries[i].cellKey;
			entries.particleIndices[i] = ptrEntries[i].particleIndex;
			entries.cellXs[i] = ptrEntries[i].cellX;
			entries.cellYs[i] = ptrEntries[i].cellY;
		}

		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	}

	const char* offsetCompute = R"(
#version 430

layout(local_size_x = 256) in;

struct Entry {
    uint cellKey;
    uint particleIndex;
    int cellX;
    int cellY;
};

layout(std430, binding = 16) buffer Entries {
    Entry entries[];
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

    uint key = entries[i].cellKey; 

    uint keyPrev = (i == 0) ? null : entries[i - 1].cellKey;

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

	void gpuOffsets() {
		const size_t n = entries.cellKeys.size();
		if (n == 0) return;

		startIndices.assign(hashTableSize, UINT32_MAX);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboOffsets);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0,
			startIndices.size() * sizeof(uint32_t), startIndices.data());

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 16, ssboEntries);

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
		else {
			std::cerr << "gpuOffsets: Failed to map offset buffer\n";
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

	void computeViscCohesionForces(std::vector<ParticlePhysics>& pParticles,
		std::vector<ParticleRendering>& rParticles, std::vector<glm::vec2>& sphForce, size_t& N);

	void groundModeBoundary(std::vector<ParticlePhysics>& pParticles,
		std::vector<ParticleRendering>& rParticles, glm::vec2 domainSize);

	void PCISPH(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, float& dt);

	void pcisphSolver(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, float& dt, glm::vec2& domainSize, bool& sphGround, glm::vec2& mouseWorldPos) {

		newGrid(pParticles);

		//newGridGPU(pParticles);

		PCISPH(pParticles, rParticles, dt);

		if (sphGround) {
			groundModeBoundary(pParticles, rParticles, domainSize);
		}
	}
};