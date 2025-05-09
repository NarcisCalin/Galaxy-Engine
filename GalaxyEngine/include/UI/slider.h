#pragma once
#include <iostream>
#include <algorithm>
#include <type_traits>
#include <cmath>
#include "../raylib/raylib.h"

class Slider {
public:
	Vector2 sliderPos;
	Vector2 sliderSize;
	Color sliderColor;

	std::string name;

	Slider(Vector2 sliderPos, Vector2 sliderSize, Color sliderColor, std::string name);

	template <typename T>
	inline bool sliderLogic(T minValue, T& value, T maxValue) {

		if (GetScreenWidth() >= 1920 && GetScreenHeight() >= 1080) {
			textSize = 17.0f;
			textSliderSeparation = 8;
		}
		else if (GetScreenWidth() < 1920 && GetScreenHeight() < 1080) {
			textSize = 16.4f;
			textSliderSeparation = 6;
		}

		handleSize = { sliderSize.x / 8, sliderSize.y + 10 };

		float range = static_cast<float>(maxValue) - static_cast<float>(minValue);
		float pct = (static_cast<float>(value) - static_cast<float>(minValue)) / range;
		float idealX = sliderPos.x + pct * (sliderSize.x - handleSize.x);

		if (setInitialPos) {
			initialHandlePosX = idealX;
			initialValue = static_cast<double>(value);
			setInitialPos = false;
		}

		if (!isClicked) {
			handlePos.x = idealX;
		}

		isHoveringSlider = (GetMouseX() > sliderPos.x && GetMouseX() < sliderPos.x + sliderSize.x
			&& GetMouseY() > sliderPos.y && GetMouseY() < sliderPos.y + sliderSize.y);
		isHoveringHandle = (GetMouseX() > handlePos.x && GetMouseX() < handlePos.x + handleSize.x
			&& GetMouseY() > handlePos.y && GetMouseY() < handlePos.y + handleSize.y);

		handleColor = {
			static_cast<unsigned char>(sliderColor.r * 1.2f),
			static_cast<unsigned char>(sliderColor.g * 1.2f),
			static_cast<unsigned char>(sliderColor.b * 1.2f),
			255
		};
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

		handlePos.x = std::clamp(handlePos.x, sliderPos.x, sliderPos.x + sliderSize.x - handleSize.x);
		handlePos.y = sliderPos.y - ((handleSize.y - sliderSize.y) / 2);

		float normalized = (handlePos.x - sliderPos.x) / (sliderSize.x - handleSize.x);
		if constexpr (std::is_same_v<T, int>) {
			value = std::clamp<T>(
				static_cast<T>(std::round(minValue + range * normalized)),
				minValue, maxValue
			);
			operatorType = 'i';
		}
		else {
			value = std::clamp(
				static_cast<T>(minValue + range * normalized),
				minValue, maxValue
			);
			operatorType = 'f';
		}

		DrawRectangle((int)sliderPos.x, (int)sliderPos.y, (int)sliderSize.x, (int)sliderSize.y, sliderColor);
		DrawRectangle((int)handlePos.x, (int)handlePos.y, (int)handleSize.x, (int)handleSize.y, handleColor);
		const char* fmt = (operatorType == 'i') ? "%s: %i" : "%s: %.4f";
		DrawText(TextFormat(fmt, name.c_str(), value),
			(int)sliderPos.x,
			(int)(sliderPos.y - textSize - textSliderSeparation),
			(int)textSize,
			WHITE);

		if (isHoveringHandle || isHoveringSlider) {
			isOnTop = true;
			if (IsMouseButtonPressed(1)) {
				handlePos.x = initialHandlePosX;
				value = static_cast<T>(initialValue);
			}
		}
		else {
			isOnTop = false;
		}

		return isOnTop;
	};

private:
	bool isHoveringSlider;
	bool isHoveringHandle;
	bool isOnTop;
	double initialValue;
	float textSize = 17.0f;
	int textSliderSeparation = 8;

	float initialMouseX;
	bool isClicked;

	bool setInitialPos = true;
	float initialHandlePosX;

	char operatorType;

	Vector2 handlePos;
	Vector2 handleSize;
	Color handleColor;
};
