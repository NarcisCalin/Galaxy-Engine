# Code Quick Guide
This is a quick guide for modding or contributing to the development of Galaxy Engine. Please try to follow this guide as much as possible. Before starting to mod or contribute to Galaxy Engine, please check the [roadmap](https://github.com/users/NarcisCalin/projects/1/views/1).

## General
- Galaxy Engine uses ***camelCase***.
- Various classes for different purposes can be added to the **"UpdateParameters"** struct in **"parameters.h"**. This is not necessary for important or complex classes like **"SPH"**, **"Physics"**, or **"UI"**.
- Global variables go to the "UpdateVariables" struct in **"parameters.h"**.
- When you pass the "UpdateParameters" or "UpdateVariables" structs to a function, **ALWAYS** name them **"myParam"** and **"myVar"**.
- All added and removed parameters that are intended to be saved when clicking the **"Save Scene"** button must be added to the **"saveSystem.h"** save and load functions.
- Keep physics and similar features inside **"updateScene();"**.
- Keep visual features (like UI, particle visuals, etc.) inside **"drawScene();"**.
- For now, the current vector struct used in Galaxy Engine is Raylib's **"Vector2"**.
- Colors currently use Raylib's **"Color"** struct.
- The current physics are built on top of a base framerate of 144 FPS.
- Try to use float instead of double. Only exception is the **"G"** constant.
- Things from files like images, fonts, etc., that are loaded into memory must be unloaded when the program closes.
- Before doing a pull request, you must check that your code runs. Please try to solve any warnings you might have before doing a pull request too.

## Particles
- Particles are composed of 2 structs.
- **"ParticlePhysics()"** is used for physics calculations only.
- **"ParticleRendering()"** is used for particle visuals like **"color"** or **"size"**, and distinctive attributes like **"isDarkMatter"** or **"canBeResized"**.
- There is one std::vector for each struct. They are named **"pParticles"** and **"rParticles"**.
- These std::vectors must always be synchronized, meaning that the position of a particle's **pParticle** at index **N**, must be the same number as its **rParticle**.
- All particles are sorted spatially with Z-Curves. This means that particles' index inside a vector or array changes constantly between frames.
- All added or removed variables of a particle's class must be updated in their stream operators as well so that they can be saved by the save system to a file.

## UI
- The Galaxy Engine UI is made entirely using the [Dear ImGui](https://github.com/ocornut/imgui) library. Please check their documentation.
- All ImGui UI elements must go after **"rlImGuiBegin();"** and before **"rlImGuiEnd();"**, which can be found in the main while loop.
- Please try to keep all UI elements inside the **"uiLogic()"** function. There can be exceptions for certain features.
- For most UI elements just keep the default ImGui font. For bigger text, please use the **"Roboto-Medium"** font. This can be found inside **"UpdateVariables"**.
- Keep global buttons in the menu on the right.
- To use Roboto-Medium or change font size for a window, you can do it like this:

![image](https://github.com/user-attachments/assets/4f70e09d-cbd8-46ae-a960-96cb5a9f57c4)
