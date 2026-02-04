#pragma once

#include "IO/io.h"

#include "Particles/particle.h"

struct ParticleDeletion {

	bool deleteSelection = false;

	bool deleteNonImportant = false;

	void deleteSelected(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles,
		std::vector<ParticlePhysics3D>& pParticles3D, std::vector<ParticleRendering3D>& rParticles3D, bool& is3DMode) {

		if (!is3DMode) {
			if (deleteSelection || IO::shortcutPress(KEY_DELETE)) {
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
		else {
			if (deleteSelection || IO::shortcutPress(KEY_DELETE)) {
				for (size_t i = pParticles3D.size(); i-- > 0;) {
					if (rParticles3D[i].isSelected) {

						std::swap(pParticles3D[i], pParticles3D.back());
						std::swap(rParticles3D[i], rParticles3D.back());

						pParticles3D.pop_back();
						rParticles3D.pop_back();
					}
				}
				deleteSelection = false;
			}
		}
	}

	void deleteStrays(std::vector<ParticlePhysics>& pParticles,
		std::vector<ParticleRendering>& rParticles, bool& isSPHEnabled,
		std::vector<ParticlePhysics3D>& pParticles3D, std::vector<ParticleRendering3D>& rParticles3D, bool& is3DMode) {

		if (deleteNonImportant) {
			if (isSPHEnabled) {
				if (!is3DMode) {
					collisionRMultiplier = 6.0f;
				}
				else {
					collisionRMultiplier = 12.0f;
				}
			}
			else {
				if (!is3DMode) {
					collisionRMultiplier = 1.0f;
				}
				else {
					collisionRMultiplier = 2.0f;
				}
			}

			if (!is3DMode) {
				std::vector<int> neighborCounts(pParticles.size(), 0);

				for (size_t i = 0; i < pParticles.size(); i++) {
					const glm::vec2& pos1 = pParticles[i].pos;
					for (size_t j = i + 1; j < pParticles.size(); j++) {
						const glm::vec2& pos2 = pParticles[j].pos;
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
			else {
				std::vector<int> neighborCounts(pParticles3D.size(), 0);

				for (size_t i = 0; i < pParticles3D.size(); i++) {
					const glm::vec3& pos1 = pParticles3D[i].pos;
					for (size_t j = i + 1; j < pParticles3D.size(); j++) {
						const glm::vec3& pos2 = pParticles3D[j].pos;
						if (std::abs(pos2.x - pos1.x) > xBreakThreshold * collisionRMultiplier)
							break;
						glm::vec3 d = pos2 - pos1;

						float dSq = glm::dot(d, d);
						if (dSq < squaredDistanceThreshold * collisionRMultiplier) {
							neighborCounts[i]++;
							neighborCounts[j]++;
						}
					}
				}

				std::vector<ParticlePhysics3D> newPParticles;
				std::vector<ParticleRendering3D> newRParticles;
				newPParticles.reserve(pParticles3D.size());
				newRParticles.reserve(rParticles3D.size());
				for (size_t i = 0; i < pParticles3D.size(); i++) {

					if (!(neighborCounts[i] < 5 && !rParticles3D[i].isSolid)) {
						newPParticles.push_back(std::move(pParticles3D[i]));
						newRParticles.push_back(std::move(rParticles3D[i]));
					}
				}
				pParticles3D.swap(newPParticles);
				rParticles3D.swap(newRParticles);

				deleteNonImportant = false;
			}
		}
	}

private:
	const float distanceThreshold = 10.0f;
	const float squaredDistanceThreshold = distanceThreshold * distanceThreshold;
	const float xBreakThreshold = 2.4f;
	float collisionRMultiplier = 1.0f;
};