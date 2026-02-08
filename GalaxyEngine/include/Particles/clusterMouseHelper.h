#pragma once

#include "Physics/quadtree.h"

struct ClusterHelper {
	static void clusterMouseHelper(Camera3D& cam3D, float& dist) {

		Ray camRay = GetScreenToWorldRay(GetMousePosition(), cam3D);
		float minDistSq = std::numeric_limits<float>::max();
		int nearestIdx = -1;

		for (size_t i = 0; i < globalNodes3D.size(); i++) {
			Node3D& n = globalNodes3D[i];

			if (n.size > 16.0f || n.endIndex - n.startIndex < 16) {
				continue;
			}

			Vector3 bMin = { n.pos.x, n.pos.y, n.pos.z };
			Vector3 bMax = { n.pos.x + n.size, n.pos.y + n.size, n.pos.z + n.size };
			BoundingBox bBox = { bMin, bMax };
			RayCollision rayColl = GetRayCollisionBox(camRay, bBox);

			if (rayColl.hit) {

				glm::vec3 d = n.centerOfMass - glm::vec3{ camRay.position.x, camRay.position.y, camRay.position.z };
				float distSq = glm::dot(d, d);

				if (distSq < minDistSq) {
					minDistSq = distSq;
					nearestIdx = static_cast<int>(i);
				}
			}
		}

		if (nearestIdx != -1) {

			/*DrawCubeWiresV({ nearest.pos.x, nearest.pos.y, nearest.pos.z },
				{ nearest.size, nearest.size, nearest.size },
				{ 128, 255, 128, 128 });*/

			dist = std::sqrt(minDistSq);
		}
	}
};