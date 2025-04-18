#pragma once
#include <iostream>
#include <array>
#include "../raylib/raylib.h"
#include "../Particles/particle.h"
#include "button.h"
#include "../parameters.h"
#include "slider.h"

class UI {
public:

	void uiLogic(UpdateParameters& myParam, UpdateVariables& myVar);

private:

	std::array<Button, 21> settingsButtonsArray = {

Button({195.0f, 80.0f}, {175.0f, 24.0f}, "Global Trails", true),

Button({780.0f, 0.0f}, {200.0f, 50.0f}, "Selected Trails", true),

Button({780.0f, 0.0f}, {200.0f, 50.0f}, "Local Trails", true),

Button({780.0f, 0.0f}, {200.0f, 50.0f}, "White Trails", true),

Button({780.0f, 0.0f}, {200.0f, 50.0f}, "Solid Color", true),

Button({780.0f, 0.0f}, {200.0f, 50.0f}, "Density Color", true),

Button({780.0f, 0.0f}, {200.0f, 50.0f}, "Force Color", true),

Button({780.0f, 0.0f}, {200.0f, 50.0f}, "Velocity Color", true),

Button({780.0f, 0.0f}, {200.0f, 50.0f}, "Selected Color", true),

Button({780.0f, 0.0f}, {200.0f, 50.0f}, "Dark Matter", true),

Button({780.0f, 0.0f}, {200.0f, 50.0f}, "Show Dark M.", true),

Button({780.0f, 0.0f}, {200.0f, 50.0f}, "Looping Space", true),

Button({780.0f, 0.0f}, {200.0f, 50.0f}, "Barnes-Hut", true),

Button({780.0f, 0.0f}, {200.0f, 50.0f}, "Multi-Threading", true),

Button({780.0f, 0.0f}, {200.0f, 50.0f}, "Collisions (!!!)", true),

Button({780.0f, 0.0f}, {200.0f, 50.0f}, "Density Size", true),

Button({780.0f, 0.0f}, {200.0f, 50.0f}, "Force Size", true),

Button({780.0f, 0.0f}, {200.0f, 50.0f}, "Glow", true),

Button({780.0f, 0.0f}, {200.0f, 50.0f}, "Predict Path", true),

Button({780.0f, 0.0f}, {200.0f, 50.0f}, "Controls", true),

Button({780.0f, 0.0f}, {200.0f, 50.0f}, "Information", true)

	};
	std::array<Button, 1> toggleSettingsButtons = {
	Button
	(
		{ 34.0f, 65.0f },
		{ 14.0f,14.0f },
		"",
		false
	)
	};

	std::array<Slider, 21> slidersArray = {
		Slider
	({20, 530.0f}, {230.0f, 7.0f}, {190, 100, 100, 255}, "Primary Red"),

	Slider({450.0f, 450.0f}, {250.0f, 10.0f}, {100, 190, 100, 255}, "Primary Green"),

	Slider({450.0f, 450.0f}, {250.0f, 10.0f}, {100, 100, 190, 255}, "Primary Blue"),

	Slider({450.0f, 450.0f}, {250.0f, 10.0f}, {128, 128, 128, 255}, "Primary Alpha"),

	Slider({450.0f, 450.0f}, {250.0f, 10.0f}, {150, 80, 80, 255}, "Secondary Red"),

	Slider({450.0f, 450.0f}, {250.0f, 10.0f}, {80, 150, 80, 255}, "Secondary Green"),

	Slider({450.0f, 450.0f}, {250.0f, 10.0f}, {80, 80, 150, 255}, "Secondary Blue"),

	Slider({450.0f, 450.0f}, {250.0f, 10.0f}, {108, 108, 108, 255}, "Secondary Alpha"),

	Slider({450.0f, 450.0f}, {250.0f, 10.0f}, {128, 128, 128, 255}, "Density Radius"),

	Slider({450.0f, 450.0f}, {250.0f, 10.0f}, {128, 128, 128, 255}, "Max Neighbors"),

	Slider({450.0f, 450.0f}, {250.0f, 10.0f}, {128, 128, 128, 255}, "Max Color Force"),

	Slider({450.0f, 450.0f}, {250.0f, 10.0f}, {128, 128, 128, 255}, "Max Size Force"),

	Slider({450.0f, 450.0f}, {250.0f, 10.0f}, {128, 128, 128, 255}, "Softening"),

	Slider({450.0f, 450.0f}, {250.0f, 10.0f}, {128, 128, 128, 255}, "Theta"),

	Slider({450.0f, 450.0f}, {250.0f, 10.0f}, {128, 128, 128, 255}, "Time Scale"),

	Slider({450.0f, 450.0f}, {250.0f, 10.0f}, {128, 128, 128, 255}, "Gravity Strength"),

	Slider({450.0f, 450.0f}, {250.0f, 10.0f}, {128, 128, 128, 255}, "Trails Length"),

	Slider({450.0f, 450.0f}, {250.0f, 10.0f}, {128, 128, 128, 255}, "Trails Thickness"),

	Slider({450.0f, 450.0f}, {250.0f, 10.0f}, {128, 128, 128, 255}, "Particles Size"),

	Slider({450.0f, 450.0f}, {250.0f, 10.0f}, {128, 128, 128, 255}, "Heavy Particle Init Mass"),

	Slider({450.0f, 450.0f}, {250.0f, 10.0f}, {128, 128, 128, 255}, "Path Prediction Length")

	};

	std::array<Slider, 12> initialConditionsArray = {
		Slider({20, 530.0f}, {230.0f, 7.0f}, {190, 100, 100, 255}, "Center Density"),

		Slider({450.0f, 450.0f}, {250.0f, 10.0f}, {128, 128, 128, 255}, "Inner Radius"),

		Slider({450.0f, 450.0f}, {250.0f, 10.0f}, {128, 128, 128, 255}, "Outer Radius"),

		Slider({450.0f, 450.0f}, {250.0f, 10.0f}, {128, 128, 128, 255}, "Top Speed"),

		Slider({450.0f, 450.0f}, {250.0f, 10.0f}, {128, 128, 128, 255}, "Inverse Speed"),

		Slider({450.0f, 450.0f}, {250.0f, 10.0f}, {128, 128, 128, 255}, "Speed Multiplier"),

		Slider({450.0f, 450.0f}, {250.0f, 10.0f}, {128, 128, 128, 255}, "DM Center Density"),

		Slider({450.0f, 450.0f}, {250.0f, 10.0f}, {128, 128, 128, 255}, "DM Inner Radius"),

		Slider({450.0f, 450.0f}, {250.0f, 10.0f}, {128, 128, 128, 255}, "DM Outer Radius"),

		Slider({450.0f, 450.0f}, {250.0f, 10.0f}, {128, 128, 128, 255}, "DM Top Speed"),

		Slider({450.0f, 450.0f}, {250.0f, 10.0f}, {128, 128, 128, 255}, "DM Inverse Speed"),

		Slider({450.0f, 450.0f}, {250.0f, 10.0f}, {128, 128, 128, 255}, "DM Speed Multiplier")
	};

	bool showSettings = true;

};