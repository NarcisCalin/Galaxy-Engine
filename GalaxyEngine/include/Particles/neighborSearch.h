#pragma once

#include "../raylib/raylib.h"
#include "particle.h"
#include "omp.h"
#include <vector>
#include <algorithm>


struct NeighborSearch {

	float densityRadius = 4.5f;

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
		for (size_t idx = 0; idx < particlesToProcess.size(); idx++) {
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
		for (size_t idx = 0; idx < particlesToProcess.size(); idx++) {
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

						const float dx = particle.pos.x - pParticles[neighborIdx].pos.x;
						const float dy = particle.pos.y - pParticles[neighborIdx].pos.y;
						const float distSq = dx * dx + dy * dy;

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