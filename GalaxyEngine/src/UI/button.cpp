#include <iostream>
#include "raylib.h"
#include "../../include/UI/button.h"
#include "../../include/parameters.h"

Button::Button(Vector2 pos, Vector2 size, std::string text, bool hasText) {
	this->pos = pos;
	this->size = size;
	this->text = text;
    this->hasText = hasText;
}

bool Button::buttonLogic(bool& isEnabled, UpdateVariables& myVar) {
    if (isEnabled) {
        color = { 70, 140, 70, 255 };
    }
    else {
        color = { 128, 128, 128, 255 };
    }

    bool isHovering = (GetMouseX() > pos.x &&
        GetMouseY() > pos.y &&
        GetMouseX() < pos.x + size.x &&
        GetMouseY() < pos.y + size.y);

    isOnTop = isHovering;

    if (isHovering) {
        color.r = static_cast<unsigned char>(color.r * 0.8);
        color.g = static_cast<unsigned char>(color.g * 0.8);
        color.b = static_cast<unsigned char>(color.b * 0.8);

        if (IsMouseButtonPressed(0)) {
            isEnabled = !isEnabled;
        }
    }

    DrawRectangleV({ pos.x, pos.y }, { size.x, size.y }, color);

    if (hasText) {
    Vector2 textCompensation = MeasureTextEx(GetFontDefault() , text.c_str(), textSize, textSeparation);
    DrawTextEx(GetFontDefault(), text.c_str(), { static_cast<float>(pos.x + size.x / 2 - (textCompensation.x / 2)), 
        static_cast<float>(pos.y + size.y / 2 - (textCompensation.y / 2)) }, textSize, textSeparation, WHITE);
 }

    return isOnTop;
}