#pragma once

struct SPHMaterial {

	uint32_t id = 0;
	std::string sphLabel = "material";

	// Base values
	float massMult = 1.0f;
	float restDens = 0.008f;
	float stiff = 1.0f;
	float visc = 1.0f;
	float cohesion = 1.0f;
	Color color = { 255, 255, 255, 255 };

	// Hot values
	float hotPoint = 1000.0f;
	float hotRestDens = 0.01f;
	float hotMassMult = 1.0f;
	float hotStiff = 1.0f;
	float hotVisc = 1.0f;
	float hotCohesion = 1.0f;
	Color hotColor = { 255, 100, 100, 255 };

	// Cold values
	float coldPoint = 0.0f;
	float coldRestDens = 0.01f;
	float coldMassMult = 1.0f;
	float coldStiff = 1.0f;
	float coldVisc = 1.0f;
	float coldCohesion = 1.0f;
	Color coldColor = { 255, 255, 255, 255 };

	// Other values
	float heatConductivity = 0.1f;
	float constraintHardness = 1.0f;

	virtual ~SPHMaterial() = default;

	SPHMaterial(uint32_t id_, const std::string& label)
		: id(id_), sphLabel(label) {
	}
};

// DISCLAIMER: Welcome to arbitrary town. What does hotPoint mean? Whatever your heart feels like.
// For example, the hot point of water means boiling in this context. And coldPoint in water's context means freezing
// These parameters actually just do whatever you like them to do at those temperatures
// I'm currently not simulating rock getting vaporized for example, so rock only has "solid" and "liquid"
// For materials that only are either solid or liquid, I set their cold point to 0.0f because the minimum temperature possible is 1.0f in GE

struct SPHWater : public SPHMaterial {
	SPHWater() : SPHMaterial(1, "water") {
		massMult = 0.6f;
		restDens = 0.095f;
		stiff = 1.0f;
		visc = 0.075f;
		cohesion = 0.05f;
		color = { 30, 65, 230, 150 };

		hotPoint = 373.2f;
		hotMassMult = 0.3f;
		hotRestDens = 0.045f;
		hotStiff = 1.0f;
		hotVisc = 0.075f;
		hotCohesion = 0.0f;
		hotColor = { 230, 230, 250, 190 };

		coldPoint = 273.2f;
		coldMassMult = 0.5f;
		coldRestDens = 0.046f;
		coldStiff = 0.6f;
		coldVisc = 2.2f;
		coldCohesion = 2500.0f;
		coldColor = { 230, 230, 240, 250 };

		heatConductivity = 0.15f;
		constraintHardness = 0.2f;
	}
};

struct SPHRock : public SPHMaterial {
	SPHRock() : SPHMaterial(2, "rock") {
		massMult = 4.0f;
		restDens = 0.008f;
		stiff = 1.4f;
		visc = 3.0f;
		cohesion = 1750.0f;
		color = { 150, 155, 160, 255 };

		hotPoint = 1370.0f;
		hotMassMult = 2.8f;
		hotRestDens = 0.005f;
		hotStiff = 1.0f;
		hotVisc = 0.6f;
		hotCohesion = 300.0f;
		hotColor = { 255, 105, 0, 255 };

		coldPoint = 0.0f;
		coldMassMult = massMult;
		coldRestDens = restDens;
		coldStiff = stiff;
		coldVisc = visc;
		coldCohesion = cohesion;
		coldColor = color;

		heatConductivity = 0.02f;
		constraintHardness = 0.57f;
	}
};

struct SPHSand : public SPHMaterial {
	SPHSand() : SPHMaterial(3, "sand") {
		massMult = 2.1f;
		restDens = 0.008f;
		stiff = 1.255f;
		visc = 0.74f;
		cohesion = 1.0f;
		color = { 200, 185, 100, 255 };

		hotPoint = 1200.0f;
		hotMassMult = 1.9f;
		hotRestDens = 0.011f;
		hotStiff = 1.12f;
		hotVisc = 0.6f;
		hotCohesion = 1.0f;
		hotColor = { 255, 105, 0, 255 };

		coldPoint = 0.0f;
		coldMassMult = massMult;
		coldRestDens = restDens;
		coldStiff = stiff;
		coldVisc = visc;
		coldCohesion = cohesion;
		coldColor = color;

		heatConductivity = 0.01f;
		constraintHardness = 0.6f;
	}
};

struct SPHSoil : public SPHMaterial {
	SPHSoil() : SPHMaterial(4, "soil") {
		massMult = 1.9f;
		restDens = 0.008f;
		stiff = 1.0f;
		visc = 2.23f;
		cohesion = 3000.0f;
		color = { 156, 110, 30, 255 };

		hotPoint = 950.0f;
		hotMassMult = 1.8f;
		hotRestDens = 0.013f;
		hotStiff = 0.9f;
		hotVisc = 1.8f;
		hotCohesion = 600.0f;
		hotColor = { 255, 105, 0, 255 };

		coldPoint = 0.0f;
		coldMassMult = massMult;
		coldRestDens = restDens;
		coldStiff = stiff;
		coldVisc = visc;
		coldCohesion = cohesion;
		coldColor = color;

		heatConductivity = 0.02f;
		constraintHardness = 0.3f;
	}
};

struct SPHMud : public SPHMaterial {
	SPHMud() : SPHMaterial(5, "mud") {
		massMult = 2.3f;
		restDens = 0.0095f;
		stiff = 1.0f;
		visc = 0.6f;
		cohesion = 100.0f;
		color = { 106, 60, 3, 255 };

		hotPoint = 1000.0f;
		hotMassMult = 2.1f;
		hotRestDens = 0.011f;
		hotStiff = 1.0f;
		hotVisc = 0.5f;
		hotCohesion = 40.0f;
		hotColor = { 255, 105, 0, 255 };

		coldPoint = 0.0f;
		coldMassMult = massMult;
		coldRestDens = restDens;
		coldStiff = stiff;
		coldVisc = visc;
		coldCohesion = cohesion;
		coldColor = color;

		heatConductivity = 0.1f;
		constraintHardness = 0.2f;
	}
};


struct SPHMaterials {

	static std::vector<std::unique_ptr<SPHMaterial>> materials;

	static std::unordered_map<uint32_t, SPHMaterial*> idToMaterial;

	static void Init() {
		materials.emplace_back(std::make_unique<SPHWater>());
		materials.emplace_back(std::make_unique<SPHRock>());
		materials.emplace_back(std::make_unique<SPHSand>());
		materials.emplace_back(std::make_unique<SPHSoil>());
		materials.emplace_back(std::make_unique<SPHMud>());

		for (auto& mat : materials) {
			idToMaterial[mat->id] = mat.get();
		}
	}
};