#pragma once

#include "parameters.h"

struct QueryNeighbors {

	static std::vector<size_t> queryNeighbors(UpdateParameters& myParam, bool& hasAVX2, size_t reserveAmount, glm::vec2& pos) {
		std::vector<size_t> neighborIndices;
		neighborIndices.reserve(reserveAmount);

		if (!hasAVX2) {
			myParam.neighborSearchV2.queryNeighbors(
				pos,
				[&](uint32_t idx) {
					neighborIndices.push_back(idx);
				}
			);
		}
		else {
			myParam.neighborSearchV2AVX2.queryNeighborsAVX2(
				pos,
				neighborIndices
			);
		}

		return neighborIndices;
	}
};

struct QueryNeighbors3D {

	static std::vector<size_t> queryNeighbors3D(UpdateParameters& myParam, bool& hasAVX2, size_t reserveAmount, glm::vec3& pos) {
		std::vector<size_t> neighborIndices;
		neighborIndices.reserve(reserveAmount);

		if (!hasAVX2) {
			myParam.neighborSearch3DV2.queryNeighbors(
				pos,
				[&](uint32_t idx) {
					neighborIndices.push_back(idx);
				}
			);
		}
		else {
			myParam.neighborSearch3DV2AVX2.queryNeighborsAVX2(
				pos,
				neighborIndices
			);
		}

		return neighborIndices;
	}
};