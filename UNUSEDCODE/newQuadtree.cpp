#include "newQuadtree.h"

Quad::Quad(const Vector2& center, float size) {
	this->center = center;
	this->size = size;
}

Quad Quad::boundingBox(const std::vector<ParticlePhysics>& pParticles) {

	float min_x = std::numeric_limits<float>::max();
	float min_y = std::numeric_limits<float>::max();
	float max_x = std::numeric_limits<float>::lowest();
	float max_y = std::numeric_limits<float>::lowest();

	for (const auto& particle : pParticles) {
		min_x = std::min(min_x, particle.pos.x);
		min_y = std::min(min_y, particle.pos.y);
		max_x = std::max(max_x, particle.pos.x);
		max_y = std::max(max_y, particle.pos.y);
	}

	float size = std::max(max_x - min_x, max_y - min_y);

	Vector2 center = { (min_x + max_x) * 0.5f, (min_y + max_y) * 0.5f };

	DrawRectangleLines(center.x - size / 2, center.y - size / 2, static_cast<int>(size), static_cast<int>(size), WHITE);

	return Quad(center, size);
}

Quad Quad::subQuadrant(size_t quadrant) const {
	float halfSize = size * 0.5f;
	float offsetX = ((quadrant & 1) ? 0.5f : -0.5f) * halfSize;
	float offsetY = ((quadrant & 2) ? 0.5f : -0.5f) * halfSize;
	return Quad(Vector2{center.x + offsetX, center.y + offsetY}, halfSize);
}

std::array<Quad, 4> Quad::subdivide() const {
	return { subQuadrant(0), subQuadrant(1), subQuadrant(2), subQuadrant(3) };
}

// PUBLIC QUADTREE METHOD DEFINITIONS
void NewQuadtree::buildTree(std::vector<ParticlePhysics>& pParticles) {
	clear();
	if (pParticles.empty()) return;

	Quad rootQuad = Quad::boundingBox(pParticles);
	nodes.emplace_back(0, rootQuad, Range{ 0, pParticles.size() });

	size_t nodeIdx = 0;
	while (nodeIdx < nodes.size()) {
		Node& node = nodes[nodeIdx];
		if (node.pParticles.size() > leafCapacity) {
			subdivide(nodeIdx, pParticles);
		}
		else {
			calculateLeafMass(node, pParticles);
		}
		nodeIdx++;
	}
	propagate();
}

Vector2 NewQuadtree::calculateAcceleration(const Vector2& pos, const std::vector<ParticlePhysics>& pParticles) {
	Vector2 acceleration = { 0.0f };
	size_t current = root;

	while (true) {
		const Node& node = nodes[current];
		Vector2 axisDistance = { node.pos.x - pos.x, node.pos.y - pos.y };
		float distanceSq = axisDistance.x * axisDistance.x + axisDistance.y * axisDistance.y;

		if (node.isBranch() && (node.quad.size * node.quad.size < thetaSq * distanceSq)) {
			float denom = (distanceSq + epsilonSq) * sqrt(distanceSq);
			if (denom > 0.0f) {
				acceleration.x += axisDistance.x * (node.mass / denom) * G;
				acceleration.y += axisDistance.y * (node.mass / denom) * G;
			}
			if (node.nextNode) {
				current = node.nextNode;
			}
			else {
				break;
			}
		}
		else if (node.isLeaf()) {
			for (size_t i = node.pParticles.start; i < node.pParticles.end; ++i) {
				const ParticlePhysics& pParticle = pParticles[i];
				Vector2 dParticle = { pParticle.pos.x - pos.x, pParticle.pos.y - pos.y };
				float dParticleSq = dParticle.x * dParticle.x + dParticle.y * dParticle.y;

				float denom = (dParticleSq + epsilonSq) * sqrt(dParticleSq);
				if (denom > 0.0f) {
					acceleration.x += dParticle.x * (pParticle.mass / denom) * G;
					acceleration.y += dParticle.y * (pParticle.mass / denom) * G;
				}
			}
			if (node.nextNode == 0) {
				current = node.children;
			}
			else {
				break;
			}
		}
		else {
			current = node.children;
		}
		if (current == 0) break;

	}
	return acceleration;
}



// PRIVATE QUADTREE METHOD DEFINITIONS
void NewQuadtree::subdivide(size_t nodeIdx, std::vector<ParticlePhysics>& pParticles) {
	Node& node = nodes[nodeIdx];
	Vector2 center = node.quad.center;

	const size_t particlesStart = node.pParticles.start;
	const size_t particlesEnd = node.pParticles.end;

	if (particlesStart >= particlesEnd || particlesEnd > pParticles.size()) {
		return;
	}

	auto start = pParticles.begin() + particlesStart;
	auto end = pParticles.begin() + particlesEnd;

	auto midY = std::partition(start, end, [&center](const ParticlePhysics& pParticle) {
		return pParticle.pos.y < center.y;
		});
	size_t split2 = midY - pParticles.begin();

	auto midLowerX = std::partition(start, midY, [&center](const ParticlePhysics& pParticle) {
		return pParticle.pos.x < center.x;
		});
	size_t split1 = midLowerX - pParticles.begin();

	auto midUpperX = std::partition(midY, end, [&center](const ParticlePhysics& pParticle) {
		return pParticle.pos.x < center.x;
		});
	size_t split3 = midUpperX - pParticles.begin();

	split1 = std::clamp(split1, particlesStart, split2);
	split2 = std::clamp(split2, split1, split3);
	split3 = std::clamp(split3, split2, particlesEnd);

	parents.push_back(nodeIdx);
	node.children = nodes.size();

	std::array<Quad, 4> quads = node.quad.subdivide();
	size_t nexts[4] = { node.children + 1, node.children + 2, node.children + 3, node.nextNode };

	nodes.emplace_back(nexts[0], quads[0], Range{ particlesStart, split1 });
	nodes.emplace_back(nexts[1], quads[1], Range{ split1, split2 });
	nodes.emplace_back(nexts[2], quads[2], Range{ split2, split3 });
	nodes.emplace_back(nexts[3], quads[3], Range{ split3, particlesEnd });

	node.nextNode = node.children;
}

void NewQuadtree::calculateLeafMass(Node& node, const std::vector<ParticlePhysics>& pParticles) {
	node.mass = 0.0f;
	node.pos = { 0.0f };

	for (size_t i = node.pParticles.start; i < node.pParticles.end; ++i) {
		node.pos.x += pParticles[i].pos.x * pParticles[i].mass;
		node.pos.y += pParticles[i].pos.y * pParticles[i].mass;
		node.mass += pParticles[i].mass;
	}
}

void NewQuadtree::propagate() {
	for (auto it = parents.rbegin(); it != parents.rend(); ++it) {
		Node& node = nodes[*it];
		node.mass = 0.0f;
		node.pos = { 0.0f };

		for (size_t i = 0; i < 4; ++i) {
			const Node& child = nodes[node.children + i];
			node.mass += child.mass;
			node.pos.x += child.pos.x * child.mass;
			node.pos.y += child.pos.y * child.mass;
		}
		if (node.mass > 0.0f) {
			node.pos.x /= node.mass;
			node.pos.y /= node.mass;
		}
	}
}
