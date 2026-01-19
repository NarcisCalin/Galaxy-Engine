#pragma once

#include "Particles/particle.h"

#include "Physics/quadtree.h"
#include "Physics/SPH.h"
#include "Physics/light.h"

#include "UX/saveSystem.h"

#include "Sound/sound.h"

#include "parameters.h"

#include "Physics/field.h"

class SaveSystem;

struct settingsParams {
	std::string text;
	std::string tooltip;
	bool& parameter;

	settingsParams(const std::string& t, const std::string& tool, bool& p) : text(t), tooltip(tool), parameter(p) {}
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
	bool bRecordingSettings = false;
	bool bStatsWindow = false;
	bool bSoundWindow = false;
	bool bLightingWindow = false;

	bool prevSPHState = false;
	bool prevMassMultiplier = false;

	void uiLogic(UpdateParameters& myParam, UpdateVariables& myVar, SPH& sph, SaveSystem& save, GESound& geSound, Lighting& lighting, Field& field);

	void statsWindowLogic(UpdateParameters& myParam, UpdateVariables& myVar);

	static void plotLinesHelper(const float& timeFactor, std::string label,
		const int length,
		float value, const float minValue, const float maxValue, ImVec2 size);


	static bool buttonHelper(std::string label, std::string tooltip, bool& parameter, float sizeX, float sizeY, bool canDeactivateSelf, 
		bool& isEnabled);

	static bool sliderHelper(std::string label, std::string tooltip, float& parameter, float minVal, float maxVal,
		float sizeX, float sizeY, bool& isEnabled);

	// Int Overload
	static bool sliderHelper(std::string label, std::string tooltip, int& parameter, int minVal, int maxVal,
		float sizeX, float sizeY, bool& isEnabled);

	bool showSettings = true;

private:
	bool loadSettings = true;

	static std::unordered_map<std::string, PlotData> plotDataMap;

	ImVec2 graphDefaultSize = { 340.0f, 250.0f };

	int graphHistoryLimit = 1000;
};

struct SimilarTypeButton {
	struct Mode {
		const char* label;
		const char* tooltip;
		bool* flag;
	};

	static void buttonIterator(std::vector<Mode>& modes, float sizeX, float sizeY, bool canDeactivateSelf, bool& isEnabled) {
		for (size_t i = 0; i < modes.size(); ++i) {
			if (UI::buttonHelper(modes[i].label, modes[i].tooltip, *modes[i].flag, sizeX, sizeY, canDeactivateSelf, isEnabled)) {
				for (size_t j = 0; j < modes.size(); ++j) {
					if (j != i) {
						*modes[j].flag = false;
					}
				}
			}
		}
	}
};