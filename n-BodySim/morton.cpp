#include <algorithm>
#include <iostream>
#include "morton.h"

uint64_t Morton::scaleToGrid(float pos, float maxVal){
    float clamped = std::clamp(pos, 0.0f, maxVal);
    return static_cast<uint64_t>((clamped / maxVal) * 1023.0f);
}

uint64_t Morton::spreadBits(uint64_t x) {
    x = (x | (x << 16)) & 0x030000FF;
    x = (x | (x << 8)) & 0x0300F00F;
    x = (x | (x << 4)) & 0x030C30C3;
    x = (x | (x << 2)) & 0x09249249;
    return x;
}

uint64_t Morton::morton2D(uint64_t x, uint64_t y){
	return (spreadBits(y) << 1) | spreadBits(x);
}

void Morton::computeMortonKeys(std::vector<ParticlePhysics>& pParticles, float screenSizeX, float screenSizeY){
	for (auto& pParticle : pParticles) {
        uint64_t ix = scaleToGrid(pParticle.pos.x, screenSizeX);
        uint64_t iy = scaleToGrid(pParticle.pos.y, screenSizeY);
		pParticle.mortonKey = morton2D(ix, iy);
	}
}

void Morton::sortParticlesByMortonKey(
    std::vector<ParticlePhysics>& pParticles,
    std::vector<ParticleRendering>& rParticles)
{

    std::vector<size_t> indices(pParticles.size());
    for (size_t i = 0; i < indices.size(); i++) {
        indices[i] = i;
    }

    std::sort(indices.begin(), indices.end(), [&](size_t a, size_t b) {
        return pParticles[a].mortonKey < pParticles[b].mortonKey;
        });

    std::vector<ParticlePhysics> pSorted;
    std::vector<ParticleRendering> rSorted;
    pSorted.reserve(pParticles.size());
    rSorted.reserve(rParticles.size());

    for (size_t i = 0; i < indices.size(); i++) {
        pSorted.push_back(pParticles[indices[i]]);
        rSorted.push_back(rParticles[indices[i]]);
    }

    pParticles = std::move(pSorted);
    rParticles = std::move(rSorted);
}
