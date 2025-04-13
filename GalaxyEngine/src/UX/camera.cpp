#include "../../include/UX/camera.h"
#include <iostream>
#include <algorithm>
#include <limits>
#include "../../include/parameters.h"

// update list is provided, check that out for changes.

SceneCamera::SceneCamera() :
    // Initialize everything with default values
    camera{ {0, 0}, {0, 0}, 0.0f, 1.0f },
    mouseWorldPos{ 0, 0 },
    panFollowingOffset{ 0, 0 },
    previousColor{ 128, 128, 128, 255 },
    followPosition{ 0, 0 },
    delta{ 0, 0 },
    isFollowing(false),
    centerCamera(false) {
}

// Main camera logic that handles panning, zooming, and resets
Camera2D SceneCamera::cameraLogic() {
    // Right mouse button dragging = panning the camera
    if (IsMouseButtonDown(1)) {
        delta = Vector2Scale(GetMouseDelta(), -1.0f / camera.zoom); // Scale by zoom for smoothness
        camera.target = Vector2Add(camera.target, delta);           // Move the target
        panFollowingOffset = Vector2Add(panFollowingOffset, delta); // Offset if following something
    }

    // Zooming with the mouse wheel
    float wheel = GetMouseWheelMove();
    if (wheel != 0 && !IsKeyDown(KEY_LEFT_CONTROL)) {
        Vector2 screenCenter = { GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f };
        camera.offset = isFollowing ? screenCenter : GetMousePosition(); // Zoom on center or cursor
        mouseWorldPos = GetScreenToWorld2D(camera.offset, camera);
        camera.target = mouseWorldPos;

        // Zoom in or out smoothly
        float scale = 0.2f * wheel;
        camera.zoom = Clamp(expf(logf(camera.zoom) + scale), 0.95f, 64.0f);
    }

    /*
       // Smooth zoom in or out, experienmental, idk, havent seen if it works yet.
        camera.zoom = Clamp(camera.zoom * (1.0f + 0.05f * wheel), 0.95f, 64.0f);

    }
    */

    // Reset camera if "F" is pressed
    if (IsKeyPressed(KEY_F)) {
        camera = { {0, 0}, {0, 0}, 0.0f, 1.0f };
        panFollowingOffset = { 0, 0 };
    }
    return camera;
}

// Handles camera following an object or selection logic
void SceneCamera::cameraFollowObject(UpdateVariables& myVar, UpdateParameters& myParam) {
    // Update mouse world position
    mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);

    static bool isDragging = false;
    static Vector2 dragStartPos = { 0, 0 };

    // Check if dragging starts (click + holding CTRL or ALT + not hovering UI)
    auto dragStarted = (IsMouseButtonPressed(1) && myVar.isMouseNotHoveringUI &&
        (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_LEFT_ALT)));
    auto dragging = (IsMouseButtonDown(1) && myVar.isMouseNotHoveringUI &&
        (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_LEFT_ALT)));

    // Start of drag
    if (dragStarted) {
        dragStartPos = GetMousePosition();
        isDragging = false;
    }

    // Update dragging status if moved enough
    if (dragging) {
        Vector2 d = Vector2Subtract(GetMousePosition(), dragStartPos);
        if (Vector2LengthSqr(d) > 25.0f) isDragging = true; // Only count as drag if moved > threshold
    }

    // Lambda to handle selection of particles
    auto handleSelection = [&](bool selectNeighbors) {
        size_t closestIndex = 0;
        float minDistSq = std::numeric_limits<float>::max();
        std::vector<int> neighborCounts(myParam.pParticles.size(), 0);

        // If selecting neighbors, count neighbors around each particle
        if (selectNeighbors) {
            for (size_t i = 0; i < myParam.pParticles.size(); ++i) {
                const auto& pi = myParam.pParticles[i];
                for (size_t j = i + 1; j < myParam.pParticles.size(); ++j) {
                    if (std::abs(myParam.pParticles[j].pos.x - pi.pos.x) > 2.4f) break;
                    float dx = pi.pos.x - myParam.pParticles[j].pos.x;
                    float dy = pi.pos.y - myParam.pParticles[j].pos.y;
                    if (dx * dx + dy * dy < 100.0f) {
                        neighborCounts[i]++;
                        neighborCounts[j]++;
                    }
                }
            }
        }

        // Loop over particles to find the closest one (or neighbors)
        for (size_t i = 0; i < myParam.pParticles.size(); ++i) {
            myParam.rParticles[i].isSelected = false;
            float dx = myParam.pParticles[i].pos.x - mouseWorldPos.x;
            float dy = myParam.pParticles[i].pos.y - mouseWorldPos.y;
            float distSq = dx * dx + dy * dy;

            // If selecting neighbors and close enough, select it
            if (selectNeighbors && neighborCounts[i] > 3 && distSq < selectionThresholdSq)
                myParam.rParticles[i].isSelected = true;

            // If just finding closest one
            if (!selectNeighbors && distSq < minDistSq) {
                minDistSq = distSq;
                closestIndex = i;
            }
        }

        // Select the closest particle if needed
        if (!selectNeighbors && minDistSq < selectionThresholdSq && !myParam.pParticles.empty())
            myParam.rParticles[closestIndex].isSelected = true;

        // Set camera to follow the selected particle(s)
        isFollowing = true;
        panFollowingOffset = { 0, 0 };

        // If trails are enabled, clear previous trails
        if (myVar.isSelectedTrailsEnabled) myParam.trails.trailDots.clear();
        };

    // Handle mouse release (click selection, no dragging)
    if (IsMouseButtonReleased(1) && !isDragging && myVar.isMouseNotHoveringUI) {
        if (IsKeyDown(KEY_LEFT_CONTROL)) handleSelection(true);  // Select neighbors
        if (IsKeyDown(KEY_LEFT_ALT)) handleSelection(false);     // Select single
    }

    // Center camera on selection manually (Z key or centerCamera flag)
    if (IsKeyPressed(KEY_Z) || centerCamera) {
        isFollowing = true;
        panFollowingOffset = { 0, 0 };
        centerCamera = false;
    }

    // If we are following something
    if (isFollowing) {
        float sumX = 0.0f, sumY = 0.0f;
        int count = 0;

        // Average the position of all selected particles
        for (size_t i = 0; i < myParam.pParticles.size(); ++i) {
            if (myParam.rParticles[i].isSelected) {
                sumX += myParam.pParticles[i].pos.x;
                sumY += myParam.pParticles[i].pos.y;
                count++;
            }
        }

        // If there are selected particles, follow them
        if (count > 0) {
            followPosition = Vector2Add({ sumX / count, sumY / count }, panFollowingOffset);
            camera.target = followPosition;
            camera.offset = { GetScreenWidth() * 0.5f, GetScreenHeight() * 0.5f };
        }

        /* For Line 173:

        / 2.0f -> slow division

            * 0.5f -> fast multiplication
        */

        else {
            // Nothing selected anymore, stop following
            isFollowing = false;
            camera = { {0, 0}, {0, 0}, 0.0f, 1.0f };
            panFollowingOffset = { 0, 0 };
        }

        // If "F" is pressed or no particles exist, stop following
        if (IsKeyPressed(KEY_F) || myParam.p_
