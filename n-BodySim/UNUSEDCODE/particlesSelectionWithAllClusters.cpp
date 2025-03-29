#include "particlesSelection.h"
#include "particleTrails.h"



ClusterGrouping::ClusterGrouping() {
	groupTrails = new ParticleTrails();
}

ClusterGrouping::~ClusterGrouping() {
	delete groupTrails;
}

void ClusterGrouping::clusterManualSelection(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles){
}


// THIS FUNCTION BELOW IS UNUSED
void ClusterGrouping::allClustersAutomaticSelection(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles, Texture2D& particleBlur) {
	std::vector<int> neighborCountsSelect(pParticles.size(), 0);
	for (size_t i = 0; i < pParticles.size(); i++) {
		const auto& pParticle = pParticles[i];
		for (size_t j = i + 1; j < pParticles.size(); j++) {
			if (std::abs(pParticles[j].pos.x - pParticle.pos.x) > 4.0f) break;
			float dx = pParticle.pos.x - pParticles[j].pos.x;
			float dy = pParticle.pos.y - pParticles[j].pos.y;
			if (dx * dx + dy * dy < selectionThreshold * selectionThreshold) {
				neighborCountsSelect[i]++;
				neighborCountsSelect[j]++;
			}
		}
	}

	int groupIdx = -1;
	groupOfGroups.clear();
	for (size_t i = 0; i < pParticles.size(); i++) {
		bool validPrev = (i > 0);
		bool validNext = (i < pParticles.size() - 1);

		if (neighborCountsSelect[i] > 3 && validPrev && neighborCountsSelect[i - 1] < 3) {
			groupOfGroups.push_back(groupOfParticles);
			groupIdx++;
		}

		if (neighborCountsSelect[i] > 3 &&
			((validNext && neighborCountsSelect[i + 1] > 3) || (validPrev && neighborCountsSelect[i - 1] > 3))) {
			if (groupIdx >= 0 && groupIdx < groupOfGroups.size()) {
				groupOfGroups[groupIdx].push_back(pParticles[i]);
			}
		}
	}

	for (const auto& group : groupOfGroups) {
		float sumX = 0.0f;
		float sumY = 0.0f;
		for (const auto& particle : group) {
			sumX += particle.pos.x;
			sumY += particle.pos.y;
		}

		size_t count = group.size();
		if (count > 0) {
			float avgX = sumX / count;
			float avgY = sumY / count;

			groupsParamsVector.emplace_back(Vector2{ avgX, avgY }, Color{ 128,128,128,255 });

			//DrawCircleV({ avgX, avgY }, 5.0f, BLUE);
		}
	}

	bool isTrue = true;
	if (groupOfGroups.size() > 0) {
		groupTrails->trailLogic(groupsParamsVector, groupsParamsVector, isTrue, clustersMaxTrailLength);
		groupTrails->drawTrail(groupsParamsVector, particleBlur);
	}

	groupsParamsVector.clear();
}
