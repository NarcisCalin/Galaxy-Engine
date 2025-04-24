#pragma once

#include "../raylib/raylib.h"
#include "particle.h"
#include "vector"

struct ParticleDeletion {

	bool deleteSelection = false;

	bool deleteNonImportant = false;

	void deleteSelected(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles) {
		if (deleteSelection || IsKeyPressed(KEY_DELETE)) {
			for (size_t i = pParticles.size(); i-- > 0;) {
				if (rParticles[i].isSelected) {

					std::swap(pParticles[i], pParticles.back());
					std::swap(rParticles[i], rParticles.back());

					pParticles.pop_back();
					rParticles.pop_back();
				}
			}
			deleteSelection = false;
		}
	}

	void deleteStrays(std::vector<ParticlePhysics>& pParticles,
		std::vector<ParticleRendering>& rParticles, bool& isCollisionsEnabled) {
		if (deleteNonImportant) {
			if (isCollisionsEnabled) {
				collisionRMultiplier = 6.0f;
			}
			else {
				collisionRMultiplier = 1.0f;
			}
			std::vector<int> neighborCounts(pParticles.size(), 0);

			for (size_t i = 0; i < pParticles.size(); i++) {
				const Vector2& pos1 = pParticles[i].pos;
				for (size_t j = i + 1; j < pParticles.size(); j++) {
					const Vector2& pos2 = pParticles[j].pos;
					if (std::abs(pos2.x - pos1.x) > xBreakThreshold * collisionRMultiplier)
						break;
					float dx = pos1.x - pos2.x;
					float dy = pos1.y - pos2.y;
					if (dx * dx + dy * dy < squaredDistanceThreshold * collisionRMultiplier) {
						neighborCounts[i]++;
						neighborCounts[j]++;
					}
				}
			}

			std::vector<ParticlePhysics> newPParticles;
			std::vector<ParticleRendering> newRParticles;
			newPParticles.reserve(pParticles.size());
			newRParticles.reserve(rParticles.size());
			for (size_t i = 0; i < pParticles.size(); i++) {

				if (!(neighborCounts[i] < 5 && !rParticles[i].isSolid)) {
					newPParticles.push_back(std::move(pParticles[i]));
					newRParticles.push_back(std::move(rParticles[i]));
				}
			}
			pParticles.swap(newPParticles);
			rParticles.swap(newRParticles);

			deleteNonImportant = false;
		}
	}
private:
	const float distanceThreshold = 10.0f;
	const float squaredDistanceThreshold = distanceThreshold * distanceThreshold;
	const float xBreakThreshold = 2.4f;
	float collisionRMultiplier = 1.0f;
};