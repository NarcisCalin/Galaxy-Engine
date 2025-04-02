#include "particleTrails.h"

ParticleTrails::ParticleTrails() {
}

void ParticleTrails::trailLogic(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles,
	std::vector<ParticlePhysics>& pParticlesSelected, std::vector<ParticleRendering>& rParticlesSelected,
	bool& isGlobalTrailsEnabled, bool& isSelectedTrailsEnabled, int& maxLength, float& timeFactor, bool& isLocalTrailsEnabled) {

	const size_t NUM_PLANETS = pParticles.size();

	if (IsKeyPressed(KEY_T) && !IsKeyDown(KEY_LEFT_CONTROL)) {
		isGlobalTrailsEnabled = !isGlobalTrailsEnabled;
		isSelectedTrailsEnabled = false;
		trailDots.clear();
	}
	if (IsKeyPressed(KEY_T) && IsKeyDown(KEY_LEFT_CONTROL)) {
		isSelectedTrailsEnabled = !isSelectedTrailsEnabled;
		isGlobalTrailsEnabled = false;
		trailDots.clear();
	}


	if (timeFactor > 0) {
		if (isGlobalTrailsEnabled) {
			for (size_t i = 0; i < pParticles.size(); i++) {

				Vector2 offset = {
			pParticles[i].pos.x - selectedParticlesAveragePos.x,
			pParticles[i].pos.y - selectedParticlesAveragePos.y
				};

				trailDots.push_back({ { pParticles[i].pos }, {offset}, rParticles[i].color });
			}

			size_t MAX_DOTS = maxLength * pParticles.size();
			if (trailDots.size() > MAX_DOTS) {
				size_t excess = trailDots.size() - MAX_DOTS;
				trailDots.erase(trailDots.begin(), trailDots.begin() + excess);
			}

			if (isLocalTrailsEnabled) {

				if (!wasLocalTrailsEnabled) {
					trailDots.clear();
				}

				if (pParticlesSelected.size() > 0) {
					float pParticlePosSumX = 0.0f;
					float pParticlePosSumY = 0.0f;
					for (const auto& selectedParticle : pParticlesSelected) {
						pParticlePosSumX += selectedParticle.pos.x;
						pParticlePosSumY += selectedParticle.pos.y;
					}
					selectedParticlesAveragePos = { pParticlePosSumX / pParticlesSelected.size(), pParticlePosSumY / pParticlesSelected.size() };

					for (auto& trailDot : trailDots) {
						trailDot.pos.x = trailDot.offset.x + selectedParticlesAveragePos.x;
						trailDot.pos.y = trailDot.offset.y + selectedParticlesAveragePos.y;
					}
				}
				wasLocalTrailsEnabled = true;
			}
			else {
				wasLocalTrailsEnabled = false;
			}
		}


		else if (isSelectedTrailsEnabled) {
			for (size_t i = 0; i < pParticlesSelected.size(); i++) {

				Vector2 offset = {
			pParticlesSelected[i].pos.x - selectedParticlesAveragePos.x,
			pParticlesSelected[i].pos.y - selectedParticlesAveragePos.y
				};

				trailDots.push_back({ { pParticlesSelected[i].pos }, {offset}, rParticlesSelected[i].color });
			}
			size_t MAX_DOTS = maxLength * pParticlesSelected.size();
			if (trailDots.size() > MAX_DOTS) {
				size_t excess = trailDots.size() - MAX_DOTS;
				trailDots.erase(trailDots.begin(), trailDots.begin() + excess);
			}

			if (isLocalTrailsEnabled) {

				if (!wasLocalTrailsEnabled) {
					trailDots.clear();
				}

				if (pParticlesSelected.size() > 0) {
					float pParticlePosSumX = 0.0f;
					float pParticlePosSumY = 0.0f;
					for (const auto& selectedParticle : pParticlesSelected) {
						pParticlePosSumX += selectedParticle.pos.x;
						pParticlePosSumY += selectedParticle.pos.y;
					}
					selectedParticlesAveragePos = { pParticlePosSumX / pParticlesSelected.size(), pParticlePosSumY / pParticlesSelected.size() };

					for (auto& trailDot : trailDots) {
						trailDot.pos.x = trailDot.offset.x + selectedParticlesAveragePos.x;
						trailDot.pos.y = trailDot.offset.y + selectedParticlesAveragePos.y;
					}
				}
				wasLocalTrailsEnabled = true;
			}
			else {
				wasLocalTrailsEnabled = false;
			}
		}
	}

	if (!isGlobalTrailsEnabled && !isSelectedTrailsEnabled) {
		trailDots.clear();
	}

	if (IsKeyPressed(KEY_C)) {
		pParticles.clear();
		rParticles.clear();
		trailDots.clear();
	}
}



void ParticleTrails::drawTrail(std::vector<ParticleRendering>& rParticles, Texture2D& particleBlur) {

	for (size_t i = 0; i < trailDots.size(); i++) {
		DrawTextureEx(particleBlur, { static_cast<float>(trailDots[i].pos.x - 0.04f * 32 / 2),
				static_cast<float>(trailDots[i].pos.y - 0.04f * 32 / 2) }, 0, 0.04f, trailDots[i].color);
	}
}