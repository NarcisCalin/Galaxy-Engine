#include <iostream>
#include <vector>
#include "../include/raylib/raylib.h"
#include "../include/Physics/physics.h"
#include "../include/UI/UI.h"
#include "../include/Particles/particle.h"
#include "../include/Particles/particleColorVisuals.h"
#include "../include/Particles/particleDeletion.h"
#include "../include/Physics/quadtree.h"
#include "../include/Physics/slingshot.h"
#include "../include/Physics/morton.h"
#include <omp.h>

// ALL UPDATES AND CHANGES ARE IN THE UPDATE LOG

UpdateParameters myParam;
UpdateVariables myVar;
UI myUI;
Physics physics;

// function to update scene logic
void updateScene() {
    Quadtree* grid = nullptr;

    // set gravitational constant with multiplier
    myVar.G = 6.674e-11 * myVar.gravityMultiplier;

    // toggle time stop when spacebar is pressed
    if (IsKeyPressed(KEY_SPACE)) myVar.isTimeStopped = !myVar.isTimeStopped;

    // set time step multiplier based on time stop flag
    myVar.timeStepMultiplier = myVar.isTimeStopped ? 0.0f : myVar.timeStepMultiplierSlider;
    myVar.timeFactor = myVar.fixedDeltaTime * myVar.timeStepMultiplier;

    // create quadtree if time factor is positive
    if (myVar.timeFactor > 0)
        grid = Quadtree::boundingBox(myParam.pParticles, myParam.rParticles);

    // draw the quadtree if enabled
    if (grid && myVar.drawQuadtree)
        grid->drawQuadtree();

    // update brush size based on camera position
    myParam.brush.brushSize(myParam.myCamera.mouseWorldPos);

    // spawn particles based on initial conditions
    myParam.particlesSpawning.particlesInitialConditions(*grid, physics, myVar, myParam);

    // update particles if time step is positive
    if (myVar.timeFactor > 0.0f) {
        // use barnes-hut method if enabled
        if (myVar.isBarnesHutEnabled) {
#pragma omp parallel for schedule(dynamic)
            for (size_t i = 0; i < myParam.pParticles.size(); ++i) {
                ParticlePhysics& pParticle = myParam.pParticles[i];
                Vector2 netForce = physics.calculateForceFromGrid(*grid, myParam.pParticles, myVar, pParticle);

                // add dark matter force if enabled
                if (myVar.isDarkMatterEnabled) {
                    Vector2 dmForce = physics.darkMatterForce(pParticle, myVar);
                    netForce.x += dmForce.x;
                    netForce.y += dmForce.y;
                }

                // update acceleration, velocity, and position using the force
                pParticle.acc.x = netForce.x / pParticle.mass;
                pParticle.acc.y = netForce.y / pParticle.mass;
                pParticle.velocity.x += (myVar.timeFactor * ((3.0f / 2.0f) * pParticle.acc.x - (1.0f / 2.0f) * pParticle.prevAcc.x));
                pParticle.velocity.y += (myVar.timeFactor * ((3.0f / 2.0f) * pParticle.acc.y - (1.0f / 2.0f) * pParticle.prevAcc.y));
            }
        }
        else {
            // otherwise, use pairwise gravity
            physics.pairWiseGravity(myParam.pParticles, myVar);
        }

        // handle collisions if enabled
        if (myVar.isCollisionsEnabled)
            physics.collisions(myParam.pParticles, myParam.rParticles, myVar.softening);

        // update physics (position, velocity, etc.)
        physics.physicsUpdate(myParam.pParticles, myParam.rParticles, myVar);
    }

    // update trails and camera following logic
    myParam.trails.trailLogic(myVar, myParam);
    myParam.myCamera.cameraFollowObject(myVar, myParam);

    // handle particle selection and deletion
    myParam.particleSelection.particleSelection(myVar, myParam);
    myParam.particleDeletion.deleteSelected(myParam.pParticles, myParam.rParticles);

    // erase brush if needed
    myParam.brush.eraseBrush(myVar, myParam);

    // cleanup the grid
    delete grid;
}

// function to draw the scene
void drawScene(Texture2D& particleBlurTex, RenderTexture2D& myUITexture) {
    // loop through each particle and render it
    for (size_t i = 0; i < myParam.pParticles.size(); ++i) {
        ParticlePhysics& pParticle = myParam.pParticles[i];
        ParticleRendering& rParticle = myParam.rParticles[i];

        // draw pixel if pixel drawing is enabled
        if (myVar.isPixelDrawingEnabled && rParticle.drawPixel) {
            DrawPixelV({ pParticle.pos.x, pParticle.pos.y }, rParticle.color);
        }
        else {
            // otherwise, draw a texture with specified size and color
            DrawTextureEx(particleBlurTex, { pParticle.pos.x - rParticle.size * myVar.particleTextureSize / 2, pParticle.pos.y - rParticle.size * myVar.particleTextureSize / 2 }, 0, rParticle.size, rParticle.color);
        }

        // update particle size based on density if enabled
        if (!myVar.isDensitySizeEnabled) {
            rParticle.size = rParticle.canBeResized ? rParticle.previousSize * myVar.particleSizeMultiplier : rParticle.previousSize;
        }
    }

    // draw color visuals and trails
    myParam.colorVisuals.particlesColorVisuals(myParam.pParticles, myParam.rParticles);
    myParam.trails.drawTrail(myParam.rParticles, particleBlurTex);

    // begin UI drawing
    BeginTextureMode(myUITexture);
    ClearBackground({ 0,0,0,0 });
    BeginMode2D(myParam.myCamera.camera);

    // set mouse world position based on screen position
    myVar.mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), myParam.myCamera.camera);
    myParam.brush.drawBrush(myVar.mouseWorldPos);

    // draw the particle connections if enabled
    if (myParam.pParticles.size() > 1 && myVar.drawZCurves) {
        for (size_t i = 0; i < myParam.pParticles.size() - 1; ++i) {
            DrawLineV(myParam.pParticles[i].pos, myParam.pParticles[i + 1].pos, WHITE);
            DrawText(TextFormat("%i", i), static_cast<int>(myParam.pParticles[i].pos.x), static_cast<int>(myParam.pParticles[i].pos.y) - 10, 10, { 128,128,128,128 });
        }
    }

    EndMode2D();

    // update UI logic
    myUI.uiLogic(myParam, myVar);
    myParam.subdivision.subdivideParticles(myVar, myParam);
    EndTextureMode();

    // draw the UI texture
    DrawTextureRec(myUITexture.texture, Rectangle{ 0, 0, static_cast<float>(myUITexture.texture.width), -static_cast<float>(myUITexture.texture.height) }, Vector2{ 0, 0 }, WHITE);
}

// function to enable multi-threading
void enableMultiThreading() {
    omp_set_num_threads(myVar.isMultiThreadingEnabled ? 16 : 1);
}

int main() {
    // setup window and graphics
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(myVar.screenWidth, myVar.screenHeight, "n-Body");
    Texture2D particleBlurTex = LoadTexture("Textures/ParticleBlur.png");
    Shader myBloom = LoadShader(nullptr, "Shaders/bloom.fs");
    RenderTexture2D myParticlesTexture = LoadRenderTexture(myVar.screenWidth, myVar.screenHeight);
    RenderTexture2D myUITexture = LoadRenderTexture(myVar.screenWidth, myVar.screenHeight);

    SetTargetFPS(myVar.targetFPS);

    // main loop
    while (!WindowShouldClose()) {
        BeginTextureMode(myParticlesTexture);
        ClearBackground(BLACK);
        BeginBlendMode(myParam.colorVisuals.blendMode);
        BeginMode2D(myParam.myCamera.cameraLogic());

        // update and draw scene
        updateScene();
        drawScene(particleBlurTex, myUITexture);

        EndMode2D();
        EndBlendMode();

        // apply bloom shader if enabled
        if (myVar.isGlowEnabled) BeginShaderMode(myBloom);
        DrawTextureRec(myParticlesTexture.texture, Rectangle{ 0, 0, static_cast<float>(myParticlesTexture.texture.width), -static_cast<float>(myParticlesTexture.texture.height) }, Vector2{ 0, 0 }, WHITE);
        if (myVar.isGlowEnabled) EndShaderMode();

        // handle screen capture and drawing UI borders if recording
        myVar.isRecording = myParam.screenCapture.screenGrab(myParticlesTexture, myVar);
        if (myVar.isRecording) DrawRectangleLinesEx({ 0, 0, static_cast<float>(myVar.screenWidth), static_cast<float>(myVar.screenHeight) }, 3, RED);

        EndDrawing();

        // enable multi-threading
        enableMultiThreading();
    }

    // clean up resources
    UnloadShader(myBloom);
    UnloadTexture(particleBlurTex);
    UnloadRenderTexture(myParticlesTexture);
    UnloadRenderTexture(myUITexture);

    CloseWindow();
    return 0;
}
