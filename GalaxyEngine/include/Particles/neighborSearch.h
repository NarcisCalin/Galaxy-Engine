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

            pi.neighborCount = 0;

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
                                pi.neighborCount++;
                            }
                        }
                    }
                }
            }
        }

        uint32_t currentOffset = 0;
        for (size_t i = 0; i < pParticles.size(); ++i) {
            pParticles[i].neighborOffset = currentOffset;
            currentOffset += pParticles[i].neighborCount;
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
            rParticles[pIdx].neighbors = pi.neighborCount;
        }
    }
};