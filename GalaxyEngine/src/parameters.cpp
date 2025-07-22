#include "parameters.h"

// Background color
ImVec4 UpdateVariables::colWindowBg = ImVec4(0.05f, 0.043f, 0.071f, 0.9f);

// Button colors
ImVec4 UpdateVariables::colButton = ImVec4(0.22f, 0.23f, 0.36f, 1.0f);
ImVec4 UpdateVariables::colButtonHover = ImVec4(0.3f, 0.4f, 0.8f, 1.0f);
ImVec4 UpdateVariables::colButtonPress = ImVec4(0.5f, 0.6f, 0.9f, 1.0f);

ImVec4 UpdateVariables::colButtonActive = ImVec4(0.25f, 0.6f, 0.2f, 1.0f);
ImVec4 UpdateVariables::colButtonActiveHover = ImVec4(0.35f, 0.7f, 0.3f, 1.0f);
ImVec4 UpdateVariables::colButtonActivePress = ImVec4(0.45f, 0.8f, 0.4f, 1.0f);

ImVec4 UpdateVariables::colButtonRedActive = ImVec4(0.65f, 0.2f, 0.2f, 1.0f);
ImVec4 UpdateVariables::colButtonRedActiveHover = ImVec4(0.75f, 0.3f, 0.3f, 1.0f);
ImVec4 UpdateVariables::colButtonRedActivePress = ImVec4(0.85f, 0.4f, 0.4f, 1.0f);

// Slider Colors
ImVec4 UpdateVariables::colSliderGrab = ImVec4(0.32f, 0.33f, 0.46f, 1.0f);
ImVec4 UpdateVariables::colSliderGrabActive = ImVec4(0.3f, 0.5f, 0.9f, 1.0f);
ImVec4 UpdateVariables::colSliderBg = ImVec4(0.12f, 0.13f, 0.26f, 1.0f);
ImVec4 UpdateVariables::colSliderBgHover = ImVec4(0.22f, 0.23f, 0.36f, 1.0f);
ImVec4 UpdateVariables::colSliderBgActive = ImVec4(0.42f, 0.43f, 0.66f, 1.0f);

// Plotline Colors
ImVec4 UpdateVariables::colPlotLine = ImVec4(0.68f, 0.7f, 0.9f, 1.0f);
ImVec4 UpdateVariables::colAxisText = ImVec4(1.0f, 0.8f, 1.0f, 1.0f);
ImVec4 UpdateVariables::colAxisGrid = ImVec4(0.4f, 0.5f, 0.6f, 1.0f);
ImVec4 UpdateVariables::colAxisBg = ImVec4(0.1f, 0.1f, 0.2f, 1.0f);
ImVec4 UpdateVariables::colFrameBg = ImVec4(0.12f, 0.12f, 0.2f, 1.0f);
ImVec4 UpdateVariables::colPlotBg = ImVec4(0.05f, 0.05f, 0.1f, 1.0f);
ImVec4 UpdateVariables::colPlotBorder = ImVec4(1.0f, 0.0f, 1.0f, 1.0f);
ImVec4 UpdateVariables::colLegendBg = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);

// Text Colors
ImVec4 UpdateVariables::colMenuInformation = ImVec4(0.77f, 0.77f, 0.97f, 1.0f);

// SPH Materials vector definition
std::vector<std::unique_ptr<SPHMaterial>> SPHMaterials::materials;
std::unordered_map<uint32_t, SPHMaterial*> SPHMaterials::idToMaterial;

// Global particle base mass
float UpdateVariables::particleBaseMass = 8500000000.0f;