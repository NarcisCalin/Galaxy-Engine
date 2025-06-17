#pragma once

struct ParticleConstraint {
	uint32_t id1;
	uint32_t id2;
	float restLength;
	float originalLength;
	float stiffness;
	float resistance;
	float displacement;
	float plasticityPoint;
	bool isBroken;
	bool isPlastic;
};