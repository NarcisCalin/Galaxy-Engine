#include "../../include/UI/UI.h"

void UI::uiLogic(UpdateParameters& myParam, UpdateVariables& myVar) {
    float screenWidth = myVar.screenWidth; // Store screenWidth for later use
    bool anyUIHovered = false;  // Could be used if more hover logic is added

    // Set up positions for toggle buttons
    toggleSettingsButtons[0].pos = { screenWidth - 34.0f, 65.0f };
    bool buttonShowSettingsHovering = toggleSettingsButtons[0].buttonLogic(showSettings, myVar);

    // Handle mouse hover state and dragging logic
    myVar.isMouseNotHoveringUI = !buttonShowSettingsHovering;

    // Toggle settings display on key press
    if (IsKeyPressed(KEY_U)) {
        showSettings = !showSettings;
    }

    // Draw the settings toggle triangle button
    DrawTriangle(
        { toggleSettingsButtons[0].pos.x + 3.0f, toggleSettingsButtons[0].pos.y + 5.0f },
        { toggleSettingsButtons[0].pos.x + 7.0f, toggleSettingsButtons[0].pos.y + 11.0f },
        { toggleSettingsButtons[0].pos.x + 11.0f, toggleSettingsButtons[0].pos.y + 5.0f }, WHITE);

    // Show notification if timestep multiplier is zero
    if (myVar.timeStepMultiplier == 0.0f) {
        DrawRectangleV({ screenWidth - 200.0f, 20.0f }, { 10.0f, 30.0f }, WHITE);
        DrawRectangleV({ screenWidth - 220.0f, 20.0f }, { 10.0f, 30.0f }, WHITE);
    }

    if (showSettings) {
        // Set up positions for settings buttons
        settingsButtonsArray[0].pos = { screenWidth - 195.0f, 80.0f };
        for (size_t i = 1; i < settingsButtonsArray.size(); ++i) {
            settingsButtonsArray[i].pos = { settingsButtonsArray[i - 1].pos.x, settingsButtonsArray[i - 1].pos.y + settingsButtonsArray[i].size.y + 13 };
            settingsButtonsArray[i].size = settingsButtonsArray[i - 1].size;
        }

        // Handle button logic for settings
        bool buttonPixelDrawingHovering = settingsButtonsArray[0].buttonLogic(myVar.isPixelDrawingEnabled, myVar);
        bool buttonGlobalTrailsHovering = settingsButtonsArray[1].buttonLogic(myVar.isGlobalTrailsEnabled, myVar);
        bool buttonSelectedTrailsHovering = settingsButtonsArray[2].buttonLogic(myVar.isSelectedTrailsEnabled, myVar);
        bool buttonLocalTrailsHovering = settingsButtonsArray[3].buttonLogic(myVar.isLocalTrailsEnabled, myVar);
        bool buttonWhiteTrailsHovering = settingsButtonsArray[4].buttonLogic(myParam.trails.whiteTrails, myVar);

        // Handle color visuals buttons
        bool buttonSolidColorHovering = settingsButtonsArray[5].buttonLogic(myParam.colorVisuals.solidColor, myVar);
        bool buttonDensityColorHovering = settingsButtonsArray[6].buttonLogic(myParam.colorVisuals.densityColor, myVar);
        bool buttonForceColorHovering = settingsButtonsArray[7].buttonLogic(myParam.colorVisuals.forceColor, myVar);
        bool buttonVelocityColorHovering = settingsButtonsArray[8].buttonLogic(myParam.colorVisuals.velocityColor, myVar);
        bool buttonSelectedColorHovering = settingsButtonsArray[9].buttonLogic(myParam.colorVisuals.selectedColor, myVar);

        // Handle other toggles
        bool buttonDarkMatterHovering = settingsButtonsArray[10].buttonLogic(myVar.isDarkMatterEnabled, myVar);
        bool buttonPeriodicBoundaryHovering = settingsButtonsArray[11].buttonLogic(myVar.isPeriodicBoundaryEnabled, myVar);
        bool buttonBarnesHutHovering = settingsButtonsArray[12].buttonLogic(myVar.isBarnesHutEnabled, myVar);
        bool buttonMultiThreadingHovering = settingsButtonsArray[13].buttonLogic(myVar.isMultiThreadingEnabled, myVar);
        bool buttonCollisionsHovering = settingsButtonsArray[14].buttonLogic(myVar.isCollisionsEnabled, myVar);

        bool buttonDensitySizeHovering = settingsButtonsArray[15].buttonLogic(myVar.isDensitySizeEnabled, myVar);
        bool buttonGlowHovering = settingsButtonsArray[16].buttonLogic(myVar.isGlowEnabled, myVar);
        bool buttonPredictPathsHovering = settingsButtonsArray[17].buttonLogic(myParam.particlesSpawning.enablePathPrediction, myVar);
        bool buttonControlsHovering = settingsButtonsArray[18].buttonLogic(myParam.controls.isShowControlsEnabled, myVar);
        bool buttonInformationHovering = settingsButtonsArray[19].buttonLogic(myVar.isInformationEnabled, myVar);

        // Set up slider positions
        slidersArray[0].sliderPos = { 20.0f, 210.0f };
        for (size_t i = 1; i < slidersArray.size(); ++i) {
            slidersArray[i].sliderPos = { slidersArray[i - 1].sliderPos.x, slidersArray[i - 1].sliderPos.y + slidersArray[i].sliderSize.y + 32 };
            slidersArray[i].sliderSize = slidersArray[i - 1].sliderSize;
        }

        // Handle slider logic
        bool sliderRed1Hovering = slidersArray[0].sliderLogic(0, myParam.colorVisuals.primaryR, 255, myVar);
        bool sliderGreen1Hovering = slidersArray[1].sliderLogic(0, myParam.colorVisuals.primaryG, 255, myVar);
        bool sliderBlue1Hovering = slidersArray[2].sliderLogic(0, myParam.colorVisuals.primaryB, 255, myVar);
        bool sliderAlpha1Hovering = slidersArray[3].sliderLogic(0, myParam.colorVisuals.primaryA, 255, myVar);

        bool sliderDensityHovering = slidersArray[8].sliderLogic(0.0f, myParam.colorVisuals.densityRadius, 30.0f, myVar);
        bool sliderMaxNeighborsHovering = slidersArray[9].sliderLogic(0, myParam.colorVisuals.maxNeighbors, 300, myVar);

        bool sliderSofteningHovering = slidersArray[12].sliderLogic(0.1f, myVar.softening, 30.0f, myVar);
        bool sliderThetaHovering = slidersArray[13].sliderLogic(0.1f, myVar.theta, 5.0f, myVar);
        bool sliderTimeScaleHovering = slidersArray[14].sliderLogic(0.0f, myVar.timeStepMultiplierSlider, 5.0f, myVar);
        bool sliderGravityStrengthHovering = slidersArray[15].sliderLogic(0.0f, myVar.gravityMultiplier, 3.0f, myVar);

        // Check if any UI element is hovered or interacted with
        if (buttonPixelDrawingHovering || buttonGlobalTrailsHovering || buttonSelectedTrailsHovering ||
            sliderRed1Hovering || sliderGreen1Hovering || sliderBlue1Hovering || sliderAlpha1Hovering) {
            myVar.isMouseNotHoveringUI = false;
            myVar.isDragging = false;
        }
        else {
            myVar.isMouseNotHoveringUI = true;
        }

        // Update color visuals on button presses
        if (buttonSolidColorHovering && IsMouseButtonPressed(0)) {
            myParam.colorVisuals.velocityColor = false;
            myParam.colorVisuals.densityColor = false;
            myParam.colorVisuals.forceColor = false;
        }
        if (buttonDensityColorHovering && IsMouseButtonPressed(0)) {
            myParam.colorVisuals.velocityColor = false;
            myParam.colorVisuals.solidColor = false;
            myParam.colorVisuals.forceColor = false;
        }

        // Reset trails when global or selected trails button is pressed
        if (buttonGlobalTrailsHovering && IsMouseButtonPressed(0)) {
            myVar.isSelectedTrailsEnabled = false;
            myParam.trails.trailDots.clear();
        }
        if (buttonSelectedTrailsHovering && IsMouseButtonPressed(0)) {
            myVar.isGlobalTrailsEnabled = false;
            myParam.trails.trailDots.clear();
        }

        // Display control and information screens
        myParam.controls.showControls(myVar);
        myParam.controls.showMoreInfo(myVar);
    }
    else {
        myVar.isMouseNotHoveringUI = !buttonShowSettingsHovering;
    }

    // Handle right-click menu
    myParam.rightClickSettings.rightClickMenu(myVar, myParam);

    // Display particle information and FPS
    DrawText(TextFormat("Particles: %i", myParam.pParticles.size()), 400, 50, 25, WHITE);
    if (myParam.pParticlesSelected.size() > 0) {
        DrawText(TextFormat("Selected Particles: %i", myParam.pParticlesSelected.size()), 700, 50, 25, WHITE);
    }

    // Display FPS with color coding baseda on performance
    int fps = GetFPS();
    if (fps >= 60) {
        DrawText(TextFormat("FPS: %i", fps), screenWidth - 150, 50, 18, GREEN);
    }
    else if (fps < 60 && fps > 30) {
        DrawText(TextFormat("FPS: %i", fps), screenWidth - 150, 50, 18, YELLOW);
    }
    else {
        DrawText(TextFormat("FPS: %i", fps), screenWidth - 150, 50, 18, RED);
    }
}
