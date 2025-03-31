#include <algorithm>
#include <iostream>
#include "morton.h"

uint32_t Morton::scaleToGrid(float pos, float maxVal){
    float clamped = std::clamp(pos, 0.0f, maxVal);
    return static_cast<uint64_t>((clamped / maxVal) * 1023.0f);
}

uint32_t Morton::spreadBits(uint32_t x) {
    x &= 0x0000000000003FF;
    x = (x | (x << 16)) & 0xFFFF0000FFFF;
    x = (x | (x << 8)) & 0x00FF00FF00FF00FF;
    x = (x | (x << 4)) & 0x0F0F0F0F0F0F0F0F;
    x = (x | (x << 2)) & 0x3333333333333333;
    x = (x | (x << 1)) & 0x5555555555555555;
    return x;
}

uint32_t Morton::morton2D(uint32_t x, uint32_t y) {
    return spreadBits(x) | (spreadBits(y) << 1);
}

void Morton::computeMortonKeys(std::vector<ParticlePhysics>& pParticles, float screenSizeX, float screenSizeY){
	for (auto& pParticle : pParticles) {
        uint32_t ix = scaleToGrid(pParticle.pos.x, screenSizeX);
        uint32_t iy = scaleToGrid(pParticle.pos.y, screenSizeY);
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
