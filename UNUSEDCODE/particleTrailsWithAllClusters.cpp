#include "particleTrails.h"

ParticleTrails::ParticleTrails() {
}

template <typename T, typename B>
void ParticleTrails::trailLogic(std::vector<T>& pParticles, std::vector<B>& rParticles, bool& trailsEnabled, int& maxLength ) {
	const size_t NUM_PLANETS = pParticles.size();

	if (IsKeyPressed(KEY_T)) {
		trailsEnabled = !trailsEnabled;
	}

	if (trailsEnabled) {
		for (size_t i = 0; i < pParticles.size(); i++) {
			trailsParameters.push_back({ { pParticles[i].pos.x, pParticles[i].pos.y }, rParticles[i].color });
		}
	}
	else {
		trailsParameters.clear();
	}

	size_t MAX_DOTS = maxLength * pParticles.size();
	if (trailsParameters.size() > MAX_DOTS) {
		size_t excess = trailsParameters.size() - MAX_DOTS;
		trailsParameters.erase(trailsParameters.begin(), trailsParameters.begin() + excess);
	}
	if (IsKeyPressed(KEY_C)) {
		pParticles.clear();
		rParticles.clear();
		trailsParameters.clear();
	}
}

template <typename T>
void ParticleTrails::drawTrail(std::vector<T>& rParticles, Texture2D& particleBlur) {

	for (size_t i = 0; i < trailsParameters.size(); i++) {
		DrawTextureEx(particleBlur, { static_cast<float>(trailsParameters[i].pos.x - 0.04f * 32 / 2),
				static_cast<float>(trailsParameters[i].pos.y - 0.04f * 32 / 2)}, 0, 0.04f, trailsParameters[i].color);
	}
}

template void ParticleTrails::trailLogic<ParticlePhysics, ParticleRendering>(std::vector<ParticlePhysics>& pParticles,
	std::vector<ParticleRendering>& rParticles,
	bool& trailsEnabled, int& maxLength);

template void ParticleTrails::trailLogic<GroupParams, GroupParams>(std::vector<GroupParams>& pParticles,
	std::vector<GroupParams>& rParticles,
	bool& trailsEnabled, int& maxLength);

template void ParticleTrails::drawTrail<ParticleRendering>(std::vector<ParticleRendering>& rParticles, Texture2D& particleBlur);

template void ParticleTrails::drawTrail<GroupParams>(std::vector<GroupParams>& rParticles, Texture2D& particleBlur);