#include <algorithm>
#include <iostream>
#include "../../include/Physics/morton.h"

uint32_t Morton::scaleToGrid(float pos, float minVal, float maxVal) {
    if (maxVal <= minVal) return 0;
    float clamped = std::clamp(pos, minVal, maxVal);
    float normalized = (clamped - minVal) / (maxVal - minVal);
    return static_cast<uint32_t>(normalized * 2047.0f);
}

uint32_t Morton::spreadBits(uint32_t x) {
    x &= 0x7FF;
    x = (x | (x << 8)) & 0x00FF00FF00FF00FF;
    x = (x | (x << 4)) & 0x0F0F0F0F0F0F0F0F;
    x = (x | (x << 2)) & 0x3333333333333333;
    x = (x | (x << 1)) & 0x5555555555555555;
    return x;
}

uint32_t Morton::morton2D(uint32_t x, uint32_t y) {
    return spreadBits(x) | (spreadBits(y) << 1);
}

void Morton::computeMortonKeys(std::vector<ParticlePhysics>& pParticles, Vector2& minPosition, float& size){
    const float maxX = minPosition.x + std::max(size, 1e-6f);
    const float maxY = minPosition.y + std::max(size, 1e-6f);

    for (auto& pParticle : pParticles) {
        uint32_t ix = scaleToGrid(pParticle.pos.x, minPosition.x, maxX);
        uint32_t iy = scaleToGrid(pParticle.pos.y, minPosition.y, maxY);
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
