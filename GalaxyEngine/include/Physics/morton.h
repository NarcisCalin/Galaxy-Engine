#pragma once

#include "Particles/particle.h"

struct Morton {

	std::vector<size_t> indicesBuffer;
	std::vector<ParticlePhysics> pSortedBuffer;
	std::vector<ParticleRendering> rSortedBuffer;

	uint64_t scaleToGrid(float pos, float minVal, float maxVal = 2097151.0f);

	uint64_t spreadBits(uint64_t x);

	uint64_t morton2D(uint64_t x, uint64_t y);

	void computeMortonKeys(std::vector<ParticlePhysics>& pParticles, glm::vec3& posSize);

	void sortParticlesByMortonKey(std::vector<ParticlePhysics>& pParticles,
		std::vector<ParticleRendering>& rParticles);



    std::vector<size_t> indicesBuffer3D;
    std::vector<ParticlePhysics3D> pSortedBuffer3D;
    std::vector<ParticleRendering3D> rSortedBuffer3D;

    uint64_t scaleToGrid3D(float pos, float minVal, float maxVal = 2097151.0f);

    uint64_t spreadBits3D(uint64_t x);

    uint64_t morton3D(uint64_t x, uint64_t y, uint64_t z);

    void computeMortonKeys3D(std::vector<ParticlePhysics3D>& pParticles,
        const glm::vec4& boundingBox);

    void sortParticlesByMortonKey3D(std::vector<ParticlePhysics3D>& pParticles,
        std::vector<ParticleRendering3D>& rParticles);

};