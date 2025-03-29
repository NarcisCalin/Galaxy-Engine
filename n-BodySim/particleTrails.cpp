#include "particleTrails.h"

ParticleTrails::ParticleTrails() {
}

void ParticleTrails::trailLogic(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, 
	bool& isGlobalTrailsEnabled, bool& isSelectedTrailsEnabled, int& maxLength) {
	const size_t NUM_PLANETS = pParticles.size();

	if (IsKeyPressed(KEY_T) && !IsKeyDown(KEY_LEFT_CONTROL)) {
		isGlobalTrailsEnabled = !isGlobalTrailsEnabled;
		isSelectedTrailsEnabled = false;
		trailsParameters.clear();
	}
	if (IsKeyPressed(KEY_T) && IsKeyDown(KEY_LEFT_CONTROL)) {
		isSelectedTrailsEnabled = !isSelectedTrailsEnabled;
		isGlobalTrailsEnabled = false;
		trailsParameters.clear();
	}

	if (isGlobalTrailsEnabled) {
		for (size_t i = 0; i < pParticles.size(); i++) {
				trailsParameters.push_back({ { pParticles[i].pos.x, pParticles[i].pos.y }, rParticles[i].color });
		}

		size_t MAX_DOTS = maxLength * pParticles.size();
		if (trailsParameters.size() > MAX_DOTS) {
			size_t excess = trailsParameters.size() - MAX_DOTS;
			trailsParameters.erase(trailsParameters.begin(), trailsParameters.begin() + excess);
		}
	}
	else if (isSelectedTrailsEnabled) {

		for (size_t i = 0; i < pParticles.size(); i++) {
			if (rParticles[i].isSelected) {
			trailsParameters.push_back({ { pParticles[i].pos.x, pParticles[i].pos.y }, rParticles[i].color });
			}
		}

		size_t MAX_DOTS = 1 * pParticles.size();
		if (trailsParameters.size() > MAX_DOTS) {
			size_t excess = trailsParameters.size() - MAX_DOTS;
			trailsParameters.erase(trailsParameters.begin(), trailsParameters.begin() + excess);
		}

	}
	else {
		trailsParameters.clear();
	}

	if (IsKeyPressed(KEY_C)) {
		pParticles.clear();
		rParticles.clear();
		trailsParameters.clear();
	}
}


void ParticleTrails::drawTrail(std::vector<ParticleRendering>& rParticles, Texture2D& particleBlur) {

	for (size_t i = 0; i < trailsParameters.size(); i++) {
		DrawTextureEx(particleBlur, { static_cast<float>(trailsParameters[i].pos.x - 0.04f * 32 / 2),
				static_cast<float>(trailsParameters[i].pos.y - 0.04f * 32 / 2) }, 0, 0.04f, trailsParameters[i].color);
	}
}