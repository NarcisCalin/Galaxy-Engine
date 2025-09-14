#include "Physics/morton.h"

uint64_t Morton::scaleToGrid(float pos, float minVal, float maxVal) {
    if (maxVal <= minVal) return 0;
    float clamped = std::clamp(pos, minVal, maxVal);
    float normalized = (clamped - minVal) / (maxVal - minVal);
    return static_cast<uint64_t>(normalized * 262143.0f);
}

uint64_t Morton::spreadBits(uint64_t x) {
    x &= 0x3FFFF;                     // keep only 18 bits
    x = (x | (x << 16)) & 0x0000FFFF0000FFFFULL;
    x = (x | (x << 8)) & 0x00FF00FF00FF00FFULL;
    x = (x | (x << 4)) & 0x0F0F0F0F0F0F0F0FULL;
    x = (x | (x << 2)) & 0x3333333333333333ULL;
    x = (x | (x << 1)) & 0x5555555555555555ULL;
    return static_cast<uint64_t>(x);
}

uint64_t Morton::morton2D(uint64_t x, uint64_t y) {
    return static_cast<uint64_t>(spreadBits(x))
        | (static_cast<uint64_t>(spreadBits(y)) << 1);
}

void Morton::computeMortonKeys(std::vector<ParticlePhysics>& pParticles,
    glm::vec3& posSize)
{
    const float maxX = posSize.x + std::max(posSize.z, 1e-6f);
    const float maxY = posSize.y + std::max(posSize.z, 1e-6f);

    for (auto& pParticle : pParticles) {
        uint64_t ix = scaleToGrid(pParticle.pos.x, posSize.x, maxX);
        uint64_t iy = scaleToGrid(pParticle.pos.y, posSize.y, maxY);
        pParticle.mortonKey = morton2D(ix, iy);
    }
}

void Morton::sortParticlesByMortonKey(
    std::vector<ParticlePhysics>& pParticles,
    std::vector<ParticleRendering>& rParticles)
{
    std::vector<size_t> indices(pParticles.size());

#pragma omp parallel for
    for (size_t i = 0; i < indices.size(); i++) {
        indices[i] = i;
    }

    std::sort(std::execution::par, indices.begin(), indices.end(),
        [&](size_t a, size_t b) {
            return pParticles[a].mortonKey < pParticles[b].mortonKey;
        });

    std::vector<ParticlePhysics> pSorted(pParticles.size());
    std::vector<ParticleRendering> rSorted(rParticles.size());

#pragma omp parallel for
    for (size_t i = 0; i < indices.size(); i++) {
        size_t src_idx = indices[i];
        pSorted[i] = std::move(pParticles[src_idx]);
        rSorted[i] = std::move(rParticles[src_idx]);
    }

    pParticles = std::move(pSorted);
    rParticles = std::move(rSorted);
}
