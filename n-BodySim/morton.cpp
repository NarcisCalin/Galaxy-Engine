#include <algorithm>
#include <stdexcept>
#include "morton.h"

uint32_t Morton::scaleToGrid(float pos, float maxVal){
	return static_cast<uint32_t>((pos / maxVal) * 1023);
}

uint32_t Morton::spreadBits(uint32_t x){
	x &= 0x3FF;
	x = (x | (x << 16)) & 0x30000FF;
	x = (x | (x << 8)) & 0x300F00F;
	x = (x | (x << 4)) & 0x30C30C3;
	x = (x | (x << 2)) & 0x9249249;
	return x;
}

uint32_t Morton::morton2D(uint32_t x, uint32_t y){
	return (spreadBits(y) << 1) | spreadBits(x);
}

void Morton::computeMortonKeys(std::vector<ParticlePhysics>& pParticles){
	for (auto& pParticle : pParticles) {
		uint32_t ix = scaleToGrid(pParticle.pos.x);
		uint32_t iy = scaleToGrid(pParticle.pos.y);
		pParticle.mortonKey = morton2D(ix, iy);
	}
}

void Morton::sortParticlesByMortonKey(
    std::vector<ParticlePhysics>& pParticles,
    std::vector<ParticleRendering>& rParticles)
{
    if (pParticles.size() != rParticles.size()) {
        throw std::runtime_error("Physics and rendering vectors must be of the same size.");
    }

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
