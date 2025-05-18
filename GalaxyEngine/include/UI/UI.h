#pragma once
#include <iostream>
#include <array>
#include "../raylib/raylib.h"
#include "../Particles/particle.h"
#include "../parameters.h"
#include "../Physics/quadtree.h"
#include "../Physics/SPH.h"
#include "../UX/saveSystem.h"
#include "../../external/imgui/imgui.h"
#include "../../external/imgui/rlImGui.h"
#include "../../external/imgui/rlImGuiColors.h"

struct settingsParams {
	std::string text;
	bool& parameter;

	settingsParams(const std::string& t, bool& p) : text(t), parameter(p) {}
};

template<typename T>
struct visualSlidersParams {
	std::string text;
	T& parameter;
	T min;
	T max;
	const T defaultVal;
	visualSlidersParams(const std::string& t, T& p, T min, T max)
		: text(t), parameter(p), min(min), max(max), defaultVal(p) {
	}
};

template<typename T>
struct physicsSlidersParams {
	std::string text;
	T& parameter;
	T min;
	T max;
	const T defaultVal;
	physicsSlidersParams(const std::string& t, T& p, T min, T max)
		: text(t), parameter(p), min(min), max(max), defaultVal(p) {
	}
};

struct sphParams {
	std::string text;
	bool& parameter;

	sphParams(const std::string& t, bool& p) : text(t), parameter(p) {}
};

class UI {
public:

	bool bVisualsSliders = true;
	bool bPhysicsSliders = false;

	void uiLogic(UpdateParameters& myParam, UpdateVariables& myVar, SPH& sph, SaveSystem& save);


	std::array<Button, 1> toggleSettingsButtons = {
	Button
	(
		{ 34.0f, 65.0f },
		{ 14.0f,14.0f },
		"",
		false
	)
	};

	bool showSettings = true;

private:
	bool loadSettings = true;

};