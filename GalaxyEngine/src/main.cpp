#include <iostream>
#include <vector>
#include <cmath>
#include <omp.h>
#include "../include/raylib/raylib.h"
#include "../include/Particles/particle.h"
#include "../include/Physics/quadtree.h"
#include "../include/Physics/slingshot.h"
#include "../include/Particles/particleTrails.h"
#include "../include/UI/button.h"
#include "../include/UX/screenCapture.h"
#include "../include/Physics/morton.h"
#include "../include/UX/camera.h"
#include "../include/UI/brush.h"
#include "../include/Particles/particleSelection.h"
#include "../include/Physics/physics.h"
#include "../include/parameters.h"
#include "../include/UI/ui.h"

// ALL CHANGES AND UPDATES ARE IN THE UPDATE LOG!

// Global Instances of key classes (parameters, UI, physics)
UpdateParameters myParam;
UpdateVariables myVar;
UI myUI;

// for line 23: Don't forget to add a Header File or it will output a Compiler error

Physics physics;

// Function to build a Quadtree grid (spatial partitioning structure for efficient calculations)
static Quadtree* gridFunction(std::vector<ParticlePhysics>& pParticles,
    std::vector<ParticleRendering>& rParticles) {
    // Creates a quadtree to organize particles and optimize force calculations
    return Quadtree::boundingBox(pParticles, rParticles);
}

// Main update logic for particle simulation, physics, and other interactions
static void updateScene() {
    Quadtree* grid = nullptr;

    // Gravitational constant adjusted by user-defined gravity multiplier
    myVar.G = 6.674e-11 * myVar.gravityMultiplier;

    // Toggle simulation time stop with space key
    if (IsKeyPressed(KEY_SPACE)) {
        myVar.isTimeStopped = !myVar.isTimeStopped;
    }

    // If time is stopped, set time factor to 0; otherwise calculate it based on fixed delta time and multiplier
    myVar.timeFactor = (myVar.isTimeStopped) ? 0.0f : myVar.fixedDeltaTime * myVar.timeStepMultiplier;

    // If time is progressing, calculate the Quadtree grid for particle interactions
    if (myVar.timeFactor > 0) {
        grid = gridFunction(myParam.pParticles, myParam.rParticles);
    }

    // Physics calculations: apply gravity forces based on Barnes-Hut method or direct pairwise gravity
    if (myVar.timeFactor > 0.0f) {
        if (myVar.isBarnesHutEnabled) {
            // Parallelize the particle calculations using OpenMP for performance
            #pragma omp parallel for schedule(dynamic)
            for (size_t i = 0; i < myParam.pParticles.size(); i++) {
                ParticlePhysics& pParticle = myParam.pParticles[i];

                // Calculate gravitational force from grid and apply dark matter force if enabled
                Vector2 netForce = physics.calculateForceFromGrid(*grid, myParam.pParticles, myVar, pParticle);
                if (myVar.isDarkMatterEnabled) {
                    Vector2 dmForce = physics.darkMatterForce(pParticle, myVar);
                    netForce.x += dmForce.x;
                    netForce.y += dmForce.y;
                }

                // Update particle acceleration
                pParticle.acc.x = netForce.x / pParticle.mass;
                pParticle.acc.y = netForce.y / pParticle.mass;

                // Verlet integration to update particle velocity using current and previous accelerations
                pParticle.velocity.x += myVar.timeFactor * ((3.0f / 2.0f) * pParticle.acc.x - (1.0f / 2.0f) * pParticle.prevAcc.x);
                pParticle.velocity.y += myVar.timeFactor * ((3.0f / 2.0f) * pParticle.acc.y - (1.0f / 2.0f) * pParticle.prevAcc.y);
            }
        }
        else {
            // Fall back to direct pairwise gravity calculations (simpler but slower)
            physics.pairWiseGravity(myParam.pParticles, myVar);
        }

        // Collision handling between particles (e.g., elastic collisions or softening)
        if (myVar.isCollisionsEnabled) {
            physics.collisions(myParam.pParticles, myParam.rParticles, myVar.softening);
        }

        // Final physics update step (e.g., updating positions based on velocity)
        physics.physicsUpdate(myParam.pParticles, myParam.rParticles, myVar);
    }

    // Handle particle trails and other interactive features
    myParam.trails.trailLogic(myVar, myParam);

    // Camera logic to follow the particles and update based on user input
    myParam.myCamera.cameraFollowObject(myVar, myParam);

    // Particle selection logic for interacting with groups of particles
    myParam.particleSelection.particleSelection(myVar, myParam);
    myParam.densitySize.sizeByDensity(myParam.pParticles, myParam.rParticles, myVar.isDensitySizeEnabled, myVar.particleSizeMultiplier);

    // Deletion of particles based on user-defined selection criteria
    myParam.particleDeletion.deleteSelected(myParam.pParticles, myParam.rParticles);
    myParam.particleDeletion.deleteNonImportanParticles(myParam.pParticles, myParam.rParticles);

    // Brush-related interactions like attraction, spinning, and grabbing particles
    myParam.brush.particlesAttractor(myVar, myParam);
    myParam.brush.particlesSpinner(myVar, myParam);
    myParam.brush.particlesGrabber(myVar, myParam);
    myParam.brush.eraseBrush(myVar, myParam);

    // Clean up the grid after the update
    if (grid != nullptr) {
        delete grid;
    }
}

// Scene rendering (handles particle drawing and UI elements)
static void drawScene(Texture2D& particleBlurTex, RenderTexture2D& myUITexture) {
    // Loop through all particles and render them, either as pixels or with textures
    for (size_t i = 0; i < myParam.pParticles.size(); ++i) {
        ParticlePhysics& pParticle = myParam.pParticles[i];
        ParticleRendering& rParticle = myParam.rParticles[i];

        // Draw particles as pixels or with textures based on user preference
        if (myVar.isPixelDrawingEnabled && rParticle.drawPixel) {
            DrawPixelV({ pParticle.pos.x, pParticle.pos.y }, rParticle.color);
        }
        else {
            // Draw particles with a texture and apply size/scale based on the particle's size
            DrawTextureEx(particleBlurTex, { pParticle.pos.x - rParticle.size * myVar.particleTextureSize / 2,
                                              pParticle.pos.y - rParticle.size * myVar.particleTextureSize / 2 }, 0,
                rParticle.size, rParticle.color);
        }

        // Resize the particle based on its density if density scaling is enabled
        if (!myVar.isDensitySizeEnabled && rParticle.canBeResized) {
            rParticle.size = rParticle.previousSize * myVar.particleSizeMultiplier;
        }
    }

    // Apply color visuals to the particles (e.g., color gradients or other effects)
    myParam.colorVisuals.particlesColorVisuals(myParam.pParticles, myParam.rParticles);

    // Render particle trails if enabled
    myParam.trails.drawTrail(myParam.rParticles, particleBlurTex);

    // Start UI rendering
    BeginTextureMode(myUITexture);
    ClearBackground({ 0, 0, 0, 0 });

    // Handle mouse position and update brush (for particle interaction with mouse)
    Vector2 mouseScreenPos = GetMousePosition();
    BeginMode2D(myParam.myCamera.cameraLogic());
    myVar.mouseWorldPos = GetScreenToWorld2D(mouseScreenPos, myParam.myCamera.camera);
    myParam.brush.drawBrush(myVar.mouseWorldPos);

    // Draw boundary of the screen (for debugging or UI purposes)
    DrawRectangleLinesEx({ 0, 0, static_cast<float>(myVar.screenWidth), static_cast<float>(myVar.screenHeight) }, 3, GRAY);

    // Update the UI logic (e.g., buttons, sliders, etc.)
    myUI.uiLogic(myParam, myVar);

    // Finalize texture mode for UI rendering
    EndMode2D();
    EndTextureMode();
}

// Function to enable or disable multi-threading based on user preference
static void enableMultiThreading() {
    // Set number of threads for OpenMP based on user preference
    omp_set_num_threads(myVar.isMultiThreadingEnabled ? 16 : 1);
}

int main() {
    // Initialize the window for the simulation
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(myVar.screenWidth, myVar.screenHeight, "n-Body Simulation");
    SetTargetFPS(myVar.targetFPS);

    // Load resources (textures, shaders, etc.)
    Texture2D particleBlurTex = LoadTexture("Textures/ParticleBlur.png");
    Shader myBloom = LoadShader(nullptr, "Shaders/bloom.fs");
    RenderTexture2D myParticlesTexture = LoadRenderTexture(myVar.screenWidth, myVar.screenHeight);
    RenderTexture2D myUITexture = LoadRenderTexture(myVar.screenWidth, myVar.screenHeight);

    // Main loop: Continues running until the window is closed
    while (!WindowShouldClose()) {
        // Particle rendering and interaction
        BeginTextureMode(myParticlesTexture);
        ClearBackground(BLACK);
        BeginBlendMode(myParam.colorVisuals.blendMode);
        BeginMode2D(myParam.myCamera.cameraLogic());

        // Update the scene (physics, particles, etc.)
        updateScene();

        // Draw the scene (particles, UI, etc.)
        drawScene(particleBlurTex, myUITexture);

        EndMode2D();
        EndBlendMode();

        // Apply bloom shader if enabled
        if (myVar.isGlowEnabled) {
            BeginShaderMode(myBloom);
        }

        // Draw particles with optional bloom effect
        DrawTextureRec(myParticlesTexture.texture, { 0, 0, static_cast<float>(myVar.screenWidth), static_cast<float>(myVar.screenHeight) },
            { 0, 0 }, WHITE);

        // End the shader mode for bloom if enabled
        if (myVar.isGlowEnabled) {
            EndShaderMode();
        }

        // Handle UI interactions, e.g., buttons, sliders, etc.
        BeginTextureMode(myUITexture);
        ClearBackground({ 0, 0, 0, 0 });

        // Update the UI elements (buttons, sliders, etc.)
        myUI.uiLogic(myParam, myVar);

        EndTextureMode();

        // Draw UI texture on the screen (HUD)
        DrawTexture(myUITexture.texture, 0, 0, WHITE);

        // Poll events and update user input
        if (IsKeyPressed(KEY_ESCAPE)) {
            break;  // Exit the simulation
        }

        // Wait until the next frame (sync with FPS)
        EndDrawing();
    }

    // Clean up resources before exiting
    UnloadTexture(particleBlurTex);
    UnloadShader(myBloom);
    UnloadRenderTexture(myParticlesTexture);
    UnloadRenderTexture(myUITexture);

    // Close the window and terminate the program
    CloseWindow();

    return 0;
}
