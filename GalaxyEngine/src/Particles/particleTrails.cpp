#include "Particles/particleTrails.h"

#include "parameters.h"

void ParticleTrails::trailLogic(UpdateVariables& myVar, UpdateParameters& myParam) {

	if (IO::shortcutPress(KEY_T) && !IsKeyDown(KEY_LEFT_CONTROL)) {
		myVar.isGlobalTrailsEnabled = !myVar.isGlobalTrailsEnabled;
		myVar.isSelectedTrailsEnabled = false;
		segments.clear();
	}
	if (IO::shortcutPress(KEY_T) && IsKeyDown(KEY_LEFT_CONTROL)) {
		myVar.isSelectedTrailsEnabled = !myVar.isSelectedTrailsEnabled;
		myVar.isGlobalTrailsEnabled = false;
		segments.clear();
	}


	if (myVar.timeFactor > 0) {
		if (myVar.isGlobalTrailsEnabled) {
			for (size_t i = 0; i < myParam.pParticles.size(); i++) {

				glm::vec2 offset = {
			myParam.pParticles[i].pos.x - selectedParticlesAveragePos.x,
			myParam.pParticles[i].pos.y - selectedParticlesAveragePos.y
				};

				glm::vec2 prevPos = myParam.pParticles[i].pos - myParam.pParticles[i].vel * myVar.timeFactor;

				glm::vec2 prevOffset = {
			prevPos.x - selectedParticlesAveragePrevPos.x,
			prevPos.y - selectedParticlesAveragePrevPos.y
				};

				segments.push_back({ { myParam.pParticles[i].pos }, { prevPos }, {offset}, {prevOffset}, myParam.rParticles[i].color });
			}

			size_t MAX_DOTS = myVar.trailMaxLength * myParam.pParticles.size();
			if (segments.size() > MAX_DOTS) {
				size_t excess = segments.size() - MAX_DOTS;
				segments.erase(segments.begin(), segments.begin() + excess);
			}

			if (myVar.isLocalTrailsEnabled) {

				if (!wasLocalTrailsEnabled) {
					segments.clear();
				}

				if (myParam.pParticlesSelected.size() > 0) {
					float pParticlePosSumX = 0.0f;
					float pParticlePosSumY = 0.0f;

					float pParticlePrevPosSumX = 0.0f;
					float pParticlePrevPosSumY = 0.0f;

					for (const auto& selectedParticle : myParam.pParticlesSelected) {
						pParticlePosSumX += selectedParticle.pos.x;
						pParticlePosSumY += selectedParticle.pos.y;

						glm::vec2 prevPos = selectedParticle.pos - selectedParticle.vel * myVar.timeFactor;

						pParticlePrevPosSumX += prevPos.x;
						pParticlePrevPosSumY += prevPos.y;
					}
					selectedParticlesAveragePos = { pParticlePosSumX / myParam.pParticlesSelected.size(), pParticlePosSumY / myParam.pParticlesSelected.size() };
					selectedParticlesAveragePrevPos = { pParticlePrevPosSumX / myParam.pParticlesSelected.size(), pParticlePrevPosSumY / myParam.pParticlesSelected.size() };

					for (auto& segment : segments) {
						segment.start.x = segment.offset.x + selectedParticlesAveragePos.x;
						segment.start.y = segment.offset.y + selectedParticlesAveragePos.y;

						segment.end.x = segment.prevOffset.x + selectedParticlesAveragePos.x;
						segment.end.y = segment.prevOffset.y + selectedParticlesAveragePos.y;
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

				glm::vec2 offset = {
			myParam.pParticlesSelected[i].pos.x - selectedParticlesAveragePos.x,
			myParam.pParticlesSelected[i].pos.y - selectedParticlesAveragePos.y
				};

				glm::vec2 prevPos = myParam.pParticlesSelected[i].pos - myParam.pParticlesSelected[i].vel * myVar.timeFactor;

				glm::vec2 prevOffset = {
			prevPos.x - selectedParticlesAveragePrevPos.x,
			prevPos.y - selectedParticlesAveragePrevPos.y
				};

				segments.push_back({ { myParam.pParticlesSelected[i].pos }, { prevPos }, {offset}, {prevOffset}, myParam.rParticlesSelected[i].color });
			}
			size_t MAX_DOTS = myVar.trailMaxLength * myParam.pParticlesSelected.size();
			if (segments.size() > MAX_DOTS) {
				size_t excess = segments.size() - MAX_DOTS;
				segments.erase(segments.begin(), segments.begin() + excess);
			}

			if (myVar.isLocalTrailsEnabled) {

				if (!wasLocalTrailsEnabled) {
					segments.clear();
				}

				if (myParam.pParticlesSelected.size() > 0) {
					float pParticlePosSumX = 0.0f;
					float pParticlePosSumY = 0.0f;

					float pParticlePrevPosSumX = 0.0f;
					float pParticlePrevPosSumY = 0.0f;

					for (const auto& selectedParticle : myParam.pParticlesSelected) {
						pParticlePosSumX += selectedParticle.pos.x;
						pParticlePosSumY += selectedParticle.pos.y;

						glm::vec2 prevPos = selectedParticle.pos - selectedParticle.vel * myVar.timeFactor;

						pParticlePrevPosSumX += prevPos.x;
						pParticlePrevPosSumY += prevPos.y;
					}
					selectedParticlesAveragePos = { pParticlePosSumX / myParam.pParticlesSelected.size(), pParticlePosSumY / myParam.pParticlesSelected.size() };
					selectedParticlesAveragePrevPos = { pParticlePrevPosSumX / myParam.pParticlesSelected.size(), pParticlePrevPosSumY / myParam.pParticlesSelected.size() };

					for (auto& segment : segments) {
						segment.start.x = segment.offset.x + selectedParticlesAveragePos.x;
						segment.start.y = segment.offset.y + selectedParticlesAveragePos.y;

						segment.end.x = segment.prevOffset.x + selectedParticlesAveragePos.x;
						segment.end.y = segment.prevOffset.y + selectedParticlesAveragePos.y;
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
		segments.clear();
	}

	if (IO::shortcutPress(KEY_C)) {
		myParam.pParticles.clear();
		myParam.rParticles.clear();
		segments.clear();
	}
}

void ParticleTrails::drawTrail(std::vector<ParticleRendering>& rParticles, Texture2D& particleBlur) {

	if (!whiteTrails) {
		if (!segments.empty()) {
			for (size_t i = 0; i < segments.size(); i++) {
				DrawLineEx({ segments[i].start.x, segments[i].start.y }, { segments[i].end.x ,segments[i].end.y }, trailThickness, segments[i].color);
			}
		}
	}
	else {
		if (!segments.empty()) {
			for (size_t i = 0; i < segments.size(); i++) {
				DrawLineEx({ segments[i].start.x, segments[i].start.y }, { segments[i].end.x ,segments[i].end.y }, trailThickness, {255,255,255,160});
			}
		}
	}

}