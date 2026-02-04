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


	if (myVar.timeFactor > 0.0f) {
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
		myParam.pParticles3D.clear();
		myParam.rParticles3D.clear();
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

void ParticleTrails::trailLogic3D(UpdateVariables& myVar, UpdateParameters& myParam) {

	if (IO::shortcutPress(KEY_T) && !IsKeyDown(KEY_LEFT_CONTROL)) {
		myVar.isGlobalTrailsEnabled = !myVar.isGlobalTrailsEnabled;
		myVar.isSelectedTrailsEnabled = false;
		segments3D.clear();
	}
	if (IO::shortcutPress(KEY_T) && IsKeyDown(KEY_LEFT_CONTROL)) {
		myVar.isSelectedTrailsEnabled = !myVar.isSelectedTrailsEnabled;
		myVar.isGlobalTrailsEnabled = false;
		segments3D.clear();
	}

	if (myVar.timeFactor > 0.0f) {
		if (myVar.isGlobalTrailsEnabled) {
			for (size_t i = 0; i < myParam.pParticles3D.size(); i++) {

				glm::vec3 offset = {
					myParam.pParticles3D[i].pos.x - selectedParticlesAveragePos3D.x,
					myParam.pParticles3D[i].pos.y - selectedParticlesAveragePos3D.y,
					myParam.pParticles3D[i].pos.z - selectedParticlesAveragePos3D.z
				};

				glm::vec3 prevPos = myParam.pParticles3D[i].pos - myParam.pParticles3D[i].vel * myVar.timeFactor;

				glm::vec3 prevOffset = {
					prevPos.x - selectedParticlesAveragePrevPos3D.x,
					prevPos.y - selectedParticlesAveragePrevPos3D.y,
					prevPos.z - selectedParticlesAveragePrevPos3D.z
				};

				segments3D.push_back({ { myParam.pParticles3D[i].pos }, { prevPos }, { offset }, { prevOffset }, myParam.rParticles3D[i].color });
			}

			size_t MAX_DOTS = myVar.trailMaxLength * myParam.pParticles3D.size();
			if (segments3D.size() > MAX_DOTS) {
				size_t excess = segments3D.size() - MAX_DOTS;
				segments3D.erase(segments3D.begin(), segments3D.begin() + excess);
			}

			if (myVar.isLocalTrailsEnabled) {

				if (!wasLocalTrailsEnabled) {
					segments3D.clear();
				}

				if (myParam.pParticlesSelected3D.size() > 0) {
					float pParticlePosSumX = 0.0f;
					float pParticlePosSumY = 0.0f;
					float pParticlePosSumZ = 0.0f;

					float pParticlePrevPosSumX = 0.0f;
					float pParticlePrevPosSumY = 0.0f;
					float pParticlePrevPosSumZ = 0.0f;

					for (const auto& selectedParticle : myParam.pParticlesSelected3D) {
						pParticlePosSumX += selectedParticle.pos.x;
						pParticlePosSumY += selectedParticle.pos.y;
						pParticlePosSumZ += selectedParticle.pos.z;

						glm::vec3 prevPos = selectedParticle.pos - selectedParticle.vel * myVar.timeFactor;

						pParticlePrevPosSumX += prevPos.x;
						pParticlePrevPosSumY += prevPos.y;
						pParticlePrevPosSumZ += prevPos.z;
					}
					selectedParticlesAveragePos3D = {
						pParticlePosSumX / myParam.pParticlesSelected3D.size(),
						pParticlePosSumY / myParam.pParticlesSelected3D.size(),
						pParticlePosSumZ / myParam.pParticlesSelected3D.size()
					};
					selectedParticlesAveragePrevPos3D = {
						pParticlePrevPosSumX / myParam.pParticlesSelected3D.size(),
						pParticlePrevPosSumY / myParam.pParticlesSelected3D.size(),
						pParticlePrevPosSumZ / myParam.pParticlesSelected3D.size()
					};

					for (auto& segment : segments3D) {
						segment.start.x = segment.offset.x + selectedParticlesAveragePos3D.x;
						segment.start.y = segment.offset.y + selectedParticlesAveragePos3D.y;
						segment.start.z = segment.offset.z + selectedParticlesAveragePos3D.z;

						segment.end.x = segment.prevOffset.x + selectedParticlesAveragePos3D.x;
						segment.end.y = segment.prevOffset.y + selectedParticlesAveragePos3D.y;
						segment.end.z = segment.prevOffset.z + selectedParticlesAveragePos3D.z;
					}
				}
				wasLocalTrailsEnabled = true;
			}
			else {
				wasLocalTrailsEnabled = false;
			}
		}

		else if (myVar.isSelectedTrailsEnabled) {
			for (size_t i = 0; i < myParam.pParticlesSelected3D.size(); i++) {

				glm::vec3 offset = {
					myParam.pParticlesSelected3D[i].pos.x - selectedParticlesAveragePos3D.x,
					myParam.pParticlesSelected3D[i].pos.y - selectedParticlesAveragePos3D.y,
					myParam.pParticlesSelected3D[i].pos.z - selectedParticlesAveragePos3D.z
				};

				glm::vec3 prevPos = myParam.pParticlesSelected3D[i].pos - myParam.pParticlesSelected3D[i].vel * myVar.timeFactor;

				glm::vec3 prevOffset = {
					prevPos.x - selectedParticlesAveragePrevPos3D.x,
					prevPos.y - selectedParticlesAveragePrevPos3D.y,
					prevPos.z - selectedParticlesAveragePrevPos3D.z
				};

				segments3D.push_back({ { myParam.pParticlesSelected3D[i].pos }, { prevPos }, { offset }, { prevOffset }, myParam.rParticlesSelected3D[i].color });
			}

			size_t MAX_DOTS = myVar.trailMaxLength * myParam.pParticlesSelected3D.size();
			if (segments3D.size() > MAX_DOTS) {
				size_t excess = segments3D.size() - MAX_DOTS;
				segments3D.erase(segments3D.begin(), segments3D.begin() + excess);
			}

			if (myVar.isLocalTrailsEnabled) {

				if (!wasLocalTrailsEnabled) {
					segments3D.clear();
				}

				if (myParam.pParticlesSelected3D.size() > 0) {
					float pParticlePosSumX = 0.0f;
					float pParticlePosSumY = 0.0f;
					float pParticlePosSumZ = 0.0f;

					float pParticlePrevPosSumX = 0.0f;
					float pParticlePrevPosSumY = 0.0f;
					float pParticlePrevPosSumZ = 0.0f;

					for (const auto& selectedParticle : myParam.pParticlesSelected3D) {
						pParticlePosSumX += selectedParticle.pos.x;
						pParticlePosSumY += selectedParticle.pos.y;
						pParticlePosSumZ += selectedParticle.pos.z;

						glm::vec3 prevPos = selectedParticle.pos - selectedParticle.vel * myVar.timeFactor;

						pParticlePrevPosSumX += prevPos.x;
						pParticlePrevPosSumY += prevPos.y;
						pParticlePrevPosSumZ += prevPos.z;
					}
					selectedParticlesAveragePos3D = {
						pParticlePosSumX / myParam.pParticlesSelected3D.size(),
						pParticlePosSumY / myParam.pParticlesSelected3D.size(),
						pParticlePosSumZ / myParam.pParticlesSelected3D.size()
					};
					selectedParticlesAveragePrevPos3D = {
						pParticlePrevPosSumX / myParam.pParticlesSelected3D.size(),
						pParticlePrevPosSumY / myParam.pParticlesSelected3D.size(),
						pParticlePrevPosSumZ / myParam.pParticlesSelected3D.size()
					};

					for (auto& segment : segments3D) {
						segment.start.x = segment.offset.x + selectedParticlesAveragePos3D.x;
						segment.start.y = segment.offset.y + selectedParticlesAveragePos3D.y;
						segment.start.z = segment.offset.z + selectedParticlesAveragePos3D.z;

						segment.end.x = segment.prevOffset.x + selectedParticlesAveragePos3D.x;
						segment.end.y = segment.prevOffset.y + selectedParticlesAveragePos3D.y;
						segment.end.z = segment.prevOffset.z + selectedParticlesAveragePos3D.z;
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
		segments3D.clear();
	}

	if (IO::shortcutPress(KEY_C)) {
		myParam.pParticles.clear();
		myParam.rParticles.clear();
		myParam.pParticles3D.clear();
		myParam.rParticles3D.clear();
		segments3D.clear();
	}
}

void ParticleTrails::drawTrail3D(std::vector<ParticleRendering3D>& rParticles3D, Texture2D& particleBlur, Camera3D& cam3D) {

	if (!whiteTrails) {
		if (!segments3D.empty()) {
			for (size_t i = 0; i < segments3D.size(); i++) {

				DrawLine3D({segments3D[i].start.x,segments3D[i].start.y,segments3D[i].start.z }, 
					{ segments3D[i].end.x,segments3D[i].end.y,segments3D[i].end.z },
					segments3D[i].color);
			}
		}
	}
	else {
		if (!segments3D.empty()) {
			for (size_t i = 0; i < segments3D.size(); i++) {
				DrawLine3D({ segments3D[i].start.x,segments3D[i].start.y,segments3D[i].start.z },
					{ segments3D[i].end.x,segments3D[i].end.y,segments3D[i].end.z },
					{ 255,255,255,160 });
			}
		}
	}
}