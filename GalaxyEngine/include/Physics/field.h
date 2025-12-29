#pragma once

#include "parameters.h"

struct Cell {
	glm::vec2 pos;
	float size;
	Color col = { 5,5,5,255 };
	bool isActive = false;
	float strength = 0.0f;

	void drawCell() {

		DrawRectangleV({ pos.x, pos.y }, { size, size }, col);
	}
};

struct Field {

	int res = 150;

	int amountX = res;
	int amountY = res;

	float cellSize = 10.0f;

	float gravityDisplayThreshold = 1000.0f;

	bool computeField = true;

	float gravityDisplaySoftness = 0.85f;

	std::vector<Cell> cells;

	glm::vec2 prevDomainSize = { 0.0f, 0.0f };

	int prevRes = 150;

	void initializeCells(UpdateVariables& myVar) {

		if (prevDomainSize != myVar.domainSize) {
			computeField = true;

			prevDomainSize = myVar.domainSize;
		}

		if (prevRes != res) {
			computeField = true;

			prevRes = res;
		}

		if (computeField) {

			if (myVar.domainSize.x >= myVar.domainSize.y) {
				cellSize = myVar.domainSize.y / static_cast<float>(res);
			}
			else {
				cellSize = myVar.domainSize.x / static_cast<float>(res);
			}

			amountX = static_cast<int>(myVar.domainSize.x / cellSize);
			amountY = static_cast<int>(myVar.domainSize.y / cellSize);

			cells.clear();
			cells.resize(amountX * amountY);

			for (int i = 0; i < amountX; i++) {
				for (int j = 0; j < amountY; j++) {

					int index = j + i * amountY;

					cells[index].size = cellSize;
					cells[index].pos.x = i * cellSize;
					cells[index].pos.y = j * cellSize;
				}
			}

			computeField = false;
		}
	}

	//void fieldLogic(UpdateParameters& myParam, UpdateVariables& myVar) {

//#pragma omp parallel for schedule(dynamic)
//		for (size_t i = 0; i < cells.size(); i++) {
//
//			float force = 0.0f;
//
//			for (size_t j = 0; j < myParam.pParticles.size(); j++) {
//
//				glm::vec2 d = myParam.pParticles[j].pos - (cells[i].pos + cells[i].size * 0.5f);
//				float distSq = d.x * d.x + d.y * d.y + myVar.softening * myVar.softening;
//
//				float dist = glm::sqrt(distSq);
//
//				if (dist > 0.0001f) {
//					force += myVar.G * (myParam.pParticles[j].mass * 10.0f) / (dist * dist);
//				}
//			}
//
//			cells[i].strength = force;
//		}
	//}

	const char* fieldGravityCompute = R"(
#version 430

layout(local_size_x = 256) in;

layout(std430, binding = 7) buffer ParticlesPos {
    float particlesPos[];
};

layout(std430, binding = 8) buffer ParticlesMass {
    float particlesMass[];
};

layout(std430, binding = 9) buffer CellsData {
    float cellsData[];
};

uniform int particleCount;
uniform int cellCount;

uniform float G;
uniform float gravityDisplaySoftness;

uniform vec2 domainSize;
uniform int periodicBoundary;

void main() {

    uint i = gl_GlobalInvocationID.x;

    if (i >= uint(cellCount))
        return;

    vec2 cellPos;
    cellPos.x = cellsData[i];
    cellPos.y = cellsData[i + cellCount];

    float force = 0.0;

    for (int j = 0; j < particleCount; j++) {

        vec2 pPos;
        pPos.x = particlesPos[j];
        pPos.y = particlesPos[j + particleCount];

        vec2 d = pPos - cellPos;

        if (periodicBoundary == 1) {
            d -= domainSize * round(d / domainSize);
        }

        float distSq = dot(d, d) + gravityDisplaySoftness * gravityDisplaySoftness;
        float dist = sqrt(distSq);

        if (dist > 0.0001) {
            force += G * (particlesMass[j] * 10.0) / (dist * dist);
        }
    }

    cellsData[i + 2 * cellCount] = force;
}
)";


	const char* fieldFragmentShader = R"(
#version 430

layout(std430, binding = 9) buffer CellsData {
    float cellsData[];
};

uniform int amountX;
uniform int amountY;
uniform float cellSize;
uniform float gravityDisplayThreshold;
uniform float stretchFactor;
uniform float exposure; 

uniform bool useTwoColorMode;
uniform vec3 colorLow;
uniform vec3 colorHigh;

in vec2 fragPosition;
out vec4 finalColor;

vec3 hsv2rgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0/3.0, 1.0/3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main() {
    int cellX = int(fragPosition.x / cellSize);
    int cellY = int(fragPosition.y / cellSize);

    if (cellX < 0 || cellX >= amountX || cellY < 0 || cellY >= amountY)
        discard;

    int cellIndex = cellY + cellX * amountY;
    float strength = cellsData[cellIndex + 2 * amountX * amountY];

    float clamped = clamp(strength, 0.0, gravityDisplayThreshold);
    float normalized = clamped / gravityDisplayThreshold;
    float stretched = log(1.0 + normalized * stretchFactor) / log(1.0 + stretchFactor);

    vec3 color;

    if (useTwoColorMode) {
        vec3 base = mix(colorLow, colorHigh, stretched);
        color = base * stretched; 

        color *= exposure; 
    } else {
        float hue = (1.0 - stretched) * 240.0;
        color = hsv2rgb(vec3(hue / 360.0, 1.0, stretched));
    }

    finalColor = vec4(color, 1.0);
}
)";

	GLuint ssboParticlesPos, ssboParticlesMass, ssboCellsData;
	GLuint gravityDisplayProgram;

	const char* fieldVertexShader = R"(
#version 430
in vec3 vertexPosition;
in vec2 vertexTexCoord;

uniform mat4 mvp;

out vec2 fragPosition;

void main()
{
    fragPosition = vertexPosition.xy; 
    gl_Position = mvp * vec4(vertexPosition, 1.0);
}
)";

	Shader drawShader;


	void fieldGravityDisplayKernel() {

		drawShader = LoadShaderFromMemory(fieldVertexShader, fieldFragmentShader);

		if (drawShader.id == 0) {
			std::cout << "Fragment shader failed to load\n";
		}

		const size_t maxVRAM = size_t(512) * 1024 * 1024;

		const size_t usableVRAM = size_t(maxVRAM * 0.8f);

		const size_t bytesPerParticle = 6 * sizeof(float);
		const size_t maxParticles = usableVRAM / bytesPerParticle;

		glGenBuffers(1, &ssboParticlesPos);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboParticlesPos);
		glBufferData(GL_SHADER_STORAGE_BUFFER,
			maxParticles * 2 * sizeof(float),
			nullptr,
			GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, ssboParticlesPos);

		glGenBuffers(1, &ssboParticlesMass);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboParticlesMass);
		glBufferData(GL_SHADER_STORAGE_BUFFER,
			maxParticles * sizeof(float),
			nullptr,
			GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, ssboParticlesMass);

		glGenBuffers(1, &ssboCellsData);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboCellsData);
		glBufferData(GL_SHADER_STORAGE_BUFFER,
			maxParticles * 3 * sizeof(float),
			nullptr,
			GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, ssboCellsData);

		gravityDisplayProgram = glCreateProgram();
		GLuint shader = glCreateShader(GL_COMPUTE_SHADER);
		glShaderSource(shader, 1, &fieldGravityCompute, nullptr);
		glCompileShader(shader);

		GLint success;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success) {
			char log[1024];
			glGetShaderInfoLog(shader, 1024, nullptr, log);
			std::cerr << log << std::endl;
		}

		glAttachShader(gravityDisplayProgram, shader);
		glLinkProgram(gravityDisplayProgram);

		glGetProgramiv(gravityDisplayProgram, GL_LINK_STATUS, &success);
		if (!success) {
			char log[1024];
			glGetProgramInfoLog(gravityDisplayProgram, 1024, nullptr, log);
			std::cerr << log << std::endl;
		}

		glDeleteShader(shader);
	}

	std::vector<float> particlesData;
	std::vector<float> particlesMassVector;
	std::vector<float> cellsData;

	void gpuGravityDisplay(UpdateParameters& myParam, UpdateVariables& myVar) {

		if (myParam.pParticles.empty() || !myVar.isGravityFieldEnabled) {
			return;
		}

		size_t particleCount = myParam.pParticles.size();

		if (!myVar.gravityFieldDMParticles) {

			particleCount = 0;

			for (size_t i = 0; i < myParam.pParticles.size(); i++) {
				if (myParam.rParticles[i].isDarkMatter) {
					continue;
				}
				particleCount++;
			}
		}

		size_t cellCount = cells.size();

		particlesData.resize(particleCount * 2);
		particlesMassVector.resize(particleCount);
		cellsData.resize(cellCount * 3);

		size_t writeIndex = 0;

		for (size_t i = 0; i < myParam.pParticles.size(); i++) {

			if (!myVar.gravityFieldDMParticles && myParam.rParticles[i].isDarkMatter) {
				continue;
			}

			particlesData[writeIndex] = myParam.pParticles[i].pos.x;
			particlesData[writeIndex + particleCount] = myParam.pParticles[i].pos.y;
			particlesMassVector[writeIndex] = myParam.pParticles[i].mass;

			writeIndex++;
		}

		for (size_t i = 0; i < cellCount; i++) {
			cellsData[i] = cells[i].pos.x;
			cellsData[i + cellCount] = cells[i].pos.y;
			cellsData[i + 2 * cellCount] = 0.0f;
		}

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboParticlesPos);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0,
			particlesData.size() * sizeof(float),
			particlesData.data());

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboParticlesMass);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0,
			particlesMassVector.size() * sizeof(float),
			particlesMassVector.data());

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboCellsData);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0,
			cellsData.size() * sizeof(float),
			cellsData.data());

		glUseProgram(gravityDisplayProgram);

		glUniform1i(glGetUniformLocation(gravityDisplayProgram, "particleCount"),
			(int)particleCount);
		glUniform1i(glGetUniformLocation(gravityDisplayProgram, "cellCount"),
			(int)cellCount);

		glUniform2f(glGetUniformLocation(gravityDisplayProgram, "domainSize"),
			myVar.domainSize.x, myVar.domainSize.y);
		glUniform1f(glGetUniformLocation(gravityDisplayProgram, "gravityDisplaySoftness"),
			gravityDisplaySoftness);
		glUniform1i(glGetUniformLocation(gravityDisplayProgram, "periodicBoundary"),
			myVar.isPeriodicBoundaryEnabled);

		glUniform1f(glGetUniformLocation(gravityDisplayProgram, "G"), myVar.G);

		GLuint groups = (cellCount + 255) / 256;
		glDispatchCompute(groups, 1, 1);

		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboCellsData);
		float* ptr = (float*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);

#pragma omp parallel for
		for (size_t i = 0; i < cellCount; i++) {
			cells[i].strength = ptr[i + 2 * cellCount];
		}

		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	}

	float hue = 180.0f;
	float saturation = 0.8f;
	float value = 0.5f;

	float gravityStretchFactor = 90.0f;

	bool gravityCustomColors = false;

	float gravityExposure = 3.0f;

	void drawField(UpdateParameters& myParam, UpdateVariables& myVar)
	{
		BeginMode2D(myParam.myCamera.camera);
		BeginShaderMode(drawShader);

		int exposureLoc = GetShaderLocation(drawShader, "exposure");

		int amtXLoc = GetShaderLocation(drawShader, "amountX");
		int amtYLoc = GetShaderLocation(drawShader, "amountY");
		int sizeLoc = GetShaderLocation(drawShader, "cellSize");
		int threshLoc = GetShaderLocation(drawShader, "gravityDisplayThreshold");
		int stretchLoc = GetShaderLocation(drawShader, "stretchFactor");

		int lowLoc = GetShaderLocation(drawShader, "colorLow");
		int highLoc = GetShaderLocation(drawShader, "colorHigh");

		float lowColor[3] = {
			static_cast<float>(myParam.colorVisuals.pColor.r / 255.0f),
			static_cast<float>(myParam.colorVisuals.pColor.g / 255.0f),
			static_cast<float>(myParam.colorVisuals.pColor.b / 255.0f)
		};

		float highColor[3] = {
			static_cast<float>(myParam.colorVisuals.sColor.r / 255.0f),
			static_cast<float>(myParam.colorVisuals.sColor.g / 255.0f),
			static_cast<float>(myParam.colorVisuals.sColor.b / 255.0f)
		};

		SetShaderValue(drawShader, exposureLoc, &gravityExposure, SHADER_UNIFORM_FLOAT);

		SetShaderValue(drawShader, lowLoc, lowColor, SHADER_UNIFORM_VEC3);
		SetShaderValue(drawShader, highLoc, highColor, SHADER_UNIFORM_VEC3);

		int toggleLoc = GetShaderLocation(drawShader, "useTwoColorMode");
		int shaderBool = gravityCustomColors ? 1 : 0;
		SetShaderValue(drawShader, toggleLoc, &shaderBool, SHADER_UNIFORM_INT);

		SetShaderValue(drawShader, amtXLoc, &amountX, SHADER_UNIFORM_INT);
		SetShaderValue(drawShader, amtYLoc, &amountY, SHADER_UNIFORM_INT);
		SetShaderValue(drawShader, sizeLoc, &cellSize, SHADER_UNIFORM_FLOAT);
		SetShaderValue(drawShader, threshLoc, &gravityDisplayThreshold, SHADER_UNIFORM_FLOAT);
		SetShaderValue(drawShader, stretchLoc, &gravityStretchFactor, SHADER_UNIFORM_FLOAT);

		DrawRectangleV({ 0.0f, 0.0f }, { myVar.domainSize.x, myVar.domainSize.y }, WHITE);

		EndShaderMode();
		EndMode2D();
	}
};