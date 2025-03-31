#pragma once
#include <iostream>
#include <cmath>
#include <algorithm>
#include <vector>
#include <limits>
#include <array>
#include <memory>
#include "raylib.h"
#include "planet.h"

struct Range {
	size_t start;
	size_t end;

	size_t size() const { return end - start; }
};

struct Quad {
	Vector2 center;
	float size;

	Quad() : center({ 0, 0 }), size(0) {}

	Quad(const Vector2& center, float size);

	static Quad boundingBox(const std::vector<ParticlePhysics>& pParticles);

	Quad subQuadrant(size_t quadrant) const;

	std::array<Quad, 4> subdivide() const;
};


struct Node {
	size_t children;
	size_t nextNode;
	Vector2 pos;
	float mass;
	Quad quad;
	Range pParticles;

	Node(size_t nextNode, const Quad& quad, Range pParticles) {
		children = 0;
		this->nextNode = nextNode;
		pos = { 0.0f };
		mass = 0.0f;
		this->quad = quad;
		this->pParticles = pParticles;
	}

	bool isLeaf() const { return children == 0; }
	bool isBranch() const { return children != 0; }
	bool isEmpty() const { return mass == 0.0f; }
};

class NewQuadtree {
public:
	static const size_t root = 0;

	NewQuadtree(float theta, float epsilon, size_t leafCapacity, double G)
		: thetaSq(theta * theta), epsilonSq(epsilon * epsilon), leafCapacity(leafCapacity), G(G){ }

	void clear() {
		nodes.clear();
		parents.clear();
	}

	void buildTree(std::vector<ParticlePhysics>& pParticles);

	Vector2 calculateAcceleration(const Vector2& pos, const std::vector<ParticlePhysics>& pParticles);

	const std::vector<Node>& getNodes() const {
        return nodes;
    }

private:
	float thetaSq;
	float epsilonSq;
	double G;
	size_t leafCapacity;
	std::vector<Node> nodes;
	std::vector<size_t> parents;

	void subdivide(size_t nodeIdx, std::vector<ParticlePhysics>& pParticles);

	void calculateLeafMass(Node& node, const std::vector<ParticlePhysics>& pParticles);

	void propagate();
};
