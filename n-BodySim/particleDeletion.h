#pragma once
#include "raylib.h"
#include "planet.h"
#include "vector"

struct ParticleDeletion {

	bool deleteSelection = false;

	bool deleteNonImportant = false;

	void deleteSelected(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles) {
		if (deleteSelection || IsKeyPressed(KEY_DELETE)) {
			for (int i = static_cast<int>(pParticles.size()) - 1; i >= 0; --i) {
				if (rParticles[i].isSelected) {
					pParticles.erase(pParticles.begin() + i);
					rParticles.erase(rParticles.begin() + i);
				}
			}
			deleteSelection = false;
		}
	}

	void deleteNonImportanParticles(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles) {
		if (deleteNonImportant) {
			const float distanceThreshold = 10.0f;
			const float xBreakThreshold = 2.4f;
			std::vector<int> neighborCounts(pParticles.size(), 0);

			for (size_t i = 0; i < pParticles.size(); i++) {
				const Vector2& pos1 = pParticles[i].pos;
				for (size_t j = i + 1; j < pParticles.size(); j++) {
					const Vector2& pos2 = pParticles[j].pos;

					if (std::abs(pos2.x - pos1.x) > xBreakThreshold) break;

					const float dx = pos1.x - pos2.x;
					const float dy = pos1.y - pos2.y;
					const float distSq = dx * dx + dy * dy;

					if (distSq < distanceThreshold * distanceThreshold) {
						neighborCounts[i]++;
						neighborCounts[j]++;
					}
				}
			}

			for (int i = static_cast<int>(pParticles.size()) - 1; i >= 0; i--) {
				if (neighborCounts[i] < 5 && !rParticles[i].isSolid) {
					pParticles.erase(pParticles.begin() + i);
					rParticles.erase(rParticles.begin() + i);
				}
			}
			deleteNonImportant = false;
		}
	}
};