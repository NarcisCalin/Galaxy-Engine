#include "Physics/collisionGrid.h"

#include "parameters.h"

void CollisionGrid::buildGrid(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles,
	Physics& physics, UpdateVariables& myVar, glm::vec2& gridSize, float& dt) {

	for (size_t i = 0; i < pParticles.size(); i++) {
        // I multiply by 4 for performance. 2 is the diameter of the particle
		float thisSize = rParticles[i].totalRadius * 4.0f;

		if (thisSize > cellSize) {
			cellSize = thisSize;
		}
	}

	int cellAmountX = static_cast<int>(gridSize.x / cellSize);
	int cellAmountY = static_cast<int>(gridSize.y / cellSize);

	int totalCells = cellAmountX * cellAmountY;
	std::vector<std::vector<size_t>> cellList(totalCells);

	for (size_t i = 0; i < pParticles.size(); ++i) {

		int xIdx = static_cast<int>((pParticles[i].pos.x - 0) / cellSize);
		int yIdx = static_cast<int>((pParticles[i].pos.y - 0) / cellSize);

		if (xIdx >= 0 && xIdx < cellAmountX &&
			yIdx >= 0 && yIdx < cellAmountY) {

			int cellId = xIdx + yIdx * cellAmountX;
			cellList[cellId].push_back(i);
		}
	}

    //std::vector<std::mutex> particleLocks(pParticles.size());


    auto checkCollision = [&](size_t a, size_t b) {
       
        if (a == b) return;

        float rA = rParticles[a].size
            * myVar.particleSizeMultiplier
            * myVar.particleTextureHalfSize;
        float rB = rParticles[b].size
            * myVar.particleSizeMultiplier
            * myVar.particleTextureHalfSize;

        glm::vec2 delta = pParticles[a].pos - pParticles[b].pos;
        float distSq = delta.x * delta.x + delta.y * delta.y;
        float sumR = rA + rB;

        if (distSq < sumR * sumR) {
     
            if (a < b) {
               // std::scoped_lock lock(particleLocks[a], particleLocks[b]);
                physics.collisions(pParticles[a], pParticles[b],
                    rParticles[a], rParticles[b],
                    myVar,
                    dt);
            }
            else {
                //std::scoped_lock lock(particleLocks[b], particleLocks[a]);
                physics.collisions(pParticles[a], pParticles[b],
                    rParticles[a], rParticles[b],
                    myVar,
                    dt);
            }
        }
        };


#pragma omp parallel for collapse(2) schedule(dynamic)
    for (int x = 0; x < cellAmountX; ++x) {
        for (int y = 0; y < cellAmountY; ++y) {
            int baseId = x + y * cellAmountX;
            auto& cell = cellList[baseId];

            for (int dx = -1; dx <= 1; ++dx) {
                for (int dy = -1; dy <= 1; ++dy) {
                    int nx = x + dx, ny = y + dy;
                    if (nx < 0 || ny < 0 || nx >= cellAmountX || ny >= cellAmountY)
                        continue;

                    int neighborId = nx + ny * cellAmountX;
                    auto& other = cellList[neighborId];

                    if (neighborId == baseId) {

                        for (size_t i = 0; i < cell.size(); ++i) {
                            for (size_t j = i + 1; j < cell.size(); ++j) {
                                checkCollision(cell[i], cell[j]);
                            }
                        }
                    }
                    else {

                        for (size_t i : cell) {
                            for (size_t j : other) {
                                checkCollision(i, j);
                            }
                        }
                    }
                }
            }
        }
    }
}
