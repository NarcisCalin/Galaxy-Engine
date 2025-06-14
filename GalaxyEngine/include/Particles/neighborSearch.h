#pragma once

#include "Particles/particle.h"

struct GridCellNS {
	std::vector<size_t> particleIndices;
};

struct NeighborSearch {

	float densityRadius = 4.5f;

	float cellSize = 3.0f;
	std::unordered_map<size_t, GridCellNS> grid;

	size_t getGridIndex(const glm::vec2& pos) const {
		size_t cellX = static_cast<size_t>(floor(pos.x / cellSize));
		size_t cellY = static_cast<size_t>(floor(pos.y / cellSize));
		return cellX * 10000 + cellY;
	}

	std::vector<size_t> getNeighborCells(size_t cellIndex) const {
		std::vector<size_t> neighbors;
		size_t cellX = cellIndex / 10000;
		size_t cellY = cellIndex % 10000;

		for (int i = -1; i <= 1; i++) {
			for (int j = -1; j <= 1; j++) {
				neighbors.push_back((cellX + i) * 10000 + (cellY + j));
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

					float r = sqrtf(std::max(rSq, 1e-6f));
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

		// Heuristics for performance
		constexpr float minCellSize = 1.0f;
		constexpr float maxCellSize = 4.0f;
		constexpr float totalSizeMultiplier = 1.5f;

		const float sizeFactor = particleSizeMultiplier
			* particleTextureHalfSize
			* totalSizeMultiplier;

		float cellSize = std::numeric_limits<float>::max();

		for (size_t i = 0; i < pParticles.size(); ++i) {
			auto& rP = rParticles[i];
			if (rP.isDarkMatter || rP.uniqueColor || rP.isSolid) {
				continue;
			}

			float candidate = rP.size * sizeFactor;

			cellSize = std::min(cellSize, candidate);
		}

		cellSize = std::clamp(cellSize, minCellSize, maxCellSize);

		float minX = std::numeric_limits<float>::max();
		float minY = std::numeric_limits<float>::max();
		float maxX = std::numeric_limits<float>::lowest();
		float maxY = std::numeric_limits<float>::lowest();

#pragma omp parallel for reduction(min:minX,minY) reduction(max:maxX,maxY) schedule(static)
		for (int64_t idx = 0; idx < particlesToProcess.size(); idx++) {
			const auto& pos = pParticles[particlesToProcess[idx]].pos;
			minX = std::min(minX, pos.x);
			minY = std::min(minY, pos.y);
			maxX = std::max(maxX, pos.x);
			maxY = std::max(maxY, pos.y);
		}

		minX -= cellSize;
		minY -= cellSize;
		maxX += cellSize;
		maxY += cellSize;

		int gridWidth = std::max(1, static_cast<int>((maxX - minX) / cellSize) + 1);
		int gridHeight = std::max(1, static_cast<int>((maxY - minY) / cellSize) + 1);

		std::vector<std::vector<size_t>> grid(gridWidth * gridHeight);

		for (const size_t i : particlesToProcess) {
			const auto& pos = pParticles[i].pos;

			int cellX = static_cast<int>((pos.x - minX) / cellSize);
			int cellY = static_cast<int>((pos.y - minY) / cellSize);

			cellX = std::max(0, std::min(cellX, gridWidth - 1));
			cellY = std::max(0, std::min(cellY, gridHeight - 1));

			int cellIdx = cellY * gridWidth + cellX;

			grid[cellIdx].push_back(i);
		}

#pragma omp parallel for schedule(dynamic)
		for (int64_t idx = 0; idx < particlesToProcess.size(); idx++) {
			const size_t i = particlesToProcess[idx];
			const auto& particle = pParticles[i];

			int cellX = static_cast<int>((particle.pos.x - minX) / cellSize);
			int cellY = static_cast<int>((particle.pos.y - minY) / cellSize);

			cellX = std::max(0, std::min(cellX, gridWidth - 1));
			cellY = std::max(0, std::min(cellY, gridHeight - 1));

			for (int ny = std::max(0, cellY - 1); ny <= std::min(gridHeight - 1, cellY + 1); ny++) {
				for (int nx = std::max(0, cellX - 1); nx <= std::min(gridWidth - 1, cellX + 1); nx++) {
					int cellIdx = ny * gridWidth + nx;

					const auto& cellParticles = grid[cellIdx];

					const size_t* neighborIdxPtr = cellParticles.data();
					const size_t cellSize = cellParticles.size();

					for (size_t n = 0; n < cellSize; n++) {
						const size_t neighborIdx = neighborIdxPtr[n];

						if (neighborIdx == i) continue;

						glm::vec2 d = particle.pos - pParticles[neighborIdx].pos;
						const float distSq = d.x * d.x + d.y * d.y;

						if (distSq < densityRadiusSq) {
#pragma omp atomic
							rParticles[i].neighbors++;
						}
					}
				}
			}
		}
	}
};