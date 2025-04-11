#pragma once
#include <iostream>
#include <algorithm>
#include <type_traits>
#include <cmath>
#include "../raylib/raylib.h"
#include "../parameters.h"

struct UpdateVariables;

class Slider {
public:
	Vector2 sliderPos;
	Vector2 sliderSize;
	Color sliderColor;

	std::string name;

	Slider(Vector2 sliderPos, Vector2 sliderSize, Color sliderColor, std::string name);

	template <typename T>
	inline bool sliderLogic(T minValue, T& value, T maxValue, UpdateVariables& myVar) {

		this->handleColor = {
			static_cast<unsigned char>(sliderColor.r * 1.2f),
			static_cast<unsigned char>(sliderColor.g * 1.2f),
			static_cast<unsigned char>(sliderColor.b * 1.2f),
			255
		};

		isHoveringSlider = (GetMouseX() > sliderPos.x && GetMouseX() < sliderPos.x + sliderSize.x
			&& GetMouseY() > sliderPos.y && GetMouseY() < sliderPos.y + sliderSize.y);

		isHoveringHandle = (GetMouseX() > handlePos.x && GetMouseX() < handlePos.x + handleSize.x
			&& GetMouseY() > handlePos.y && GetMouseY() < handlePos.y + handleSize.y);

		if (isHoveringHandle) {
			handleColor = {
				static_cast<unsigned char>(handleColor.r * 0.8f),
				static_cast<unsigned char>(handleColor.g * 0.8f),
				static_cast<unsigned char>(handleColor.b * 0.8f),
				255
			};
		}
		
		if (isHoveringHandle && IsMouseButtonPressed(0)) {
			initialMouseX = GetMouseX() - handlePos.x;
			isClicked = true;
		}

		if (IsMouseButtonReleased(0)) {
			isClicked = false;
		}

		if (isClicked) {
			handlePos.x = GetMouseX() - initialMouseX;
		}

		if constexpr (std::is_same_v<T, int>) {
			operatorType = 'i';

		}
		else if constexpr (std::is_same_v<T, float>) {
			operatorType = 'f';

		}

		char formatString[10];
		sprintf_s(formatString, "%%%c", operatorType);


		handlePos.x = std::clamp(handlePos.x, sliderPos.x, sliderPos.x + sliderSize.x - handleSize.x);

		handleSize = { sliderSize.x / 8, sliderSize.y + 10 };
		handlePos = { handlePos.x, sliderPos.y - ((handleSize.y - sliderSize.y) / 2) };

		float sliderValue = handlePos.x - sliderPos.x;

		float normalizedSlider = sliderValue / (sliderSize.x - handleSize.x);

		if (setInitialPos) {
			handlePos.x += (sliderSize.x - handleSize.x) * (float)value / maxValue;
			initialHandlePosX = handlePos.x;
			setInitialPos = false;
		}

		if (isOnTop && IsMouseButtonPressed(1)) {
			handlePos.x = initialHandlePosX;
		}

		if constexpr (std::is_same_v<T, int>) {
			value = static_cast<T>(std::round(maxValue * normalizedSlider));
		}
		else {
			value = std::clamp(static_cast<T>(maxValue * normalizedSlider), minValue, maxValue);
		}


		DrawRectangle(static_cast<int>(sliderPos.x), static_cast<int>(sliderPos.y), static_cast<int>(sliderSize.x), static_cast<int>(sliderSize.y), sliderColor);

		DrawRectangle(static_cast<int>(handlePos.x), static_cast<int>(handlePos.y), static_cast<int>(handleSize.x), static_cast<int>(handleSize.y), handleColor);

		DrawText(TextFormat("%s: %s", name.c_str(), TextFormat(formatString, value)), static_cast<int>(sliderPos.x), static_cast<int>(sliderPos.y - textSize - 8), static_cast<int>(textSize), WHITE);
		if (isHoveringHandle || isHoveringSlider) {
			isOnTop = true;
			myVar.isMouseNotHoveringUI = false;
			myVar.isDragging = false;
		}
		else {
			isOnTop = false;
			myVar.isMouseNotHoveringUI = true;
		}

		return isOnTop;
	};

private:
	bool isHoveringSlider;
	bool isHoveringHandle;
	bool isOnTop;

	float textSize = 17;

	float initialMouseX;
	bool isClicked;

	bool setInitialPos = true;
	float initialHandlePosX;

	char operatorType;

	bool isFirstUpdate = true;

	Vector2 handlePos;
	Vector2 handleSize;
	Color handleColor;

	Vector2 minHandlePos;
	Vector2 maxHandlePos;
};
