#include <iostream>
#include <vector>
#include <cmath>
#include <omp.h>
#include "../include/raylib/raylib.h"
#include "../include/Particles/particle.h"
#include "../include/Physics/quadtree.h"
#include "../include/Particles/particleTrails.h"
#include "../include/UI/UI.h"
#include "../include/Physics/physics.h"
#include "../include/parameters.h"

//changes in updatelog

UpdateParameters myParam;
UpdateVariables myVar;
UI myUI;
Physics physics;

static Quadtree* gridFunction(std::vector<ParticlePhysics>& pParticles, std::vector<ParticleRendering>& rParticles) {
    return Quadtree::boundingBox(pParticles, rParticles);
}

static void updateScene() {
    Quadtree* grid = nullptr;
    myVar.G = 6.674e-11 * myVar.gravityMultiplier;

    if (IsKeyPressed(KEY_SPACE)) myVar.isTimeStopped = !myVar.isTimeStopped;
    myVar.timeStepMultiplier = myVar.isTimeStopped ? 0.0f : myVar.timeStepMultiplierSlider;
    myVar.timeFactor = myVar.fixedDeltaTime * myVar.timeStepMultiplier;

    if (myVar.timeFactor > 0) grid = gridFunction(myParam.pParticles, myParam.rParticles);

    if (grid && myVar.drawQuadtree) grid->drawQuadtree();

    myParam.brush.brushSize(myParam.myCamera.mouseWorldPos);
    myParam.particlesSpawning.particlesInitialConditions(*grid, physics, myVar, myParam);

    if (myVar.timeFactor > 0.0f) {
        if (myVar.isBarnesHutEnabled) {
#pragma omp parallel for schedule(dynamic)
            for (size_t i = 0; i < myParam.pParticles.size(); i++) {
                ParticlePhysics& pParticle = myParam.pParticles[i];
                Vector2 netForce = physics.calculateForceFromGrid(*grid, myParam.pParticles, myVar, pParticle);
                if (myVar.isDarkMatterEnabled) {
                    Vector2 dmForce = physics.darkMatterForce(pParticle, myVar);
                    netForce.x += dmForce.x;
                    netForce.y += dmForce.y;
                }
                pParticle.acc.x = netForce.x / pParticle.mass;
                pParticle.acc.y = netForce.y / pParticle.mass;
                pParticle.velocity.x += myVar.timeFactor * ((3.0f / 2.0f) * pParticle.acc.x - (1.0f / 2.0f) * pParticle.prevAcc.x);
                pParticle.velocity.y += myVar.timeFactor * ((3.0f / 2.0f) * pParticle.acc.y - (1.0f / 2.0f) * pParticle.prevAcc.y);
            }
        }
        else {
            physics.pairWiseGravity(myParam.pParticles, myVar);
        }

        if (myVar.isCollisionsEnabled) physics.collisions(myParam.pParticles, myParam.rParticles, myVar.softening);
        physics.physicsUpdate(myParam.pParticles, myParam.rParticles, myVar);
    }

    myParam.trails.trailLogic(myVar, myParam);
    myParam.myCamera.cameraFollowObject(myVar, myParam);
    myParam.particleSelection.particleSelection(myVar, myParam);
    myParam.densitySize.sizeByDensity(myParam.pParticles, myParam.rParticles, myVar.isDensitySizeEnabled, myVar.particleSizeMultiplier);
    myParam.particleDeletion.deleteSelected(myParam.pParticles, myParam.rParticles);
    myParam.brush.eraseBrush(myVar, myParam);

    if (grid) delete grid;
}

static void drawScene(Texture2D& particleBlurTex, RenderTexture2D& myUITexture) {
    for (size_t i = 0; i < myParam.pParticles.size(); ++i) {
        ParticlePhysics& pParticle = myParam.pParticles[i];
        ParticleRendering& rParticle = myParam.rParticles[i];
        if (myVar.isPixelDrawingEnabled && rParticle.drawPixel) {
            DrawPixelV({ pParticle.pos.x, pParticle.pos.y }, rParticle.color);
        }
        else {
            DrawTextureEx(particleBlurTex, { pParticle.pos.x - rParticle.size * myVar.particleTextureSize / 2,
                                             pParticle.pos.y - rParticle.size * myVar.particleTextureSize / 2 }, 0,
                rParticle.size, rParticle.color);
        }
        if (!myVar.isDensitySizeEnabled && rParticle.canBeResized) {
            rParticle.size = rParticle.previousSize * myVar.particleSizeMultiplier;
        }
    }

    myParam.colorVisuals.particlesColorVisuals(myParam.pParticles, myParam.rParticles);
    myParam.trails.drawTrail(myParam.rParticles, particleBlurTex);

    BeginTextureMode(myUITexture);
    ClearBackground({ 0, 0, 0, 0 });
    myVar.mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), myParam.myCamera.camera);
    myParam.brush.drawBrush(myVar.mouseWorldPos);
    myUI.uiLogic(myParam, myVar);
    EndTextureMode();
}

static void enableMultiThreading() {
    omp_set_num_threads(myVar.isMultiThreadingEnabled ? 16 : 1);
}

int main() {
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(myVar.screenWidth, myVar.screenHeight, "n-Body");
    Texture2D particleBlurTex = LoadTexture("Textures/ParticleBlur.png");
    Shader myBloom = LoadShader(nullptr, "Shaders/bloom.fs");
    RenderTexture2D myParticlesTexture = LoadRenderTexture(myVar.screenWidth, myVar.screenHeight);
    RenderTexture2D myUITexture = LoadRenderTexture(myVar.screenWidth, myVar.screenHeight);
    SetTargetFPS(myVar.targetFPS);

    while (!WindowShouldClose()) {
        BeginTextureMode(myParticlesTexture);
        ClearBackground(BLACK);
        BeginBlendMode(myParam.colorVisuals.blendMode);
        BeginMode2D(myParam.myCamera.cameraLogic());
        updateScene();
        drawScene(particleBlurTex, myUITexture);
        EndMode2D();
        EndBlendMode();

        if (myVar.isGlowEnabled) BeginShaderMode(myBloom);
        DrawTextureRec(myParticlesTexture.texture, { 0, 0, static_cast<float>(myParticlesTexture.texture.width), -static_cast<float>(myParticlesTexture.texture.height) }, { 0, 0 }, WHITE);
        if (myVar.isGlowEnabled) EndShaderMode();
        DrawTextureRec(myUITexture.texture, { 0, 0, static_cast<float>(myUITexture.texture.width), -static_cast<float>(myUITexture.texture.height) }, { 0, 0 }, WHITE);

        myVar.isRecording = myParam.screenCapture.screenGrab(myParticlesTexture, myVar);
        if (myVar.isRecording) DrawRectangleLinesEx({ 0, 0, static_cast<float>(myVar.screenWidth), static_cast<float>(myVar.screenHeight) }, 3, RED);

        EndDrawing();
        enableMultiThreading();
    }

    UnloadShader(myBloom);
    UnloadTexture(particleBlurTex);
    UnloadRenderTexture(myParticlesTexture);
    UnloadRenderTexture(myUITexture);
    CloseWindow();
    return 0;
}
