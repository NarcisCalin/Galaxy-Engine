# Galaxy Engine

## About

Join Galaxy Engine's [Discord community!](https://discord.gg/Xd5JUqNFPM)

Check out the official [Steam page!](https://store.steampowered.com/app/3762210/Galaxy_Engine/)

Galaxy Engine is a free and open source project made for learning purposes by [NarcisCalin](https://github.com/NarcisCalin). It is made with C++ and it is built on top of the [Raylib](https://github.com/raysan5/raylib) library.

Galaxy Engine uses libraries from the [FFmpeg](https://github.com/FFmpeg/FFmpeg) project under the LGPLv2.1

The entire UI is made with the help of the [Dear ImGui](https://github.com/ocornut/imgui) library.

Special thanks to [Crisosphinx](https://github.com/crisosphinx) for helping me improve my code.

Special thanks to [SergioCelso](https://github.com/SCelso) for making the learning process a little easier and also for helping me implement the initial version of the Barnes-Hut quadtree.

Special thanks to [Emily](https://github.com/Th3T3chn0G1t) for helping me on multiple occasions, including setting up the CMake configuration and assisting with various parts of the project.



---

<img src="https://github.com/user-attachments/assets/d26ca9b3-51f6-4744-9561-4fab50ee0a96" width="1280">

![GalaxyCollisionField](https://github.com/user-attachments/assets/fa6d4eb8-7986-49dd-b274-e78ba03614bc)

![PlanetaryCollision](https://github.com/user-attachments/assets/15cd619c-3274-445c-9618-de88055d4ff0)

## Features
---
### INTERACTIVITY
Galaxy Engine was built with interactivity in mind

<img src="https://github.com/user-attachments/assets/affe69df-3d86-4593-baa4-80e7d7dc1372" width="1280">

![Interactivity](https://github.com/user-attachments/assets/ef3077a5-91c1-43fd-aa30-516b3e84fabc)

### SPH FLUID PHYSICS
Galaxy Engine includes SPH fluid physics for different types of simulation

<img src="https://github.com/user-attachments/assets/637dc0c9-2bce-4a5f-b93b-8d4036c6349a" width="1280">

![PlanetsHQ](https://github.com/user-attachments/assets/0bbe27c9-c61d-435e-b20d-9cd055591e41)

![PlanetaryHQ2](https://github.com/user-attachments/assets/b185971f-0463-4086-831d-713c1d1c1a9c)

![AsteroidImpact](https://github.com/user-attachments/assets/d20d97a9-aaf7-4909-b73e-50d9945c17d3)

### REAL TIME PHYSICS
Thanks to the Barnes-Hut algorithm, Galaxy Engine can simulate tens or even hundreds of thousands of particles in real time

![BH](https://github.com/user-attachments/assets/d1db1b59-ce03-46c5-9360-0a7b10199de4)

### RENDERING
Engine Galaxy includes a recorder so you can compile videos showcasing millions of particles

![20Mil](https://github.com/user-attachments/assets/454af8b4-c8d5-4f9b-b2d6-a831c1dcc9f8)


## HOW TO INSTALL
---
- Download the latest release version from the releases tab and unzip the zip file.
- Run the executable file inside the folder.
- (Currently Galaxy Engine binaries are only available on Windows)

### IMPORTANT INFORMATION
- Galaxy Engine releases might get flagged as a virus by your browser or by Windows Defender. This is normal. It happens mainly because the program is not signed by Microsoft (Which costs money). If you don't trust the binaries, you can always compile Galaxy Engine yourself

## HOW TO BUILD
---

### Dependencies
- [CMake](https://cmake.org/): On Windows, you can install it from [their website](https://cmake.org/download/) or the [Chocolatey cmake package](https://community.chocolatey.org/packages/cmake), and on Linux you can install the `cmake` package with your package manager.
- A C++ compiler: Any compiler is probably gonna work, but Clang is recommended as it's known to produce a faster binary than GCC for this project.
  - [Clang](https://clang.llvm.org/): On Linux, you may install the `clang` package from a package manager. On Windows, it can be installed from [their website](https://clang.llvm.org/get_started.html), the [Chocolatey llvm package](https://community.chocolatey.org/packages/llvm), or from the Visual Studio Installer: 

    ![image](https://github.com/user-attachments/assets/b46a0e7d-188e-43a3-bf7e-fb3edced233a)

### Basic instructions
These instructions assume you have already met the above requirements.

- Clone or download this repo
- Build the project with CMake
- After doing this you should have the executable file inside the build folder
- The last step is running the executable file from the same working directory as the "Textures" folder (otherwise the particles won't be visible). This applies to the "Shaders" and "fonts" folder as well
