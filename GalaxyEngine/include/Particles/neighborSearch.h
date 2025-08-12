#pragma once

#include "Particles/particle.h"

struct GridCellNS {
	std::vector<size_t> particleIndices;
};

struct NeighborSearch {

	float densityRadius = 4.5f; // Heuristic

	float cellSize = 3.0f; // Heuristic
	std::unordered_map<size_t, GridCellNS> grid;

	int cellAmount = 3840;

	size_t getGridIndex(const glm::vec2& pos) const {
		size_t cellX = static_cast<size_t>(floor(pos.x / cellSize));
		size_t cellY = static_cast<size_t>(floor(pos.y / cellSize));
		return cellX * cellAmount + cellY;
	}

	std::vector<size_t> getNeighborCells(size_t cellIndex) const {
		std::vector<size_t> neighbors;
		size_t cellX = cellIndex / cellAmount;
		size_t cellY = cellIndex % cellAmount;

		for (int i = -1; i <= 1; i++) {
			for (int j = -1; j <= 1; j++) {
				neighbors.push_back((cellX + i) * cellAmount + (cellY + j));
			}
		}

		return neighbors;
	}

	void updateGrid(const std::vector<ParticlePhysics>& pParticles, const std::vector<ParticleRendering>& rParticles) {
		grid.clear();

		for (size_t i = 0; i < pParticles.size(); i++) {

			if (rParticles[i].isDarkMatter) { continue; }

			size_t cellIndex = getGridIndex(pParticles[i].pos);
			grid[cellIndex].particleIndices.push_back(i);
		}
	}

	static std::unordered_map<uint32_t, size_t> idToIndex;
	static void idToI(const std::vector<ParticlePhysics>& pParticles) {
		idToIndex.clear();
		for (size_t i = 0; i < pParticles.size(); i++) {
			idToIndex[pParticles[i].id] = i;
		}
	}

	void neighborSearchHash(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles) {

		float h2 = cellSize * cellSize;

		updateGrid(pParticles, rParticles);
		#pragma omp parallel for
		for (size_t i = 0; i < pParticles.size(); ++i) {

			if (rParticles[i].isDarkMatter) continue;

			auto& pi = pParticles[i];

			size_t cellIndex = getGridIndex(pi.pos);
			auto neighborCells = getNeighborCells(cellIndex);

			for (auto neighCellIdx : neighborCells) {
				auto it = grid.find(neighCellIdx);
				if (it == grid.end()) continue;
				auto& cell = it->second;

				for (auto pjIdx : cell.particleIndices) {

					if (pjIdx == i) continue;
					if (rParticles[pjIdx].isDarkMatter) continue;

					auto& pj = pParticles[pjIdx];

					glm::vec2 d = { pj.pos.x - pi.pos.x, pj.pos.y - pi.pos.y };
					float   rSq = d.x * d.x + d.y * d.y;
					if (rSq >= h2) continue;

					pi.neighborIds.push_back(pj.id);
				}
			}
		}
	}

	void neighborSearch(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles,
		float& particleSizeMultiplier, float& particleTextureHalfSize) {

		float densityRadiusSq = densityRadius * densityRadius;

		if (pParticles.empty()) {
			return;
		}

		std::vector<size_t> particlesToProcess;
		particlesToProcess.reserve(pParticles.size());
		for (size_t i = 0; i < pParticles.size(); i++) {
			if (!rParticles[i].isDarkMatter && !rParticles[i].uniqueColor) {
				particlesToProcess.push_back(i);

				rParticles[i].neighbors = 0;
			}
		}

		float cellSize = 2.5f; // Heuristic

		float minX = std::numeric_limits<float>::max();
		float minY = std::numeric_limits<float>::max();
		float maxX = std::numeric_limits<float>::lowest();
		float maxY = std::numeric_limits<float>::lowest();

		std::vector<int> cellXList(particlesToProcess.size());
		std::vector<int> cellYList(particlesToProcess.size());

#pragma omp parallel
		{
			float localMinX = std::numeric_limits<float>::max();
			float localMinY = std::numeric_limits<float>::max();
			float localMaxX = std::numeric_limits<float>::lowest();
			float localMaxY = std::numeric_limits<float>::lowest();

#pragma omp for nowait
			for (int64_t idx = 0; idx < (int64_t)particlesToProcess.size(); idx++) {
				const auto& pos = pParticles[particlesToProcess[idx]].pos;
				localMinX = std::min(localMinX, pos.x);
				localMinY = std::min(localMinY, pos.y);
				localMaxX = std::max(localMaxX, pos.x);
				localMaxY = std::max(localMaxY, pos.y);
			}

#pragma omp critical
			{
				minX = std::min(minX, localMinX);
				minY = std::min(minY, localMinY);
				maxX = std::max(maxX, localMaxX);
				maxY = std::max(maxY, localMaxY);
			}
		}

		minX -= cellSize;
		minY -= cellSize;
		maxX += cellSize;
		maxY += cellSize;

		int gridWidth = std::max(1, static_cast<int>((maxX - minX) / cellSize) + 1);
		int gridHeight = std::max(1, static_cast<int>((maxY - minY) / cellSize) + 1);
		int numCells = gridWidth * gridHeight;

		std::vector<size_t> cellCounts(numCells, 0);

#pragma omp parallel for
		for (int64_t idx = 0; idx < (int64_t)particlesToProcess.size(); idx++) {
			const auto& pos = pParticles[particlesToProcess[idx]].pos;

			int cx = static_cast<int>((pos.x - minX) / cellSize);
			int cy = static_cast<int>((pos.y - minY) / cellSize);
			cx = std::max(0, std::min(cx, gridWidth - 1));
			cy = std::max(0, std::min(cy, gridHeight - 1));

			int cellIdx = cy * gridWidth + cx;
			cellXList[idx] = cx;
			cellYList[idx] = cy;

#pragma omp atomic
			cellCounts[cellIdx]++;
		}

		std::vector<size_t> cellStart(numCells + 1, 0);
		for (int i = 0; i < numCells; i++) {
			cellStart[i + 1] = cellStart[i] + cellCounts[i];
		}

		std::vector<size_t> cellParticles(particlesToProcess.size());
		std::vector<size_t> fillCursor = cellStart;

#pragma omp parallel for
		for (int64_t idx = 0; idx < (int64_t)particlesToProcess.size(); idx++) {
			int cx = cellXList[idx];
			int cy = cellYList[idx];
			int cellIdx = cy * gridWidth + cx;

			size_t writePos;
#pragma omp atomic capture
			writePos = fillCursor[cellIdx]++;

			cellParticles[writePos] = particlesToProcess[idx];
		}

		std::vector<int> localNeighborCounts(particlesToProcess.size(), 0);

#pragma omp parallel for schedule(dynamic)
		for (int64_t idx = 0; idx < (int64_t)particlesToProcess.size(); idx++) {
			size_t i = particlesToProcess[idx];
			const auto& particle = pParticles[i];

			int cx = cellXList[idx];
			int cy = cellYList[idx];

			for (int ny = std::max(0, cy - 1); ny <= std::min(gridHeight - 1, cy + 1); ny++) {
				for (int nx = std::max(0, cx - 1); nx <= std::min(gridWidth - 1, cx + 1); nx++) {
					int neighborCellIdx = nx + ny * gridWidth;

					size_t start = cellStart[neighborCellIdx];
					size_t end = cellStart[neighborCellIdx + 1];

					for (size_t p = start; p < end; p++) {
						size_t neighborIdx = cellParticles[p];
						if (neighborIdx == i) continue;

						glm::vec2 d = particle.pos - pParticles[neighborIdx].pos;
						float distSq = d.x * d.x + d.y * d.y;

						if (distSq < densityRadiusSq) {
							localNeighborCounts[idx]++;
						}
					}
				}
			}
		}

		for (size_t k = 0; k < particlesToProcess.size(); k++) {
			rParticles[particlesToProcess[k]].neighbors = localNeighborCounts[k];
		}
	}
};