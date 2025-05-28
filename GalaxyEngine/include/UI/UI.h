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
	std::string tooltip;
	bool& parameter;

	settingsParams(const std::string& t, const std::string& tool, bool& p) : text(t), tooltip(tool), parameter(p) {}
};

template<typename T>
struct visualSlidersParams {
	std::string text;
	std::string tooltip;
	T& parameter;
	T min;
	T max;
	const T defaultVal;
	visualSlidersParams(const std::string& t, const std::string& tool, T& p, T min, T max)
		: text(t), tooltip(tool), parameter(p), min(min), max(max), defaultVal(p) {
	}
};

template<typename T>
struct physicsSlidersParams {
	std::string text;
	std::string tooltip;
	T& parameter;
	T min;
	T max;
	const T defaultVal;
	physicsSlidersParams(const std::string& t, const std::string& tool, T& p, T min, T max)
		: text(t), tooltip(tool), parameter(p), min(min), max(max), defaultVal(p) {
	}
};

struct sphParams {
	std::string text;
	bool& parameter;

	sphParams(const std::string& t, bool& p) : text(t), parameter(p) {}
};

struct PlotData {
	std::vector<float> values;
	int offset = 0;
};

class UI {
public:

	bool bVisualsSliders = true;
	bool bPhysicsSliders = false;
	bool statsWindow = false;

	void uiLogic(UpdateParameters& myParam, UpdateVariables& myVar, SPH& sph, SaveSystem& save);

	void statsWindowLogic(UpdateParameters& myParam, UpdateVariables& myVar);

	void plotLinesHelper(std::string label,
		const int length,
		float value, const float minValue, const float maxValue, ImVec2 size);

	bool showSettings = true;

private:
	bool loadSettings = true;

	std::unordered_map<std::string, PlotData> plotDataMap;

};