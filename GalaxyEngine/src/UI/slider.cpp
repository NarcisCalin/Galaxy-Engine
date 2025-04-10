#include "../../include/UI/slider.h"

Slider::Slider(Vector2 sliderPos, Vector2 sliderSize, Color sliderColor, std::string name){
	this->sliderPos = sliderPos;
	this->sliderSize = sliderSize;
	this->sliderColor = sliderColor;
	this->name = name;

	this->handleColor = {
			static_cast<unsigned char>(sliderColor.r * 1.2f),
			static_cast<unsigned char>(sliderColor.g * 1.2f),
			static_cast<unsigned char>(sliderColor.b * 1.2f),
			255
	};

	this->handlePos = { 0, 0 };
	this->handleSize = { 0, 0 };
	this->initialHandlePosX = 0.0f;
	this->initialMouseX = 0.0f;
	this->isClicked = false;
	this->isHoveringHandle = false;
	this->isHoveringSlider = false;
	this->isOnTop = false;
	this->maxHandlePos = { 0, 0 };
	this->minHandlePos = { 0, 0 };
	this->operatorType = '\0';
}
