#pragma once

#include "Particles/particle.h"

struct NeighborSearch {

	std::vector<uint32_t> globalNeighborList;

	float originalDensityRadius = 3.5f;
	float densityRadius = originalDensityRadius;
	float cellSize = 3.0f;

	std::vector<size_t> cellCounts;
	std::vector<size_t> cellStart;
	std::vector<size_t> cellParticles;
	std::vector<int> cellXList;
	std::vector<int> cellYList;

	std::vector<size_t> idToIndexTable;

	void calculateGridBounds(const std::vector<ParticlePhysics>& pParticles,
		const std::vector<size_t>& activeIndices,
		int& gridWidth, int& gridHeight,
		float& minX, float& minY)
	{

		float maxX = std::numeric_limits<float>::lowest();
		float maxY = std::numeric_limits<float>::lowest();
		minX = std::numeric_limits<float>::max();
		minY = std::numeric_limits<float>::max();

#pragma omp parallel for reduction(min:minX, minY) reduction(max:maxX, maxY)
		for (int64_t i = 0; i < (int64_t)activeIndices.size(); ++i) {
			const auto& pos = pParticles[activeIndices[i]].pos;
			if (pos.x < minX) minX = pos.x;
			if (pos.y < minY) minY = pos.y;
			if (pos.x > maxX) maxX = pos.x;
			if (pos.y > maxY) maxY = pos.y;
		}

		minX -= cellSize; minY -= cellSize;
		maxX += cellSize; maxY += cellSize;

		gridWidth = std::max(1, static_cast<int>((maxX - minX) / cellSize) + 1);
		gridHeight = std::max(1, static_cast<int>((maxY - minY) / cellSize) + 1);
	}

	void UpdateNeighbors(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles) {
		if (pParticles.empty()) return;

		float densityRadiusSq = densityRadius * densityRadius;

		std::vector<size_t> activeIndices;
		activeIndices.reserve(pParticles.size());
		for (size_t i = 0; i < pParticles.size(); i++) {
			if (!rParticles[i].isDarkMatter) {
				activeIndices.push_back(i);
			}
		}

		int gridWidth, gridHeight;
		float minX, minY;
		calculateGridBounds(pParticles, activeIndices, gridWidth, gridHeight, minX, minY);

		int numCells = gridWidth * gridHeight;

		if (cellCounts.size() < numCells) cellCounts.resize(numCells);
		std::fill(cellCounts.begin(), cellCounts.end(), 0);

		if (cellXList.size() < activeIndices.size()) cellXList.resize(activeIndices.size());
		if (cellYList.size() < activeIndices.size()) cellYList.resize(activeIndices.size());

#pragma omp parallel for
		for (int64_t i = 0; i < (int64_t)activeIndices.size(); ++i) {
			size_t pIdx = activeIndices[i];
			const auto& pos = pParticles[pIdx].pos;

			int cx = static_cast<int>((pos.x - minX) / cellSize);
			int cy = static_cast<int>((pos.y - minY) / cellSize);

			cx = std::max(0, std::min(cx, gridWidth - 1));
			cy = std::max(0, std::min(cy, gridHeight - 1));

			cellXList[i] = cx;
			cellYList[i] = cy;

			int cellIdx = cy * gridWidth + cx;
#pragma omp atomic
			cellCounts[cellIdx]++;
		}

		if (cellStart.size() < numCells + 1) cellStart.resize(numCells + 1);
		cellStart[0] = 0;
		for (int i = 0; i < numCells; i++) {
			cellStart[i + 1] = cellStart[i] + cellCounts[i];
		}

		if (cellParticles.size() < activeIndices.size()) cellParticles.resize(activeIndices.size());

		std::vector<size_t> fillCursor = cellStart;

#pragma omp parallel for
		for (int64_t i = 0; i < (int64_t)activeIndices.size(); ++i) {
			int cx = cellXList[i];
			int cy = cellYList[i];
			int cellIdx = cy * gridWidth + cx;

			size_t writePos;
#pragma omp atomic capture
			writePos = fillCursor[cellIdx]++;

			cellParticles[writePos] = activeIndices[i];
		}

#pragma omp parallel for schedule(dynamic)
		for (int64_t i = 0; i < (int64_t)activeIndices.size(); ++i) {
			size_t pIdx = activeIndices[i];
			auto& pi = pParticles[pIdx];

			rParticles[pIdx].neighbors = 0;

			int cx = cellXList[i];
			int cy = cellYList[i];

			for (int ny = -1; ny <= 1; ++ny) {
				for (int nx = -1; nx <= 1; ++nx) {
					int neighborCX = cx + nx;
					int neighborCY = cy + ny;

					if (neighborCX >= 0 && neighborCX < gridWidth &&
						neighborCY >= 0 && neighborCY < gridHeight) {

						int cellIdx = neighborCY * gridWidth + neighborCX;
						size_t start = cellStart[cellIdx];
						size_t end = cellStart[cellIdx + 1];

						for (size_t k = start; k < end; ++k) {
							size_t neighborIdx = cellParticles[k];

							if (neighborIdx == pIdx) continue;

							glm::vec2 d = pi.pos - pParticles[neighborIdx].pos;
							float distSq = d.x * d.x + d.y * d.y;

							if (distSq < densityRadiusSq) {
								rParticles[pIdx].neighbors++;
							}
						}
					}
				}
			}
		}

		uint32_t currentOffset = 0;
		for (size_t i = 0; i < pParticles.size(); ++i) {
			pParticles[i].neighborOffset = currentOffset;
			currentOffset += rParticles[i].neighbors;
		}

		if (globalNeighborList.size() < currentOffset) {
			globalNeighborList.resize(currentOffset);
		}

#pragma omp parallel for schedule(dynamic)
		for (int64_t i = 0; i < (int64_t)activeIndices.size(); ++i) {
			size_t pIdx = activeIndices[i];
			auto& pi = pParticles[pIdx];

			uint32_t currentNeighborIndex = 0;

			int cx = cellXList[i];
			int cy = cellYList[i];

			for (int ny = -1; ny <= 1; ++ny) {
				for (int nx = -1; nx <= 1; ++nx) {
					int neighborCX = cx + nx;
					int neighborCY = cy + ny;

					if (neighborCX >= 0 && neighborCX < gridWidth &&
						neighborCY >= 0 && neighborCY < gridHeight) {

						int cellIdx = neighborCY * gridWidth + neighborCX;
						size_t start = cellStart[cellIdx];
						size_t end = cellStart[cellIdx + 1];

						for (size_t k = start; k < end; ++k) {
							size_t neighborIdx = cellParticles[k];

							if (neighborIdx == pIdx) continue;

							glm::vec2 d = pi.pos - pParticles[neighborIdx].pos;
							float distSq = d.x * d.x + d.y * d.y;

							if (distSq < densityRadiusSq) {

								globalNeighborList[pi.neighborOffset + currentNeighborIndex] = pParticles[neighborIdx].id;
								currentNeighborIndex++;
							}
						}
					}
				}
			}
		}
	}
};

struct NeighborSearch3D {

	std::vector<uint32_t> globalNeighborList3D;

	float originalDensityRadius = 3.5f;
	float densityRadius = originalDensityRadius;
	float cellSize = 3.5f;

	std::vector<size_t> cellCounts;
	std::vector<size_t> cellStart;
	std::vector<size_t> cellParticles;

	std::vector<int> cellXList;
	std::vector<int> cellYList;
	std::vector<int> cellZList;

	std::vector<size_t> idToIndexTable;

	void calculateGridBounds(const std::vector<ParticlePhysics3D>& pParticles,
		const std::vector<size_t>& activeIndices,
		int& gridWidth, int& gridHeight, int& gridDepth,
		float& minX, float& minY, float& minZ)
	{
		float maxX = std::numeric_limits<float>::lowest();
		float maxY = std::numeric_limits<float>::lowest();
		float maxZ = std::numeric_limits<float>::lowest();

		minX = std::numeric_limits<float>::max();
		minY = std::numeric_limits<float>::max();
		minZ = std::numeric_limits<float>::max();

#pragma omp parallel for reduction(min:minX, minY, minZ) reduction(max:maxX, maxY, maxZ)
		for (int64_t i = 0; i < (int64_t)activeIndices.size(); ++i) {
			const auto& pos = pParticles[activeIndices[i]].pos;

			if (pos.x < minX) minX = pos.x;
			if (pos.y < minY) minY = pos.y;
			if (pos.z < minZ) minZ = pos.z;

			if (pos.x > maxX) maxX = pos.x;
			if (pos.y > maxY) maxY = pos.y;
			if (pos.z > maxZ) maxZ = pos.z;
		}

		minX -= cellSize; minY -= cellSize; minZ -= cellSize;
		maxX += cellSize; maxY += cellSize; maxZ += cellSize;

		gridWidth = std::max(1, static_cast<int>((maxX - minX) / cellSize) + 1);
		gridHeight = std::max(1, static_cast<int>((maxY - minY) / cellSize) + 1);
		gridDepth = std::max(1, static_cast<int>((maxZ - minZ) / cellSize) + 1);
	}

	void UpdateNeighbors(std::vector<ParticlePhysics3D>& pParticles, std::vector<ParticleRendering3D>& rParticles) {
		if (pParticles.empty()) return;

		float densityRadiusSq = densityRadius * densityRadius;

		std::vector<size_t> activeIndices;
		activeIndices.reserve(pParticles.size());
		for (size_t i = 0; i < pParticles.size(); i++) {
			if (!rParticles[i].isDarkMatter) {
				activeIndices.push_back(i);
			}
		}

		int gridWidth, gridHeight, gridDepth;
		float minX, minY, minZ;
		calculateGridBounds(pParticles, activeIndices, gridWidth, gridHeight, gridDepth, minX, minY, minZ);

		int numCells = gridWidth * gridHeight * gridDepth;

		if (cellCounts.size() < numCells) cellCounts.resize(numCells);
		std::fill(cellCounts.begin(), cellCounts.end(), 0);

		if (cellXList.size() < activeIndices.size()) cellXList.resize(activeIndices.size());
		if (cellYList.size() < activeIndices.size()) cellYList.resize(activeIndices.size());
		if (cellZList.size() < activeIndices.size()) cellZList.resize(activeIndices.size());

#pragma omp parallel for
		for (int64_t i = 0; i < (int64_t)activeIndices.size(); ++i) {
			size_t pIdx = activeIndices[i];
			const auto& pos = pParticles[pIdx].pos;

			int cx = static_cast<int>((pos.x - minX) / cellSize);
			int cy = static_cast<int>((pos.y - minY) / cellSize);
			int cz = static_cast<int>((pos.z - minZ) / cellSize);

			cx = std::max(0, std::min(cx, gridWidth - 1));
			cy = std::max(0, std::min(cy, gridHeight - 1));
			cz = std::max(0, std::min(cz, gridDepth - 1));

			cellXList[i] = cx;
			cellYList[i] = cy;
			cellZList[i] = cz;

			int cellIdx = cz * (gridWidth * gridHeight) + cy * gridWidth + cx;

#pragma omp atomic
			cellCounts[cellIdx]++;
		}

		if (cellStart.size() < numCells + 1) cellStart.resize(numCells + 1);
		cellStart[0] = 0;
		for (int i = 0; i < numCells; i++) {
			cellStart[i + 1] = cellStart[i] + cellCounts[i];
		}

		if (cellParticles.size() < activeIndices.size()) cellParticles.resize(activeIndices.size());

		std::vector<size_t> fillCursor = cellStart;

#pragma omp parallel for
		for (int64_t i = 0; i < (int64_t)activeIndices.size(); ++i) {
			int cx = cellXList[i];
			int cy = cellYList[i];
			int cz = cellZList[i];

			int cellIdx = cz * (gridWidth * gridHeight) + cy * gridWidth + cx;

			size_t writePos;
#pragma omp atomic capture
			writePos = fillCursor[cellIdx]++;

			cellParticles[writePos] = activeIndices[i];
		}

#pragma omp parallel for schedule(dynamic)
		for (int64_t i = 0; i < (int64_t)activeIndices.size(); ++i) {
			size_t pIdx = activeIndices[i];
			auto& pi = pParticles[pIdx];

			rParticles[pIdx].neighbors = 0;

			int cx = cellXList[i];
			int cy = cellYList[i];
			int cz = cellZList[i];

			for (int nz = -1; nz <= 1; ++nz) {

				for (int ny = -1; ny <= 1; ++ny) {

					for (int nx = -1; nx <= 1; ++nx) {

						int neighborCX = cx + nx;
						int neighborCY = cy + ny;
						int neighborCZ = cz + nz;

						if (neighborCX >= 0 && neighborCX < gridWidth &&
							neighborCY >= 0 && neighborCY < gridHeight &&
							neighborCZ >= 0 && neighborCZ < gridDepth) {

							int cellIdx = neighborCZ * (gridWidth * gridHeight) + neighborCY * gridWidth + neighborCX;

							size_t start = cellStart[cellIdx];
							size_t end = cellStart[cellIdx + 1];

							for (size_t k = start; k < end; ++k) {
								size_t neighborIdx = cellParticles[k];

								if (neighborIdx == pIdx) continue;

								glm::vec3 d = pi.pos - pParticles[neighborIdx].pos;
								float distSq = d.x * d.x + d.y * d.y + d.z * d.z;

								if (distSq < densityRadiusSq) {
									rParticles[pIdx].neighbors++;
								}
							}
						}
					}
				}
			}
		}

		uint32_t currentOffset = 0;
		for (size_t i = 0; i < pParticles.size(); ++i) {
			pParticles[i].neighborOffset = currentOffset;
			currentOffset += rParticles[i].neighbors;
		}

		if (globalNeighborList3D.size() < currentOffset) {
			globalNeighborList3D.resize(currentOffset);
		}

#pragma omp parallel for schedule(dynamic)
		for (int64_t i = 0; i < (int64_t)activeIndices.size(); ++i) {
			size_t pIdx = activeIndices[i];
			auto& pi = pParticles[pIdx];

			uint32_t currentNeighborIndex = 0;

			int cx = cellXList[i];
			int cy = cellYList[i];
			int cz = cellZList[i];

			for (int nz = -1; nz <= 1; ++nz) {
				for (int ny = -1; ny <= 1; ++ny) {
					for (int nx = -1; nx <= 1; ++nx) {

						int neighborCX = cx + nx;
						int neighborCY = cy + ny;
						int neighborCZ = cz + nz;

						if (neighborCX >= 0 && neighborCX < gridWidth &&
							neighborCY >= 0 && neighborCY < gridHeight &&
							neighborCZ >= 0 && neighborCZ < gridDepth) {

							int cellIdx = neighborCZ * (gridWidth * gridHeight) + neighborCY * gridWidth + neighborCX;
							size_t start = cellStart[cellIdx];
							size_t end = cellStart[cellIdx + 1];

							for (size_t k = start; k < end; ++k) {
								size_t neighborIdx = cellParticles[k];

								if (neighborIdx == pIdx) continue;

								glm::vec3 d = pi.pos - pParticles[neighborIdx].pos;
								float distSq = d.x * d.x + d.y * d.y + d.z * d.z;

								if (distSq < densityRadiusSq) {
									globalNeighborList3D[pi.neighborOffset + currentNeighborIndex] = pParticles[neighborIdx].id;
									currentNeighborIndex++;
								}
							}
						}
					}
				}
			}
		}
	}
};

struct NeighborSearchV2 {

	float searchRadius = 3.5f;
	float cellSize = 3.0f;
	float invCellSize = 1.0f / 3.0f;

	struct EntryArrays {
		std::vector<uint32_t> cellKeys;
		std::vector<uint32_t> particleIndices;

		std::vector<float> posX;
		std::vector<float> posY;

		std::vector<int> cellXs;
		std::vector<int> cellYs;

		size_t size;
	};

	const uint32_t hashTableSize = 16384;

	EntryArrays entries;

	std::vector<uint32_t> countBuffer;
	std::vector<uint32_t> offsetBuffer;
	std::vector<uint32_t> startIndices;

	NeighborSearchV2() {
		countBuffer.resize(hashTableSize + 1);
		offsetBuffer.resize(hashTableSize + 1);
		startIndices.resize(hashTableSize);
	}

	glm::ivec2 posToCellCoord(const glm::vec2& pos) const {
		return glm::ivec2((int)(pos.x * invCellSize), (int)(pos.y * invCellSize));
	}

	uint32_t hashCell(int cellX, int cellY) const {
		uint32_t h = ((uint32_t)cellX * 73856093) ^ ((uint32_t)cellY * 19349663);
		return h % hashTableSize;
	}

	void newGrid(const std::vector<ParticlePhysics>& pParticles) {
		const size_t n = pParticles.size();
		if (n == 0) return;

		if (entries.cellKeys.size() < n) {
			entries.cellKeys.resize(n);
			entries.particleIndices.resize(n);
			entries.posX.resize(n);
			entries.posY.resize(n);
			entries.cellXs.resize(n);
			entries.cellYs.resize(n);
		}
		entries.size = n;

		std::fill(countBuffer.begin(), countBuffer.end(), 0);
		std::fill(startIndices.begin(), startIndices.end(), UINT32_MAX);

		for (size_t i = 0; i < n; i++) {
			glm::ivec2 coord = posToCellCoord(pParticles[i].pos);
			uint32_t key = hashCell(coord.x, coord.y);

			entries.cellKeys[i] = key;
			entries.particleIndices[i] = i;
			entries.cellXs[i] = coord.x;
			entries.cellYs[i] = coord.y;
		}

		static std::vector<uint32_t> tempKeys;
		static std::vector<uint32_t> tempIndices;
		static std::vector<int> tempXs, tempYs;

		if (tempKeys.size() < n) {
			tempKeys.resize(n); tempIndices.resize(n);
			tempXs.resize(n); tempYs.resize(n);
		}

		for (size_t i = 0; i < n; i++) {
			glm::ivec2 coord = posToCellCoord(pParticles[i].pos);
			uint32_t key = hashCell(coord.x, coord.y);

			tempKeys[i] = key;
			tempIndices[i] = i;
			tempXs[i] = coord.x;
			tempYs[i] = coord.y;

			countBuffer[key]++;
		}

		uint32_t currentOffset = 0;
		for (size_t i = 0; i < hashTableSize; i++) {
			offsetBuffer[i] = currentOffset;
			startIndices[i] = currentOffset;
			currentOffset += countBuffer[i];

			if (countBuffer[i] == 0) startIndices[i] = UINT32_MAX;
		}

		for (size_t i = 0; i < n; i++) {
			uint32_t key = tempKeys[i];
			uint32_t destIndex = offsetBuffer[key]++;

			entries.cellKeys[destIndex] = key;
			entries.particleIndices[destIndex] = tempIndices[i];
			entries.cellXs[destIndex] = tempXs[i];
			entries.cellYs[destIndex] = tempYs[i];

			const auto& p = pParticles[tempIndices[i]];
			entries.posX[destIndex] = p.pos.x;
			entries.posY[destIndex] = p.pos.y;
		}
	}

	template <typename Func>
	void queryNeighbors(const glm::vec2& pos, Func&& callback) {
		glm::ivec2 cell = posToCellCoord(pos);
		int cellRadius = (int)ceil(searchRadius * invCellSize);
		float r2 = searchRadius * searchRadius;

		for (int dx = -cellRadius; dx <= cellRadius; dx++) {
			for (int dy = -cellRadius; dy <= cellRadius; dy++) {
				int neighborX = cell.x + dx;
				int neighborY = cell.y + dy;
				uint32_t key = hashCell(neighborX, neighborY);

				uint32_t start = startIndices[key];
				if (start == UINT32_MAX) continue;

				for (size_t i = start; i < entries.size; i++) {
					if (entries.cellKeys[i] != key) break;

					if (entries.cellXs[i] != neighborX || entries.cellYs[i] != neighborY) continue;

					float dx = entries.posX[i] - pos.x;
					float dy = entries.posY[i] - pos.y;
					float distSq = dx * dx + dy * dy;

					if (distSq <= r2) {
						callback(entries.particleIndices[i]);
					}
				}
			}
		}
	}

	void neighborAmount(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles) {

#pragma omp parallel for schedule(static)
		for (size_t i = 0; i < pParticles.size(); ++i) {

			if (rParticles[i].isPinned || rParticles[i].isBeingDrawn || rParticles[i].isDarkMatter) continue;

			rParticles[i].neighbors = 0;

			queryNeighbors(pParticles[i].pos, [&](uint32_t neighborIdx) {
				rParticles[i].neighbors++;
				});
		}
	}
};

struct NeighborSearchV2AVX2 {

	float searchRadius = 3.5f;
	float cellSize = 3.0f;
	const uint32_t hashTableSize = 16384;

	struct EntryArrays {
		std::vector<uint32_t> cellKeys;
		std::vector<uint32_t> particleIndices;
		std::vector<int> cellXs;
		std::vector<int> cellYs;
		size_t size = 0;
	};

	EntryArrays entries;

	std::vector<uint32_t> startIndices;
	std::vector<uint32_t> cellCounts;

	NeighborSearchV2AVX2() {
		startIndices.resize(hashTableSize);
		cellCounts.resize(hashTableSize);
	}

	std::pair<int, int> posToCellCoord(const glm::vec2& pos) {
		int cellX = static_cast<int>(std::floor(pos.x / cellSize));
		int cellY = static_cast<int>(std::floor(pos.y / cellSize));
		return { cellX, cellY };
	}

	uint32_t hashCell(int cellX, int cellY) {
		uint32_t h = (uint32_t)((cellX * 73856093) ^ (cellY * 19349663));
		return h % hashTableSize;
	}

	void newGridAVX2(const std::vector<ParticlePhysics>& pParticles) {
		const size_t n = pParticles.size();
		if (n == 0) return;

		if (entries.cellKeys.size() < n) {
			entries.cellKeys.resize(n);
			entries.particleIndices.resize(n);
			entries.cellXs.resize(n);
			entries.cellYs.resize(n);
		}
		entries.size = n;

		std::fill(std::execution::unseq, startIndices.begin(), startIndices.end(), UINT32_MAX);
		std::fill(std::execution::unseq, cellCounts.begin(), cellCounts.end(), 0);

#pragma omp parallel for
		for (long long i = 0; i < n; i++) {
			auto [cx, cy] = posToCellCoord(pParticles[i].pos);
			entries.cellKeys[i] = hashCell(cx, cy);
			entries.particleIndices[i] = (uint32_t)i;
			entries.cellXs[i] = cx;
			entries.cellYs[i] = cy;
		}

		std::vector<uint32_t> sortIndices(n);
		std::iota(sortIndices.begin(), sortIndices.end(), 0);

		std::sort(std::execution::par_unseq, sortIndices.begin(), sortIndices.end(),
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

#pragma omp parallel for
		for (long long i = 0; i < (n & ~7); i += 8) {
			__m256i idxVec = _mm256_loadu_si256((__m256i*) & sortIndices[i]);

			__m256i keys = _mm256_i32gather_epi32((const int*)entries.cellKeys.data(), idxVec, 4);
			__m256i pIds = _mm256_i32gather_epi32((const int*)entries.particleIndices.data(), idxVec, 4);
			__m256i xs = _mm256_i32gather_epi32((const int*)entries.cellXs.data(), idxVec, 4);
			__m256i ys = _mm256_i32gather_epi32((const int*)entries.cellYs.data(), idxVec, 4);

			_mm256_storeu_si256((__m256i*) & sorted.cellKeys[i], keys);
			_mm256_storeu_si256((__m256i*) & sorted.particleIndices[i], pIds);
			_mm256_storeu_si256((__m256i*) & sorted.cellXs[i], xs);
			_mm256_storeu_si256((__m256i*) & sorted.cellYs[i], ys);
		}

		for (size_t i = (n & ~7); i < n; i++) {
			uint32_t idx = sortIndices[i];
			sorted.cellKeys[i] = entries.cellKeys[idx];
			sorted.particleIndices[i] = entries.particleIndices[idx];
			sorted.cellXs[i] = entries.cellXs[idx];
			sorted.cellYs[i] = entries.cellYs[idx];
		}

		entries = std::move(sorted);

		if (n > 0) {
			uint32_t currentKey = entries.cellKeys[0];
			startIndices[currentKey] = 0;
			uint32_t count = 1;

			for (size_t i = 1; i < n; i++) {
				uint32_t key = entries.cellKeys[i];
				if (key != currentKey) {
					cellCounts[currentKey] = count;
					startIndices[key] = (uint32_t)i;
					currentKey = key;
					count = 1;
				}
				else {
					count++;
				}
			}
			cellCounts[currentKey] = count;
		}
	}

	void queryNeighborsAVX2(const glm::vec2& pos, std::vector<size_t>& neighbors) {
		neighbors.clear();
		auto [cellX, cellY] = posToCellCoord(pos);

		__m256i targetX = _mm256_set1_epi32(cellX);
		__m256i targetY = _mm256_set1_epi32(cellY);

		for (int dx = -1; dx <= 1; dx++) {
			for (int dy = -1; dy <= 1; dy++) {
				int neighborX = cellX + dx;
				int neighborY = cellY + dy;

				uint32_t cellKey = hashCell(neighborX, neighborY);
				uint32_t start = startIndices[cellKey];

				if (start == UINT32_MAX) continue;

				uint32_t count = cellCounts[cellKey];

				__m256i currentTargetX = _mm256_set1_epi32(neighborX);
				__m256i currentTargetY = _mm256_set1_epi32(neighborY);

				size_t i = 0;
				size_t currentIdx = start;

				for (; i + 8 <= count; i += 8, currentIdx += 8) {

					__m256i cellXs = _mm256_loadu_si256((__m256i*) & entries.cellXs[currentIdx]);
					__m256i cellYs = _mm256_loadu_si256((__m256i*) & entries.cellYs[currentIdx]);

					__m256i matchX = _mm256_cmpeq_epi32(cellXs, currentTargetX);
					__m256i matchY = _mm256_cmpeq_epi32(cellYs, currentTargetY);
					__m256i match = _mm256_and_si256(matchX, matchY);

					int mask = _mm256_movemask_ps(_mm256_castsi256_ps(match));

					if (mask != 0) {
						if (mask & 1) neighbors.push_back(entries.particleIndices[currentIdx + 0]);
						if (mask & 2) neighbors.push_back(entries.particleIndices[currentIdx + 1]);
						if (mask & 4) neighbors.push_back(entries.particleIndices[currentIdx + 2]);
						if (mask & 8) neighbors.push_back(entries.particleIndices[currentIdx + 3]);
						if (mask & 16) neighbors.push_back(entries.particleIndices[currentIdx + 4]);
						if (mask & 32) neighbors.push_back(entries.particleIndices[currentIdx + 5]);
						if (mask & 64) neighbors.push_back(entries.particleIndices[currentIdx + 6]);
						if (mask & 128) neighbors.push_back(entries.particleIndices[currentIdx + 7]);
					}
				}

				for (; i < count; i++, currentIdx++) {
					if (entries.cellXs[currentIdx] == neighborX && entries.cellYs[currentIdx] == neighborY) {
						neighbors.push_back(entries.particleIndices[currentIdx]);
					}
				}
			}
		}
	}

	void neighborAmount(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles) {

#pragma omp parallel 
		{
			std::vector<size_t> neighborIndices;
			neighborIndices.reserve(128);

#pragma omp for
			for (long long i = 0; i < pParticles.size(); ++i) {

				if (rParticles[i].isPinned || rParticles[i].isBeingDrawn || rParticles[i].isDarkMatter) continue;

				queryNeighborsAVX2(pParticles[i].pos, neighborIndices);

				rParticles[i].neighbors = (int)neighborIndices.size();
			}
		}
	}
};

struct NeighborSearch3DV2 {

	float searchRadius = 3.5f;
	float cellSize = 3.0f;
	float invCellSize = 1.0f / 3.0f;

	struct EntryArrays {
		std::vector<uint32_t> cellKeys;
		std::vector<uint32_t> particleIndices;

		std::vector<float> posX;
		std::vector<float> posY;
		std::vector<float> posZ;

		std::vector<int> cellXs;
		std::vector<int> cellYs;
		std::vector<int> cellZs;

		size_t size = 0;
	};

	const uint32_t hashTableSize = 32768;
	EntryArrays entries;

	std::vector<uint32_t> countBuffer;
	std::vector<uint32_t> offsetBuffer;

	std::vector<uint32_t> tempKeys;
	std::vector<uint32_t> tempIndices;
	std::vector<int> tempXs;
	std::vector<int> tempYs;
	std::vector<int> tempZs;

	NeighborSearch3DV2() {
		countBuffer.resize(hashTableSize + 1);
		offsetBuffer.resize(hashTableSize + 1);
	}

	glm::ivec3 posToCellCoord(const glm::vec3& pos) const {
		return glm::ivec3(
			(int)std::floor(pos.x * invCellSize),
			(int)std::floor(pos.y * invCellSize),
			(int)std::floor(pos.z * invCellSize)
		);
	}

	uint32_t hashCell(int cellX, int cellY, int cellZ) const {

		uint32_t h = ((uint32_t)cellX * 73856093)
			^ ((uint32_t)cellY * 19349663)
			^ ((uint32_t)cellZ * 83492791);

		return h % hashTableSize;
	}

	void newGrid(const std::vector<ParticlePhysics3D>& pParticles) {
		const size_t n = pParticles.size();
		if (n == 0) return;

		if (entries.cellKeys.size() < n) {
			entries.cellKeys.resize(n);
			entries.particleIndices.resize(n);
			entries.posX.resize(n);
			entries.posY.resize(n);
			entries.posZ.resize(n);
			entries.cellXs.resize(n);
			entries.cellYs.resize(n);
			entries.cellZs.resize(n);

			tempKeys.resize(n);
			tempIndices.resize(n);
			tempXs.resize(n);
			tempYs.resize(n);
			tempZs.resize(n);
		}
		entries.size = n;

		std::fill(countBuffer.begin(), countBuffer.end(), 0);

		for (size_t i = 0; i < n; i++) {
			glm::ivec3 coord = posToCellCoord(pParticles[i].pos);
			uint32_t key = hashCell(coord.x, coord.y, coord.z);

			tempKeys[i] = key;
			tempIndices[i] = (uint32_t)i;
			tempXs[i] = coord.x;
			tempYs[i] = coord.y;
			tempZs[i] = coord.z;

			countBuffer[key]++;
		}

		uint32_t currentOffset = 0;
		for (size_t i = 0; i < hashTableSize; i++) {
			offsetBuffer[i] = currentOffset;
			currentOffset += countBuffer[i];
		}

		for (size_t i = 0; i < n; i++) {
			uint32_t key = tempKeys[i];

			uint32_t destIndex = offsetBuffer[key]++;

			entries.cellKeys[destIndex] = key;
			entries.particleIndices[destIndex] = tempIndices[i];
			entries.cellXs[destIndex] = tempXs[i];
			entries.cellYs[destIndex] = tempYs[i];
			entries.cellZs[destIndex] = tempZs[i];

			const auto& p = pParticles[tempIndices[i]];
			entries.posX[destIndex] = p.pos.x;
			entries.posY[destIndex] = p.pos.y;
			entries.posZ[destIndex] = p.pos.z;
		}
	}

	template <typename Func>
	void queryNeighbors(const glm::vec3& pos, Func&& callback) {
		glm::ivec3 cell = posToCellCoord(pos);

		int searchGridDist = (int)std::ceil(searchRadius * invCellSize);
		float r2 = searchRadius * searchRadius;

		for (int dz = -searchGridDist; dz <= searchGridDist; dz++) {
			for (int dy = -searchGridDist; dy <= searchGridDist; dy++) {
				for (int dx = -searchGridDist; dx <= searchGridDist; dx++) {

					int neighborX = cell.x + dx;
					int neighborY = cell.y + dy;
					int neighborZ = cell.z + dz;

					uint32_t key = hashCell(neighborX, neighborY, neighborZ);

					uint32_t end = offsetBuffer[key];
					uint32_t count = countBuffer[key];
					uint32_t start = end - count;

					if (count == 0) continue;

					for (size_t i = start; i < end; i++) {
						if (entries.cellXs[i] != neighborX ||
							entries.cellYs[i] != neighborY ||
							entries.cellZs[i] != neighborZ) {
							continue;
						}

						float dX = entries.posX[i] - pos.x;
						float dY = entries.posY[i] - pos.y;
						float dZ = entries.posZ[i] - pos.z;

						float distSq = dX * dX + dY * dY + dZ * dZ;

						if (distSq <= r2) {
							callback(entries.particleIndices[i]);
						}
					}
				}
			}
		}
	}

	void neighborAmount(std::vector<ParticlePhysics3D>& pParticles, std::vector<ParticleRendering3D>& rParticles) {

#pragma omp parallel for schedule(static)
		for (long long i = 0; i < pParticles.size(); ++i) {

			if (rParticles[i].isPinned || rParticles[i].isBeingDrawn || rParticles[i].isDarkMatter) continue;

			rParticles[i].neighbors = 0;

			queryNeighbors(pParticles[i].pos, [&](uint32_t neighborIdx) {
				 if (neighborIdx == i) return; 

				rParticles[i].neighbors++;
				});
		}
	}
};

struct NeighborSearch3DV2AVX2 {

	float searchRadius = 3.5f;
	float cellSize = 3.0f;
	const uint32_t hashTableSize = 32768;

	struct EntryArrays {
		std::vector<uint32_t> cellKeys;
		std::vector<uint32_t> particleIndices;

		std::vector<int> cellXs;
		std::vector<int> cellYs;
		std::vector<int> cellZs;

		size_t size = 0;
	};

	EntryArrays entries;

	std::vector<uint32_t> startIndices;
	std::vector<uint32_t> cellCounts;

	NeighborSearch3DV2AVX2() {
		startIndices.resize(hashTableSize);
		cellCounts.resize(hashTableSize);
	}

	std::tuple<int, int, int> posToCellCoord(const glm::vec3& pos) {
		int cellX = static_cast<int>(std::floor(pos.x / cellSize));
		int cellY = static_cast<int>(std::floor(pos.y / cellSize));
		int cellZ = static_cast<int>(std::floor(pos.z / cellSize));
		return { cellX, cellY, cellZ };
	}

	uint32_t hashCell(int cellX, int cellY, int cellZ) {
		uint32_t h = (uint32_t)((cellX * 73856093) ^ (cellY * 19349663) ^ (cellZ * 83492791));
		return h % hashTableSize;
	}

	void newGridAVX2(const std::vector<ParticlePhysics3D>& pParticles) {
		const size_t n = pParticles.size();
		if (n == 0) return;

		if (entries.cellKeys.size() < n) {
			entries.cellKeys.resize(n);
			entries.particleIndices.resize(n);
			entries.cellXs.resize(n);
			entries.cellYs.resize(n);
			entries.cellZs.resize(n);
		}
		entries.size = n;

		std::fill(std::execution::unseq, startIndices.begin(), startIndices.end(), UINT32_MAX);
		std::fill(std::execution::unseq, cellCounts.begin(), cellCounts.end(), 0);

#pragma omp parallel for
		for (long long i = 0; i < n; i++) {
			auto [cx, cy, cz] = posToCellCoord(pParticles[i].pos);
			entries.cellKeys[i] = hashCell(cx, cy, cz);
			entries.particleIndices[i] = (uint32_t)i;
			entries.cellXs[i] = cx;
			entries.cellYs[i] = cy;
			entries.cellZs[i] = cz;
		}

		std::vector<uint32_t> sortIndices(n);
		std::iota(sortIndices.begin(), sortIndices.end(), 0);

		std::sort(std::execution::par_unseq, sortIndices.begin(), sortIndices.end(),
			[&](uint32_t a, uint32_t b) {
				return entries.cellKeys[a] < entries.cellKeys[b];
			}
		);

		EntryArrays sorted;
		sorted.cellKeys.resize(n);
		sorted.particleIndices.resize(n);
		sorted.cellXs.resize(n);
		sorted.cellYs.resize(n);
		sorted.cellZs.resize(n);
		sorted.size = n;

#pragma omp parallel for
		for (long long i = 0; i < (n & ~7); i += 8) {
			__m256i idxVec = _mm256_loadu_si256((__m256i*) & sortIndices[i]);

			__m256i keys = _mm256_i32gather_epi32((const int*)entries.cellKeys.data(), idxVec, 4);
			__m256i pIds = _mm256_i32gather_epi32((const int*)entries.particleIndices.data(), idxVec, 4);
			__m256i xs = _mm256_i32gather_epi32((const int*)entries.cellXs.data(), idxVec, 4);
			__m256i ys = _mm256_i32gather_epi32((const int*)entries.cellYs.data(), idxVec, 4);
			__m256i zs = _mm256_i32gather_epi32((const int*)entries.cellZs.data(), idxVec, 4);

			_mm256_storeu_si256((__m256i*) & sorted.cellKeys[i], keys);
			_mm256_storeu_si256((__m256i*) & sorted.particleIndices[i], pIds);
			_mm256_storeu_si256((__m256i*) & sorted.cellXs[i], xs);
			_mm256_storeu_si256((__m256i*) & sorted.cellYs[i], ys);
			_mm256_storeu_si256((__m256i*) & sorted.cellZs[i], zs);
		}

		for (size_t i = (n & ~7); i < n; i++) {
			uint32_t idx = sortIndices[i];
			sorted.cellKeys[i] = entries.cellKeys[idx];
			sorted.particleIndices[i] = entries.particleIndices[idx];
			sorted.cellXs[i] = entries.cellXs[idx];
			sorted.cellYs[i] = entries.cellYs[idx];
			sorted.cellZs[i] = entries.cellZs[idx];
		}

		entries = std::move(sorted);

		if (n > 0) {
			uint32_t currentKey = entries.cellKeys[0];
			startIndices[currentKey] = 0;
			uint32_t count = 1;

			for (size_t i = 1; i < n; i++) {
				uint32_t key = entries.cellKeys[i];
				if (key != currentKey) {
					cellCounts[currentKey] = count;
					startIndices[key] = (uint32_t)i;
					currentKey = key;
					count = 1;
				}
				else {
					count++;
				}
			}
			cellCounts[currentKey] = count;
		}
	}

	void queryNeighborsAVX2(const glm::vec3& pos, std::vector<size_t>& neighbors) {
		neighbors.clear();
		auto [cellX, cellY, cellZ] = posToCellCoord(pos);

		for (int dz = -1; dz <= 1; dz++) {
			for (int dy = -1; dy <= 1; dy++) {
				for (int dx = -1; dx <= 1; dx++) {

					int neighborX = cellX + dx;
					int neighborY = cellY + dy;
					int neighborZ = cellZ + dz;

					uint32_t cellKey = hashCell(neighborX, neighborY, neighborZ);
					uint32_t start = startIndices[cellKey];

					if (start == UINT32_MAX) continue;

					uint32_t count = cellCounts[cellKey];

					__m256i currentTargetX = _mm256_set1_epi32(neighborX);
					__m256i currentTargetY = _mm256_set1_epi32(neighborY);
					__m256i currentTargetZ = _mm256_set1_epi32(neighborZ);

					size_t i = 0;
					size_t currentIdx = start;

					for (; i + 8 <= count; i += 8, currentIdx += 8) {

						__m256i cellXs = _mm256_loadu_si256((__m256i*) & entries.cellXs[currentIdx]);
						__m256i cellYs = _mm256_loadu_si256((__m256i*) & entries.cellYs[currentIdx]);
						__m256i cellZs = _mm256_loadu_si256((__m256i*) & entries.cellZs[currentIdx]);

						__m256i matchX = _mm256_cmpeq_epi32(cellXs, currentTargetX);
						__m256i matchY = _mm256_cmpeq_epi32(cellYs, currentTargetY);
						__m256i matchZ = _mm256_cmpeq_epi32(cellZs, currentTargetZ);

						__m256i match = _mm256_and_si256(matchX, matchY);
						match = _mm256_and_si256(match, matchZ);

						int mask = _mm256_movemask_ps(_mm256_castsi256_ps(match));

						if (mask != 0) {
							if (mask & 1) neighbors.push_back(entries.particleIndices[currentIdx + 0]);
							if (mask & 2) neighbors.push_back(entries.particleIndices[currentIdx + 1]);
							if (mask & 4) neighbors.push_back(entries.particleIndices[currentIdx + 2]);
							if (mask & 8) neighbors.push_back(entries.particleIndices[currentIdx + 3]);
							if (mask & 16) neighbors.push_back(entries.particleIndices[currentIdx + 4]);
							if (mask & 32) neighbors.push_back(entries.particleIndices[currentIdx + 5]);
							if (mask & 64) neighbors.push_back(entries.particleIndices[currentIdx + 6]);
							if (mask & 128) neighbors.push_back(entries.particleIndices[currentIdx + 7]);
						}
					}

					for (; i < count; i++, currentIdx++) {
						if (entries.cellXs[currentIdx] == neighborX &&
							entries.cellYs[currentIdx] == neighborY &&
							entries.cellZs[currentIdx] == neighborZ) {

							neighbors.push_back(entries.particleIndices[currentIdx]);
						}
					}
				}
			}
		}
	}

	void neighborAmount(std::vector<ParticlePhysics3D>& pParticles, std::vector<ParticleRendering3D>& rParticles) {

#pragma omp parallel 
		{
			std::vector<size_t> neighborIndices;
			neighborIndices.reserve(128);

#pragma omp for
			for (long long i = 0; i < pParticles.size(); ++i) {

				if (rParticles[i].isPinned || rParticles[i].isBeingDrawn || rParticles[i].isDarkMatter) continue;

				queryNeighborsAVX2(pParticles[i].pos, neighborIndices);

				rParticles[i].neighbors = (int)neighborIndices.size();
			}
		}
	}
};