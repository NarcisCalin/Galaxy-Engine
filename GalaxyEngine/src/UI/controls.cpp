#include "../../include/UI/controls.h"
#include "../../include/parameters.h"

void Controls::showControls(bool& isMouseNotHoveringUI, bool& isDragging) {
    if (!isShowControlsEnabled) return;

    float screenW = static_cast<float>(GetScreenWidth());
    float screenH = static_cast<float>(GetScreenHeight());

    constexpr float referenceWidth = 1920.0f;
    constexpr float referenceHeight = 1080.0f;

    float scaleX = screenW / referenceWidth;
    float scaleY = screenH / referenceHeight;
    float uiScale = std::clamp(std::min(scaleX, scaleY), 0.5f, 2.0f);

    controlsBoxSizeX = 435.0f * uiScale;
    controlsBoxSizeY = 810.0f * uiScale;

    fontSize = 18.0f * uiScale;
    fontSeparation = 1.0f * uiScale;
    fontYBias = 0.0f * uiScale;
    fontYSpacing = 30.0f * uiScale;

    Vector2 pageButtonPos = {
        screenW * 0.5f,
        screenH * 0.5f - controlsBoxSizeY * 0.5f - 14.0f * uiScale
    };
    Button changePageButton(pageButtonPos,
        { 14.0f * uiScale, 14.0f * uiScale },
        "", false);

    bool buttonNextPageHovering = changePageButton.buttonLogic(nextPage);
    if (buttonNextPageHovering) {
        isMouseNotHoveringUI = false;
        isDragging = false;
    }

    DrawTriangle(
        { pageButtonPos.x + 3.0f * uiScale, pageButtonPos.y + 5.0f * uiScale },
        { pageButtonPos.x + 7.0f * uiScale, pageButtonPos.y + 11.0f * uiScale },
        { pageButtonPos.x + 11.0f * uiScale, pageButtonPos.y + 5.0f * uiScale },
        WHITE
    );

    DrawRectangle(screenW * 0.5f - controlsBoxSizeX * 0.5f,
        screenH * 0.5f - controlsBoxSizeY * 0.5f,
        controlsBoxSizeX, controlsBoxSizeY,
        { 170,170,170,170 });

    float leftMargin = 20.0f * uiScale;
    float initialY = screenH * 0.5f - controlsBoxSizeY * 0.5f + 70.0f * uiScale;

    if (!nextPage) {
        DrawText("PAGE 1.",
            screenW * 0.5f - controlsBoxSizeX * 0.5f + leftMargin,
            screenH * 0.5f - controlsBoxSizeY * 0.5f + 20.0f * uiScale,
            30.0f * uiScale,
            WHITE);

        float leftMargin = 20.0f * uiScale;
        float yCursor = screenH * 0.5f - controlsBoxSizeY * 0.5f + 70.0f * uiScale;

        for (size_t i = 0; i < controlsArrayParticles.size(); i++) {
            DrawTextEx(
                GetFontDefault(),
                TextFormat("%s", controlsArrayParticles[i].c_str()),
                { screenW * 0.5f - controlsBoxSizeX * 0.5f + leftMargin,
                  yCursor },
                fontSize, fontSeparation, WHITE);
            yCursor += fontYSpacing;
        }

        float secondListY = initialY + controlsArrayParticles.size() * (fontYSpacing + fontSize);
        for (size_t i = 0; i < controlsArrayCamAndSelec.size(); i++) {
            DrawTextEx(
                GetFontDefault(),
                TextFormat("%s", controlsArrayCamAndSelec[i].c_str()),
                { screenW * 0.5f - controlsBoxSizeX * 0.5f + leftMargin,
                  yCursor },
                fontSize, fontSeparation, WHITE);
            yCursor += fontYSpacing;
        }
    }
    else {
        DrawText("PAGE 2.",
            screenW * 0.5f - controlsBoxSizeX * 0.5f + leftMargin,
            screenH * 0.5f - controlsBoxSizeY * 0.5f + 20.0f * uiScale,
            30.0f * uiScale,
            WHITE);

        for (size_t i = 0; i < controlsArrayUtility.size(); i++) {
            DrawTextEx(GetFontDefault(),
                TextFormat("%s", controlsArrayUtility[i].c_str()),
                { screenW * 0.5f - controlsBoxSizeX * 0.5f + leftMargin,
                  initialY + fontYSpacing * i + fontYBias },
                fontSize,
                fontSeparation,
                WHITE);
        }
    }
}

void Controls::showMoreInfo() {
    if (!isInformationEnabled) return;

    float screenW = static_cast<float>(GetScreenWidth());
    float screenH = static_cast<float>(GetScreenHeight());

    constexpr float referenceWidth = 1920.0f;
    constexpr float referenceHeight = 1080.0f;
    float scaleX = screenW / referenceWidth;
    float scaleY = screenH / referenceHeight;
    float uiScale = std::clamp(std::min(scaleX, scaleY), 0.5f, 2.0f);

    float leftMargin = 20.0f * uiScale;
    float topMargin = 10.0f * uiScale;
    float lineSpacing = 30.0f * uiScale;
    float infoFontSize = 18.0f * uiScale;
    float fontSpacing = 1.0f * uiScale;

    Font font = GetFontDefault();
    float maxLineWidth = 0;
    for (auto& line : informationArray) {
        Vector2 sz = MeasureTextEx(font, line.c_str(), infoFontSize, fontSpacing);
        maxLineWidth = std::max(maxLineWidth, sz.x);
    }
    float textBlockHeight = informationArray.size() * lineSpacing;

    float infoBoxW = leftMargin * 2 + maxLineWidth;
    float infoBoxH = topMargin * 2 + textBlockHeight;

    infoBoxW = std::min(infoBoxW, screenW * 0.9f);
    infoBoxH = std::min(infoBoxH, screenH * 0.9f);

    float boxX = screenW * 0.5f - infoBoxW * 0.5f;
    float boxY = screenH * 0.5f - infoBoxH * 0.5f;
    DrawRectangleV(
        { boxX, boxY },
        { infoBoxW, infoBoxH },
        { 170, 170, 170, 170 }
    );

    float xPos = boxX + leftMargin;
    float yCursor = boxY + topMargin;
    for (auto& line : informationArray) {
        DrawTextEx(font, line.c_str(), { xPos, yCursor }, infoFontSize, fontSpacing, WHITE);
        yCursor += lineSpacing;
    }
}
