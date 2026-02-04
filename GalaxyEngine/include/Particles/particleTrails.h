#pragma once

#include "Particles/particle.h"

struct UpdateVariables;
struct UpdateParameters;


class ParticleTrails {
public:
	float size = 5;
	float trailThickness = 0.5f;
	bool whiteTrails = false;
	struct Segment {
		glm::vec2 start;
		glm::vec2 end;
		glm::vec2 offset;
		glm::vec2 prevOffset;
		Color color;
	};
	struct Segment3D {
		glm::vec3 start;
		glm::vec3 end;
		glm::vec3 offset;
		glm::vec3 prevOffset;
		Color color;
	};
	std::vector<Segment> segments;
	std::vector<Segment3D> segments3D;
	glm::vec2 selectedParticlesAveragePos = { 0.0f, 0.0f };
	glm::vec2 selectedParticlesAveragePrevPos = { 0.0f, 0.0f };
	glm::vec3 selectedParticlesAveragePos3D = { 0.0f, 0.0f, 0.0f };
	glm::vec3 selectedParticlesAveragePrevPos3D = { 0.0f, 0.0f, 0.0f };
	void trailLogic(UpdateVariables& myVar, UpdateParameters& myParam);
	void drawTrail(std::vector<ParticleRendering>& rParticles, Texture2D& particleBlur);
	void trailLogic3D(UpdateVariables& myVar, UpdateParameters& myParam);
	void drawTrail3D(std::vector<ParticleRendering3D>& rParticles3D, Texture2D& particleBlur, Camera3D& cam3D);
private:
	bool wasLocalTrailsEnabled = false;
};