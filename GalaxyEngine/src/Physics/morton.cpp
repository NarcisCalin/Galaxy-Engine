#include "Physics/morton.h"

uint64_t Morton::scaleToGrid(float pos, float minVal, float maxVal) {
    if (maxVal <= minVal) return 0;
    float clamped = std::clamp(pos, minVal, maxVal);
    float normalized = (clamped - minVal) / (maxVal - minVal);

    uint64_t scaled = static_cast<uint64_t>(normalized * 262144.0f);
    return std::min(scaled, 262143ULL);
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
    std::vector<ParticleRendering>& rParticles) {
    const size_t n = pParticles.size();

    indicesBuffer.resize(n);
    pSortedBuffer.resize(n);
    rSortedBuffer.resize(n);

    std::iota(indicesBuffer.begin(), indicesBuffer.end(), 0);

    std::sort(indicesBuffer.begin(), indicesBuffer.end(),
        [&](size_t a, size_t b) {
            return pParticles[a].mortonKey < pParticles[b].mortonKey;
        });

    for (size_t i = 0; i < n; i++) {
        pSortedBuffer[i] = pParticles[indicesBuffer[i]];
        rSortedBuffer[i] = rParticles[indicesBuffer[i]];
    }

    std::swap(pParticles, pSortedBuffer);
    std::swap(rParticles, rSortedBuffer);
}

// ---- 3D Implementation ---- //

uint64_t Morton::scaleToGrid3D(float pos, float minVal, float maxVal) {
    if (maxVal <= minVal) return 0;

    float clamped = std::clamp(pos, minVal, maxVal);
    float normalized = (clamped - minVal) / (maxVal - minVal);

    uint64_t scaled = static_cast<uint64_t>(normalized * 2097151.0f);
    return std::min(scaled, 2097151ULL);
}

uint64_t Morton::spreadBits3D(uint64_t x) {
    uint64_t result = 0;

    for (int i = 0; i < 21; ++i) {
        uint64_t bit = (x >> i) & 1ULL;
        result |= (bit << (3 * i));
    }

    return result;
}

uint64_t Morton::morton3D(uint64_t x, uint64_t y, uint64_t z) {
    return spreadBits3D(x) | (spreadBits3D(y) << 1) | (spreadBits3D(z) << 2);
}

void Morton::computeMortonKeys3D(std::vector<ParticlePhysics3D>& pParticles,
    const glm::vec4& boundingBox) {

    float minX = boundingBox.x;
    float minY = boundingBox.y;
    float minZ = boundingBox.z;

    float size = boundingBox.w;

    float maxX = minX + size;
    float maxY = minY + size;
    float maxZ = minZ + size;

    for (auto& pParticle : pParticles) {
        uint64_t ix = scaleToGrid3D(pParticle.pos.x, minX, maxX);
        uint64_t iy = scaleToGrid3D(pParticle.pos.y, minY, maxY);
        uint64_t iz = scaleToGrid3D(pParticle.pos.z, minZ, maxZ);

        pParticle.mortonKey = morton3D(ix, iy, iz);
    }
}

void Morton::sortParticlesByMortonKey3D(
    std::vector<ParticlePhysics3D>& pParticles,
    std::vector<ParticleRendering3D>& rParticles) {

    const size_t n = pParticles.size();

    indicesBuffer3D.resize(n);
    pSortedBuffer3D.resize(n);
    rSortedBuffer3D.resize(n);

    std::iota(indicesBuffer3D.begin(), indicesBuffer3D.end(), 0);

    std::sort(indicesBuffer3D.begin(), indicesBuffer3D.end(),
        [&](size_t a, size_t b) {
            return pParticles[a].mortonKey < pParticles[b].mortonKey;
        });

    for (size_t i = 0; i < n; i++) {
        pSortedBuffer3D[i] = pParticles[indicesBuffer3D[i]];
        rSortedBuffer3D[i] = rParticles[indicesBuffer3D[i]];
    }

    std::swap(pParticles, pSortedBuffer3D);
    std::swap(rParticles, rSortedBuffer3D);
}