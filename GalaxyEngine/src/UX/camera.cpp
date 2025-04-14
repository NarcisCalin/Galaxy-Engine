#include "../../include/UX/camera.h"
#include <iostream>
#include <algorithm>
#include <limits>
#include "../../include/parameters.h"

/*
   Refer to update log for changes
*/

// Constructor for SceneCamera that initializes all the camera and tracking variables to default values
SceneCamera::SceneCamera() :
    // Default initialization for camera properties and states
    camera{ {0, 0}, {0, 0}, 0.0f, 1.0f },  // Camera position, offset, angle, and zoom
    mouseWorldPos{ 0, 0 },                  // Mouse position in world coordinates
    panFollowingOffset{ 0, 0 },             // Offset for following object while panning
    previousColor{ 128, 128, 128, 255 },    // Default color (possibly for UI elements)
    followPosition{ 0, 0 },                 // The position the camera will follow
    delta{ 0, 0 },                          // Difference in position used for panning
    isFollowing(false),                     // Camera not following anything initially
    centerCamera(true)                     // Camera centered initially
{
}

// Define the selection threshold for selecting particles (adjust as needed)
const float selectionThresholdSq = 100.0f * 100.0f;  // Adjust this value as needed

// Main camera logic function, handles panning, zooming, and resets
Camera2D SceneCamera::cameraLogic() {
    // Handle camera panning with right mouse button drag
    if (IsMouseButtonDown(1)) {
        // Get the mouse movement and scale it by the current zoom factor
        delta = Vector2Scale(GetMouseDelta(), -1.0f / camera.zoom); // Smooth panning considering zoom
        camera.target = Vector2Add(camera.target, delta);           // Update camera target position
        panFollowingOffset = Vector2Add(panFollowingOffset, delta); // Update offset if camera is following something
    }

    // Handle zooming with the mouse wheel
    float wheel = GetMouseWheelMove();  // Get scroll amount
    if (wheel != 0 && !IsKeyDown(KEY_LEFT_CONTROL)) {  // Zoom if mouse wheel is scrolled and control is not held
        // Determine where to zoom (either on the screen center or the current mouse position)
        Vector2 screenCenter = { GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f };
        camera.offset = isFollowing ? screenCenter : GetMousePosition();  // Set zoom origin to either screen center or mouse position
        mouseWorldPos = GetScreenToWorld2D(camera.offset, camera);  // Convert screen position to world coordinates
        camera.target = mouseWorldPos;  // Set the camera target to the world position

        // Adjust zoom level based on scroll direction, using a logarithmic scale for smoothness
        float scale = 0.2f * wheel;
        camera.zoom = Clamp(expf(logf(camera.zoom) + scale), 0.95f, 64.0f);  // Apply zoom limit between 0.95 and 64
    }

    // Handle camera reset if the "F" key is pressed
    if (IsKeyPressed(KEY_F)) {
        // Reset camera position, offset, and zoom to default values
        camera = { {0, 0}, {0, 0}, 0.0f, 1.0f };
        panFollowingOffset = { 0, 0 };  // Reset pan offset
    }

    return camera;  // Return the updated camera object
}

// Function to handle camera following logic for objects or selection
void SceneCamera::cameraFollowObject(UpdateVariables& myVar, UpdateParameters& myParam) {
    // Update mouse position in world coordinates
    mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);

    // Track dragging state for selecting objects (particles)
    static bool isDragging = false;  // Whether the user is currently dragging
    static Vector2 dragStartPos = { 0, 0 };  // The initial position when dragging starts

    // Check if dragging starts (right-click + holding CTRL or ALT and not hovering UI)
    auto dragStarted = (IsMouseButtonPressed(1) && myVar.isMouseNotHoveringUI &&
        (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_LEFT_ALT)));

    // Check if the mouse is currently dragging
    auto dragging = (IsMouseButtonDown(1) && myVar.isMouseNotHoveringUI &&
        (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_LEFT_ALT)));

    // If dragging started, store initial mouse position
    if (dragStarted) {
        dragStartPos = GetMousePosition();  // Store where the drag started
        isDragging = false;  // Reset dragging state
    }

    // Update dragging status if the mouse has moved more than a small threshold
    if (dragging) {
        Vector2 d = Vector2Subtract(GetMousePosition(), dragStartPos);  // Calculate movement distance
        if (Vector2LengthSqr(d) > 25.0f) isDragging = true;  // Trigger drag if distance moved > threshold
    }

    // Lambda function to handle particle selection logic (either single or multiple neighbors)
    auto handleSelection = [&](bool selectNeighbors) {
        size_t closestIndex = 0;  // Track the closest particle index
        float minDistSq = std::numeric_limits<float>::max();  // Initialize with a very large distance
        std::vector<int> neighborCounts(myParam.pParticles.size(), 0);  // Neighbor count for each particle

        // If selecting neighbors, count the number of neighbors around each particle
        if (selectNeighbors) {
            for (size_t i = 0; i < myParam.pParticles.size(); ++i) {
                const auto& pi = myParam.pParticles[i];
                for (size_t j = i + 1; j < myParam.pParticles.size(); ++j) {
                    if (std::abs(myParam.pParticles[j].pos.x - pi.pos.x) > 2.4f) break;  // Skip far particles
                    float dx = pi.pos.x - myParam.pParticles[j].pos.x;
                    float dy = pi.pos.y - myParam.pParticles[j].pos.y;
                    if (dx * dx + dy * dy < 100.0f) {  // If close enough, count as neighbors
                        neighborCounts[i]++;
                        neighborCounts[j]++;
                    }
                }
            }
        }

        // Loop through all particles to find the closest one or the ones to select
        for (size_t i = 0; i < myParam.pParticles.size(); ++i) {
            myParam.rParticles[i].isSelected = false;  // Deselect any previously selected particles
            float dx = myParam.pParticles[i].pos.x - mouseWorldPos.x;  // Calculate distance to mouse
            float dy = myParam.pParticles[i].pos.y - mouseWorldPos.y;
            float distSq = dx * dx + dy * dy;  // Squared distance for efficiency

            // If selecting neighbors, select particles that are close enough and have enough neighbors
            if (selectNeighbors && neighborCounts[i] > 3 && distSq < selectionThresholdSq)
                myParam.rParticles[i].isSelected = true;

            // If selecting a single particle, find the closest one
            if (!selectNeighbors && distSq < minDistSq) {
                minDistSq = distSq;  // Update closest particle distance
                closestIndex = i;
            }
        }

        // If no neighbors selected and the closest particle is within selection threshold, select it
        if (!selectNeighbors && minDistSq < selectionThresholdSq && !myParam.pParticles.empty())
            myParam.rParticles[closestIndex].isSelected = true;

        // Set camera to follow the selected particle(s)
        isFollowing = true;
        panFollowingOffset = { 0, 0 };  // Reset pan offset

        // If trails are enabled, clear any previous trails
        if (myVar.isSelectedTrailsEnabled) myParam.trails.trailDots.clear();
        };

    // Handle mouse button release for selection (right-click without dragging)
    if (IsMouseButtonReleased(1) && !isDragging && myVar.isMouseNotHoveringUI) {
        if (IsKeyDown(KEY_LEFT_CONTROL)) handleSelection(true);  // Select neighbors
        if (IsKeyDown(KEY_LEFT_ALT)) handleSelection(false);     // Select a single particle
    }

    // Center camera on selected particle(s) when "Z" key is pressed or when the centerCamera flag is true
    if (IsKeyPressed(KEY_Z) || centerCamera) {
        isFollowing = true;  // Enable following
        panFollowingOffset = { 0, 0 };  // Reset pan offset
        centerCamera = false;  // Reset center camera flag
    }

    // If the camera is following something
    if (isFollowing) {
        float sumX = 0.0f, sumY = 0.0f;  // To store average position of selected particles
        int count = 0;  // Count the number of selected particles

        // Loop through selected particles and calculate the average position
        for (size_t i = 0; i < myParam.pParticles.size(); ++i) {
            if (myParam.rParticles[i].isSelected) {
                sumX += myParam.pParticles[i].pos.x;  // Add x position
                sumY += myParam.pParticles[i].pos.y;  // Add y position
                count++;  // Increment selected particle count
            }
        }

        // If there are selected particles, update the camera to follow their average position
        if (count > 0) {
            followPosition = Vector2Add({ sumX / count, sumY / count }, panFollowingOffset);  // Average position
            camera.target = followPosition;  // Update camera target
            camera.offset = { GetScreenWidth() * 0.5f, GetScreenHeight() * 0.5f };  // Keep the camera centered
        }
        else {
            // If no particles are selected, stop following and reset the camera
            isFollowing = false;
            camera = { {0, 0}, {0, 0}, 0.0f, 1.0f };
        }
    }
}
