#pragma once
#include <iostream>
#include <array>
#include "../raylib/raylib.h"
#include "../Particles/particle.h"
#include "button.h"
#include "../parameters.h"
#include "slider.h"
#include "../Physics/quadtree.h"
#include "../Physics/SPH.h"
#include "../UX/saveSystem.h"

class UI {
public:

	bool bVisualsSliders = true;
	bool bPhysicsSliders = false;

	void uiLogic(UpdateParameters& myParam, UpdateVariables& myVar, SPH& sph, SaveSystem& save);


	std::array<Button, 29> settingsButtonsArray = {

Button({195.0f, 80.0f}, {175.0f, 20.0f}, "Fullscreen", true),

Button({780.0f, 0.0f}, {200.0f, 50.0f}, "Controls", true),

Button({780.0f, 0.0f}, {200.0f, 50.0f}, "Information", true),

Button({195.0f, 80.0f}, {175.0f, 24.0f}, "Global Trails", true),

Button({780.0f, 0.0f}, {200.0f, 50.0f}, "Selected Trails", true),

Button({780.0f, 0.0f}, {200.0f, 50.0f}, "Local Trails", true),

Button({780.0f, 0.0f}, {200.0f, 50.0f}, "White Trails", true),

Button({780.0f, 0.0f}, {200.0f, 50.0f}, "Solid Color", true),

Button({780.0f, 0.0f}, {200.0f, 50.0f}, "Density Color", true),

Button({780.0f, 0.0f}, {200.0f, 50.0f}, "Force Color", true),

Button({780.0f, 0.0f}, {200.0f, 50.0f}, "Velocity Color", true),

Button({780.0f, 0.0f}, {200.0f, 50.0f}, "Delta V Color", true),

Button({780.0f, 0.0f}, {200.0f, 50.0f}, "SPH Color", true),

Button({780.0f, 0.0f}, {200.0f, 50.0f}, "Selected Color", true),

Button({780.0f, 0.0f}, {200.0f, 50.0f}, "Dark Matter", true),

Button({780.0f, 0.0f}, {200.0f, 50.0f}, "Show Dark M.", true),

Button({780.0f, 0.0f}, {200.0f, 50.0f}, "Looping Space", true),

Button({780.0f, 0.0f}, {200.0f, 50.0f}, "Multi-Threading", true),

Button({780.0f, 0.0f}, {200.0f, 50.0f}, "SPH", true),

Button({780.0f, 0.0f}, {200.0f, 50.0f}, "Collisions (!!!)", true),

Button({780.0f, 0.0f}, {200.0f, 50.0f}, "Density Size", true),

Button({780.0f, 0.0f}, {200.0f, 50.0f}, "Force Size", true),

Button({780.0f, 0.0f}, {200.0f, 50.0f}, "Glow", true),

Button({780.0f, 0.0f}, {200.0f, 50.0f}, "Predict Path", true),

Button({780.0f, 0.0f}, {200.0f, 50.0f}, "Visual Settings", true),

Button({780.0f, 0.0f}, {200.0f, 50.0f}, "Physics Settings", true),

Button({780.0f, 0.0f}, {200.0f, 50.0f}, "Ship Gas", true),

Button({780.0f, 0.0f}, {200.0f, 50.0f}, "Save Scene", true),

Button({780.0f, 0.0f}, {200.0f, 50.0f}, "Load Scene", true)

	};


	std::array<Button, 4> SPHMaterialButtonsArray = {

Button({195.0f, 80.0f}, {175.0f, 20.0f}, "SPH Water", true),

Button({195.0f, 80.0f}, {175.0f, 20.0f}, "SPH Rock", true),

Button({195.0f, 80.0f}, {175.0f, 20.0f}, "SPH Sand", true),

Button({195.0f, 80.0f}, {175.0f, 20.0f}, "SPH Mud", true)
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

	std::array<Slider, 19> visualsSliders = {

	Slider({20, 530.0f}, {230.0f, 7.0f}, {190, 100, 100, 255}, "Primary Red"),

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

	Slider({450.0f, 450.0f}, {250.0f, 10.0f}, {128, 128, 128, 255}, "Max DeltaV Accel"),

	Slider({450.0f, 450.0f}, {250.0f, 10.0f}, {128, 128, 128, 255}, "Trails Length"),

	Slider({450.0f, 450.0f}, {250.0f, 10.0f}, {128, 128, 128, 255}, "Trails Thickness"),

	Slider({450.0f, 450.0f}, {250.0f, 10.0f}, {128, 128, 128, 255}, "Particles Size"),

	Slider({450.0f, 450.0f}, {250.0f, 10.0f}, {128, 128, 128, 255}, "Path Prediction Length"),

	Slider({450.0f, 450.0f}, {250.0f, 10.0f}, {128, 128, 128, 255}, "Particle Amount Multiplier"),

	Slider({450.0f, 450.0f}, {250.0f, 10.0f}, {128, 128, 128, 255}, "DM Amount Multiplier")

	};

	std::array<Slider, 14> physicsSliders = {

	Slider({20, 530.0f}, {230.0f, 7.0f}, {120, 128, 128, 255}, "Softening"),

	Slider({450.0f, 450.0f}, {250.0f, 10.0f}, {128, 128, 128, 255}, "Theta"),

	Slider({450.0f, 450.0f}, {250.0f, 10.0f}, {128, 128, 128, 255}, "Time Scale"),

	Slider({450.0f, 450.0f}, {250.0f, 10.0f}, {128, 128, 128, 255}, "Collision Substeps"),

	Slider({450.0f, 450.0f}, {250.0f, 10.0f}, {128, 128, 128, 255}, "Gravity Strength"),

	Slider({450.0f, 450.0f}, {250.0f, 10.0f}, {128, 128, 128, 255}, "Heavy Particle Init Mass"),

	Slider({450.0f, 450.0f}, {250.0f, 10.0f}, {128, 128, 128, 255}, "Collision Bounciness"),

	Slider({450.0f, 450.0f}, {250.0f, 10.0f}, {128, 128, 128, 255}, "Threads Amount"),

	Slider({450.0f, 450.0f}, {250.0f, 10.0f}, {128, 128, 128, 255}, "SPH Rest Pressure"),

	Slider({450.0f, 450.0f}, {250.0f, 10.0f}, {128, 128, 128, 255}, "SPH Stiffness"),

	Slider({450.0f, 450.0f}, {250.0f, 10.0f}, {128, 128, 128, 255}, "SPH Radius"),

	Slider({450.0f, 450.0f}, {250.0f, 10.0f}, {128, 128, 128, 255}, "SPH Mass Multiplier"),

	Slider({450.0f, 450.0f}, {250.0f, 10.0f}, {128, 128, 128, 255}, "SPH Viscosity"),

	Slider({450.0f, 450.0f}, {250.0f, 10.0f}, {128, 128, 128, 255}, "SPH Cohesion")

	};

	bool showSettings = true;

};