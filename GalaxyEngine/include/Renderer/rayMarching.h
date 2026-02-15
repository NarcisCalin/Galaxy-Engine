#pragma once

#include "Particles/particle.h"
#include "UX/camera.h"
#include "UI/UI.h"
#include "Physics/quadtree.h"

struct RayMarcher {

	struct Pixel {
		int x;
		int y;
		int size;
		Color color;
	};

	int screenResX = 480;
	int screenResY = 270;

	int pixelSize = 3;

	int pixelAmountX = screenResX / pixelSize;
	int pixelAmountY = screenResY / pixelSize;

	float aspectRatio = static_cast<float>(pixelAmountX) / static_cast<float>(pixelAmountY);

	std::vector<Pixel> pixels;

	bool initPixels = true;

	void initPixelGrid() {

		if (initPixels) {

			pixelAmountX = screenResX / pixelSize;
			pixelAmountY = screenResY / pixelSize;

			aspectRatio = static_cast<float>(pixelAmountX) / static_cast<float>(pixelAmountY);

			for (int y = 0; y < pixelAmountY; y++) {
				for (int x = 0; x < pixelAmountX; x++) {

					pixels.emplace_back(Pixel{ x * pixelSize, y * pixelSize, pixelSize , Color{ 255, 0, 0, 255 } });
				}
			}

			initPixels = false;
		}
	}

	void drawPixels() {
		for (Pixel& p : pixels) {

			DrawRectangle(p.x, p.y, p.size, p.size, { p.color.r,p.color.g, p.color.b, p.color.a });
		}
	}

	struct MarcherRay {
		glm::vec3 source;
		glm::vec3 dir;
		float totalLength = FLT_MAX;
		float minLength = FLT_MAX;
		int steps = 0;
		bool hit = false;
	};

	std::vector<MarcherRay> rays;

	float camSizeMult = 1.0f;

	float collisionDistMultiplier = 1.0f;
	int marchingSteps = 100;

	float fov = 45.0f;

	float nearPlane = 0.0001f;

	float farPlane = 1000.0f;

	float testThreshold = 10.0f;

	void createCameraRay(glm::vec3 dir, glm::vec3 source) {
		rays.emplace_back(source, dir);
	}

	float smoothingValue = 5.5f;

	float smoothMin(float a, float b, float k) {
		float h = glm::clamp(0.5f + 0.5f * (b - a) / k, 0.0f, 1.0f);
		return glm::mix(b, a, h) - k * h * (1.0f - h);
	}

	void rayParticleLogic(std::vector<ParticlePhysics3D>& pParticles, std::vector<ParticleRendering3D>& rParticles, MarcherRay& ray) {

		glm::vec3 initSource = ray.source;

		if (pParticles.empty()) return;

		float minSceneDist = FLT_MAX;
		float minRayLength = FLT_MAX;

		for (int iter = 0; iter < marchingSteps; iter++) {

			minSceneDist = FLT_MAX;

			for (size_t i = 0; i < pParticles.size(); i++) {
				ParticlePhysics3D& p = pParticles[i];
				ParticleRendering3D& r = rParticles[i];

				float centerDist = glm::distance(ray.source, p.pos);

				float surfaceDist = centerDist - (r.totalRadius * collisionDistMultiplier);

				minSceneDist = smoothMin(minSceneDist, surfaceDist, smoothingValue);

				minRayLength = smoothMin(minRayLength, surfaceDist, smoothingValue);;

			}

			if (minSceneDist < nearPlane) {
				ray.hit = true;
				ray.totalLength = glm::distance(initSource, ray.source);
				ray.minLength = minSceneDist;
				ray.steps = iter;
				return;
			}

			ray.source += ray.dir * minSceneDist;

			if (minSceneDist > farPlane) {
				ray.totalLength = glm::distance(initSource, ray.source);
				ray.minLength = minRayLength;
				ray.steps = marchingSteps;

				return;
			}
		}
	}

	float de(glm::vec3 p0) {
		glm::vec4 p(p0, 1.0f);

		for (int i = 0; i < 8; i++) {
			p.x = std::fmod(p.x - 1.0f, 2.0f);
			if (p.x < 0.0f) p.x += 2.0f;
			p.x -= 1.0f;

			p.y = std::fmod(p.y - 1.0f, 2.0f);
			if (p.y < 0.0f) p.y += 2.0f;
			p.y -= 1.0f;

			p.z = std::fmod(p.z - 1.0f, 2.0f);
			if (p.z < 0.0f) p.z += 2.0f;
			p.z -= 1.0f;

			float scale = 1.4f / glm::dot(glm::vec3(p), glm::vec3(p));
			p *= scale;
		}

		glm::vec2 xz(p.x / p.w, p.z / p.w);
		return glm::length(xz) * 0.25f;
	}

	void fractal(MarcherRay& ray) {
		glm::vec3 initSource = ray.source;
		float minRayLength = FLT_MAX;

		for (int iter = 0; iter < marchingSteps; iter++) {

			float minSceneDist = de(ray.source);

			minRayLength = std::min(minRayLength, minSceneDist);

			if (minSceneDist < nearPlane) {
				ray.hit = true;
				ray.totalLength = glm::distance(initSource, ray.source);
				ray.minLength = minSceneDist;
				ray.steps = iter;
				return;
			}

			ray.source += ray.dir * minSceneDist;

			if (glm::distance(initSource, ray.source) > farPlane) {
				ray.totalLength = glm::distance(initSource, ray.source);
				ray.minLength = minRayLength;
				ray.steps = marchingSteps;
				return;
			}
		}

		ray.totalLength = glm::distance(initSource, ray.source);
		ray.minLength = minRayLength;
		ray.steps = marchingSteps;
	}

	void rayMarchUI() {

		bool enabled = true;

		UI::sliderHelper("Collision Distance", "", collisionDistMultiplier, 0.1f, 100.0f, 200.0f, 50.0f, enabled, UI::LogSlider);
		UI::sliderHelper("Marching Steps", "", marchingSteps, 1, 200, 200.0f, 50.0f, enabled);

		UI::sliderHelper("Test Threshold", "", testThreshold, 0.001f, 100.0f, 200.0f, 50.0f, enabled, UI::LogSlider);
		UI::sliderHelper("Smooth Value", "", smoothingValue, 0.0f, 15.0f, 200.0f, 50.0f, enabled, UI::LogSlider);

		UI::sliderHelper("Near Plane", "", nearPlane, 0.001f, 1.0f, 200.0f, 50.0f, enabled, UI::LogSlider);
		UI::sliderHelper("Far Plane", "", farPlane, 100.0f, 100000.0f, 200.0f, 50.0f, enabled, UI::LogSlider);
	}

	void cameraRays(SceneCamera3D& cam, std::vector<ParticlePhysics3D>& pParticles, std::vector<ParticleRendering3D>& rParticles) {

		rays.clear();

		float halfFovRadians = glm::radians(fov) * 0.5f;
		float verticalScale = tan(halfFovRadians) * 2.0f;

		for (int y = 0; y < pixelAmountY; y++) {
			for (int x = 0; x < pixelAmountX; x++) {

				float offsetX = (static_cast<float>(x) / static_cast<float>(pixelAmountX) - 0.5f) * aspectRatio;
				float offsetY = (static_cast<float>(y) / static_cast<float>(pixelAmountY) - 0.5f);

				glm::vec3 dir = -cam.camNormal;

				dir += cam.camRight * offsetX * verticalScale;

				dir -= cam.camUp * offsetY * verticalScale;

				dir = glm::normalize(dir);

				glm::vec3 rayOrigin = { cam.cam3D.position.x, cam.cam3D.position.y, cam.cam3D.position.z };

				createCameraRay(dir, rayOrigin);
			}
		}

#pragma omp parallel for schedule(dynamic)
		for (size_t i = 0; i < rays.size(); i++) {

			MarcherRay& ray = rays[i];

			rayParticleLogic(pParticles, rParticles, ray);
			fractal(ray);
		}

		for (size_t i = 0; i < pixels.size(); i++) {

			Pixel& p = pixels[i];
			MarcherRay& r = rays[i];

			if (true) {
				unsigned char value = static_cast<unsigned char>((float(std::min(float(r.steps), abs(testThreshold))) / testThreshold) * 255.0f);

				p.color = { value,value,value, 255 };
			}
			else {
				p.color = { 0,0,0,200 };
			}
		}
	}

	const char* rayMarcherCompute = R"(
#version 430 core
layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
layout(rgba16f, binding = 0) uniform image2D imgOutput;

uniform vec3 camPos;
uniform vec3 camDir;
uniform vec3 camRight;
uniform vec3 camUp;
uniform float fov;
uniform int maxSteps;
uniform float collisionMult;
uniform float testThreshold;
uniform float smoothVal;
uniform float nearPlane;
uniform float farPlane;
uniform vec2 resolution;

struct Particle {
    vec3 pos;
    float radius;
};

layout(std430, binding = 1) buffer ParticleBuffer {
    Particle particles[];
};

float smoothMin(float a, float b, float k) {
    float h = clamp(0.5 + 0.5 * (b - a) / k, 0.0, 1.0);
    return mix(b, a, h) - k * h * (1.0 - h);
}

  float de( vec3 p0 ){
    vec4 p = vec4(p0, 1.);
    for(int i = 0; i < 8; i++){
      p.xyz = mod(p.xyz-1.,2.)-1.;
      p*=1.4/dot(p.xyz,p.xyz);
    }
    return (length(p.xz/p.w)*0.25);
  }

vec3 calcNormal(vec3 p) {
    const float h = 0.0001;
    return normalize(vec3(
        de(vec3(p.x + h, p.y, p.z)) - de(vec3(p.x - h, p.y, p.z)),
        de(vec3(p.x, p.y + h, p.z)) - de(vec3(p.x, p.y - h, p.z)),
        de(vec3(p.x, p.y, p.z + h)) - de(vec3(p.x, p.y, p.z - h))
    ));
}

struct RayResult {
    vec3 color;
    vec3 hitPos;
    vec3 hitNormal;
    int steps;
    float minDist;
    float maxDist;
    bool hit;
};

RayResult rayParticle(vec3 origin, vec3 direction) {
    RayResult result;
    result.hit = false;
    result.steps = maxSteps;
    
    vec3 currentPos = origin;
    float totalDist = 0.0;
    float minDist = 1000000.0f;
    float maxDist = 0.0f;
    
    for (int i = 0; i < maxSteps; i++) {
        float sceneDist = 1000000.0f;
        
        for (int p = 0; p < particles.length(); p++) {
            float distToParticle = distance(currentPos, particles[p].pos);
            float surfaceDist = distToParticle - (particles[p].radius * collisionMult);

            sceneDist = smoothMin(sceneDist, surfaceDist, smoothVal);
        }

        minDist = min(minDist, sceneDist);
        maxDist = max(maxDist, sceneDist);

        if (sceneDist < nearPlane) {
            result.hit = true;
            result.hitPos = currentPos;
            result.hitNormal = calcNormal(currentPos);
            result.steps = i;
            result.minDist = minDist;
            result.maxDist = maxDist;
            break;
        }

        totalDist += sceneDist;
        currentPos += direction * sceneDist;

        if (totalDist > farPlane) {
            result.steps = maxSteps;
            result.minDist = minDist;
            result.maxDist = maxDist;
            break;
        }
    }
    
    if (!result.hit && totalDist <= farPlane) {
        result.minDist = minDist;
        result.maxDist = maxDist;
    }
    
    float intensity = float(result.steps) / abs(testThreshold);
    if (testThreshold < 0.0){
        intensity = 1.0 - intensity;
    }
    result.color = vec3(intensity);
    
    return result;
}

RayResult fractal(vec3 origin, vec3 direction) {
    RayResult result;
    result.hit = false;
    result.steps = maxSteps;
    
    vec3 currentPos = origin;
    float totalDist = 0.0;
    float minDist = 1000000.0f;
    float maxDist = 0.0f;
    
    for (int i = 0; i < maxSteps; i++) {
        float dist = de(currentPos);
        
        minDist = min(minDist, dist);
        maxDist = max(maxDist, dist);
        
        if (dist < nearPlane) {
            result.hit = true;
            result.hitPos = currentPos;
            result.hitNormal = calcNormal(currentPos);
            result.steps = i;
            result.minDist = minDist;
            result.maxDist = maxDist;
            break;
        }
        
        totalDist += dist;
        currentPos += direction * dist;
        
        if (totalDist > farPlane) {
            result.steps = maxSteps;
            result.minDist = minDist;
            result.maxDist = maxDist;
            break;
        }
    }
    if (!result.hit && totalDist <= farPlane) {
        result.minDist = minDist;
        result.maxDist = maxDist;
    }
    
    float intensity = float(result.steps) / abs(testThreshold);
    if (testThreshold < 0.0){
        intensity = 1.0 - intensity;
    }
    result.color = vec3(intensity);
    
    return result;
}

void main() {
    ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
    ivec2 dims = imageSize(imgOutput);
    if (pixel_coords.x >= dims.x || pixel_coords.y >= dims.y) return;
    
    float aspectRatio = resolution.x / resolution.y;
    vec2 uv = (vec2(pixel_coords) / resolution) * 2.0 - 1.0;
    
    float halfFovRadians = radians(fov) * 0.5;
    float verticalScale = tan(halfFovRadians) * 2.0;
    float offsetX = uv.x * 0.5 * aspectRatio * verticalScale;
    float offsetY = uv.y * 0.5 * verticalScale;
    
    vec3 rayDir = normalize(-camDir + (camRight * offsetX) + (camUp * offsetY));

    vec3 finalColor = vec3(0.0);
    vec3 currentOrigin = camPos;
    vec3 currentDir = rayDir;
    
    RayResult result = rayParticle(currentOrigin, currentDir);
    
    if (true) {
        finalColor += result.color;

    } else {
        finalColor = vec3(0.0f);
    }
    
    vec4 color = vec4(finalColor, 1.0);
    imageStore(imgOutput, pixel_coords, color);
}
)";


	GLuint rayMarcherProgram;
	GLuint particleSSBO;
	GLuint highResTexID;
	GLuint lowResTexID;
	Texture2D highResTextureRayMarcher;
	Texture2D lowResTextureRayMarcher;

	void createShaderProgram() {
		GLuint shader = glCreateShader(GL_COMPUTE_SHADER);
		glShaderSource(shader, 1, &rayMarcherCompute, NULL);
		glCompileShader(shader);

		int success;
		char infoLog[1024];
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success) {
			glGetShaderInfoLog(shader, 1024, NULL, infoLog);
			std::cerr << "COMPUTE SHADER ERROR:\n" << infoLog << std::endl;
		}

		rayMarcherProgram = glCreateProgram();
		glAttachShader(rayMarcherProgram, shader);
		glLinkProgram(rayMarcherProgram);
		glDeleteShader(shader);
	}

	void createOutputTextures() {

		glGenTextures(1, &highResTexID);
		glBindTexture(GL_TEXTURE_2D, highResTexID);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, GetScreenWidth(), GetScreenHeight(), 0, GL_RGBA, GL_FLOAT, NULL);

		highResTextureRayMarcher.id = highResTexID;
		highResTextureRayMarcher.width = GetScreenWidth();
		highResTextureRayMarcher.height = GetScreenHeight();
		highResTextureRayMarcher.mipmaps = 1;
		highResTextureRayMarcher.format = PIXELFORMAT_UNCOMPRESSED_R16G16B16A16;

		int lowW = GetScreenWidth() / 8;
		int lowH = GetScreenHeight() / 8;

		glGenTextures(1, &lowResTexID);
		glBindTexture(GL_TEXTURE_2D, lowResTexID);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, lowW, lowH, 0, GL_RGBA, GL_FLOAT, NULL);

		lowResTextureRayMarcher.id = lowResTexID;
		lowResTextureRayMarcher.width = lowW;
		lowResTextureRayMarcher.height = lowH;
		lowResTextureRayMarcher.mipmaps = 1;
		lowResTextureRayMarcher.format = PIXELFORMAT_UNCOMPRESSED_R16G16B16A16;

		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void createSSBO() {
		glGenBuffers(1, &particleSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleSSBO);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, particleSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	void Init() {
		createOutputTextures();
		createShaderProgram();
		createSSBO();
		std::cout << "RayMarcher Initialized on GPU." << std::endl;
	}

	struct GPUParticle {
		float x, y, z;
		float radius;
	};

	void Run(const SceneCamera3D& cam, const std::vector<ParticlePhysics3D>& pParticles, const std::vector<ParticleRendering3D>& rParticles,
		bool& lowResRayMarching) {

		int currentW = GetScreenWidth();
		int currentH = GetScreenHeight();
		GLuint targetTex = highResTexID;

		if (lowResRayMarching) {
			currentW /= 8;
			currentH /= 8;
			targetTex = lowResTexID;
		}

		static std::vector<GPUParticle> gpuData;
		gpuData.clear();
		gpuData.reserve(pParticles.size());

		for (size_t i = 0; i < pParticles.size(); i++) {
			GPUParticle p;
			p.x = pParticles[i].pos.x;
			p.y = pParticles[i].pos.y;
			p.z = pParticles[i].pos.z;
			p.radius = rParticles[i].totalRadius;
			gpuData.push_back(p);
		}

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleSSBO);

		glBufferData(GL_SHADER_STORAGE_BUFFER, gpuData.size() * sizeof(GPUParticle), gpuData.data(), GL_DYNAMIC_DRAW);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		glUseProgram(rayMarcherProgram);

		glBindImageTexture(0, targetTex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);

		glUniform3f(glGetUniformLocation(rayMarcherProgram, "camPos"), cam.cam3D.position.x, cam.cam3D.position.y, cam.cam3D.position.z);
		glUniform3f(glGetUniformLocation(rayMarcherProgram, "camDir"), cam.camNormal.x, cam.camNormal.y, cam.camNormal.z);
		glUniform3f(glGetUniformLocation(rayMarcherProgram, "camRight"), cam.camRight.x, cam.camRight.y, cam.camRight.z);
		glUniform3f(glGetUniformLocation(rayMarcherProgram, "camUp"), cam.camUp.x, cam.camUp.y, cam.camUp.z);

		glUniform1f(glGetUniformLocation(rayMarcherProgram, "fov"), 45.0f);
		glUniform1i(glGetUniformLocation(rayMarcherProgram, "maxSteps"), marchingSteps);
		glUniform1f(glGetUniformLocation(rayMarcherProgram, "collisionMult"), collisionDistMultiplier);
		glUniform1f(glGetUniformLocation(rayMarcherProgram, "testThreshold"), testThreshold);
		glUniform1f(glGetUniformLocation(rayMarcherProgram, "smoothVal"), smoothingValue);
		glUniform1f(glGetUniformLocation(rayMarcherProgram, "nearPlane"), nearPlane);
		glUniform1f(glGetUniformLocation(rayMarcherProgram, "farPlane"), farPlane);
		glUniform2f(glGetUniformLocation(rayMarcherProgram, "resolution"), (float)currentW, (float)currentH);

		glDispatchCompute((currentW + 7) / 8, (currentH + 7) / 8, 1);

		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	}

	void Draw(bool& lowResRayMarching) {
		Texture2D texToDraw = lowResRayMarching ? lowResTextureRayMarcher : highResTextureRayMarcher;

		Rectangle source = { 0.0f, 0.0f, (float)texToDraw.width, -(float)texToDraw.height };

		Rectangle dest = { 0.0f, 0.0f, (float)GetScreenWidth(), (float)GetScreenHeight() };

		DrawTexturePro(texToDraw, source, dest, { 0,0 }, 0.0f, WHITE);
	}

	void Unload() {
		glDeleteProgram(rayMarcherProgram);

		glDeleteTextures(1, &highResTexID);
		glDeleteTextures(1, &lowResTexID);

		glDeleteBuffers(1, &particleSSBO);
	}
};