#include "../../include/Particles/particleTrails.h"
#include "../../include/parameters.h"

ParticleTrails::ParticleTrails() {
}

void ParticleTrails::trailLogic(UpdateVariables& myVar, UpdateParameters& myParam) {

	const size_t NUM_PLANETS = myParam.pParticles.size();

	if (IsKeyPressed(KEY_T) && !IsKeyDown(KEY_LEFT_CONTROL)) {
		myVar.isGlobalTrailsEnabled = !myVar.isGlobalTrailsEnabled;
		myVar.isSelectedTrailsEnabled = false;
		trailDots.clear();
	}
	if (IsKeyPressed(KEY_T) && IsKeyDown(KEY_LEFT_CONTROL)) {
		myVar.isSelectedTrailsEnabled = !myVar.isSelectedTrailsEnabled;
		myVar.isGlobalTrailsEnabled = false;
		trailDots.clear();
	}


	if (myVar.timeFactor > 0) {
		if (myVar.isGlobalTrailsEnabled) {
			for (size_t i = 0; i < myParam.pParticles.size(); i++) {

				Vector2 offset = {
			myParam.pParticles[i].pos.x - selectedParticlesAveragePos.x,
			myParam.pParticles[i].pos.y - selectedParticlesAveragePos.y
				};

				trailDots.push_back({ { myParam.pParticles[i].pos }, {offset}, myParam.rParticles[i].color });
			}

			size_t MAX_DOTS = myVar.trailMaxLength * myParam.pParticles.size();
			if (trailDots.size() > MAX_DOTS) {
				size_t excess = trailDots.size() - MAX_DOTS;
				trailDots.erase(trailDots.begin(), trailDots.begin() + excess);
			}

			if (myVar.isLocalTrailsEnabled) {

				if (!wasLocalTrailsEnabled) {
					trailDots.clear();
				}

				if (myParam.pParticlesSelected.size() > 0) {
					float pParticlePosSumX = 0.0f;
					float pParticlePosSumY = 0.0f;
					for (const auto& selectedParticle : myParam.pParticlesSelected) {
						pParticlePosSumX += selectedParticle.pos.x;
						pParticlePosSumY += selectedParticle.pos.y;
					}
					selectedParticlesAveragePos = { pParticlePosSumX / myParam.pParticlesSelected.size(), pParticlePosSumY / myParam.pParticlesSelected.size() };

					for (auto& TrailDot : trailDots) {
						TrailDot.pos.x = TrailDot.offset.x + selectedParticlesAveragePos.x;
						TrailDot.pos.y = TrailDot.offset.y + selectedParticlesAveragePos.y;
					}
				}
				wasLocalTrailsEnabled = true;
			}
			else {
				wasLocalTrailsEnabled = false;
			}
		}


		else if (myVar.isSelectedTrailsEnabled) {
			for (size_t i = 0; i < myParam.pParticlesSelected.size(); i++) {

				Vector2 offset = {
			myParam.pParticlesSelected[i].pos.x - selectedParticlesAveragePos.x,
			myParam.pParticlesSelected[i].pos.y - selectedParticlesAveragePos.y
				};

				trailDots.push_back({ { myParam.pParticlesSelected[i].pos }, {offset}, myParam.rParticlesSelected[i].color });
			}
			size_t MAX_DOTS = myVar.trailMaxLength * myParam.pParticlesSelected.size();
			if (trailDots.size() > MAX_DOTS) {
				size_t excess = trailDots.size() - MAX_DOTS;
				trailDots.erase(trailDots.begin(), trailDots.begin() + excess);
			}

			if (myVar.isLocalTrailsEnabled) {

				if (!wasLocalTrailsEnabled) {
					trailDots.clear();
				}

				if (myParam.pParticlesSelected.size() > 0) {
					float pParticlePosSumX = 0.0f;
					float pParticlePosSumY = 0.0f;
					for (const auto& selectedParticle : myParam.pParticlesSelected) {
						pParticlePosSumX += selectedParticle.pos.x;
						pParticlePosSumY += selectedParticle.pos.y;
					}
					selectedParticlesAveragePos = { pParticlePosSumX / myParam.pParticlesSelected.size(), pParticlePosSumY / myParam.pParticlesSelected.size() };

					for (auto& TrailDot : trailDots) {
						TrailDot.pos.x = TrailDot.offset.x + selectedParticlesAveragePos.x;
						TrailDot.pos.y = TrailDot.offset.y + selectedParticlesAveragePos.y;
					}
				}
				wasLocalTrailsEnabled = true;
			}
			else {
				wasLocalTrailsEnabled = false;
			}
		}
	}

	if (!myVar.isGlobalTrailsEnabled && !myVar.isSelectedTrailsEnabled) {
		trailDots.clear();
	}

	if (IsKeyPressed(KEY_C)) {
		myParam.pParticles.clear();
		myParam.rParticles.clear();
		trailDots.clear();
	}
}



void ParticleTrails::drawTrail(std::vector<ParticleRendering>& rParticles, Texture2D& particleBlur) {

	if (!whiteTrails) {
		for (size_t i = 0; i < trailDots.size(); i++) {
			DrawTextureEx(particleBlur, { static_cast<float>(trailDots[i].pos.x - trailThickness * 32 / 2),
					static_cast<float>(trailDots[i].pos.y - trailThickness * 32 / 2) }, 0, trailThickness, trailDots[i].color);
		}
	}
	else {
		for (size_t i = 0; i < trailDots.size(); i++) {
			DrawTextureEx(particleBlur, { static_cast<float>(trailDots[i].pos.x - trailThickness * 32 / 2),
					static_cast<float>(trailDots[i].pos.y - trailThickness * 32 / 2) }, 0, trailThickness, {230,230,230,180});
		}
	}

}