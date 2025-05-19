# Galaxy Engine

## About

Join Galaxy Engine's [Discord community!](https://discord.gg/Xd5JUqNFPM)

Galaxy Engine is a project made for learning purposes by [NarcisCalin](https://github.com/NarcisCalin). It is made with C++ and it uses the [Raylib](https://github.com/raysan5/raylib) library.

Galaxy Engine uses libraries from the [FFmpeg](https://github.com/FFmpeg/FFmpeg) project under the LGPLv2.1

The entire UI is made with the help of the [Dear ImGui](https://github.com/ocornut/imgui) library.

Galaxy Engine is available for free and its code is open source.

Special thanks to [Crisosphinx](https://github.com/crisosphinx) for helping me improve my code.

Special thanks to [SergioCelso](https://github.com/SCelso) for making the learning process a little easier and also for helping me implement the initial version of the Barnes-Hut quadtree.

Special thanks to [Emily](https://github.com/Th3T3chn0G1t) for helping me on multiple occasions, including setting up the CMake configuration and assisting with various parts of the project.



---
![ManyGalaxiesGif](https://github.com/user-attachments/assets/70b784ac-22ab-4261-9cf6-5e651492cf6f)

![SPH1](https://github.com/user-attachments/assets/926cdae3-43f9-48a5-9f97-5aa2dd4785ec)


![BigBangBanner](https://github.com/user-attachments/assets/a3e2bef5-d9b0-4175-91f6-7edd0d532a44)

## Features
---
### INTERACTIVITY
Galaxy Engine was built with interactivity in mind


![Interactivity](https://github.com/user-attachments/assets/3a1ae9ea-f31b-4938-9d55-6b32dbe83bed)


### CUSTOMIZATION
There are multiple parameters to allow you to achieve the look you want


![Customization](https://github.com/user-attachments/assets/3a569153-642e-4e09-beae-5888a6bcfec0)


### SUBDIVISION
Add more quality to the simulation easily by subdividing the particles



![Subdivision](https://github.com/user-attachments/assets/c414549b-920f-45ad-b330-205f94632465)


### SPH FLUID PHYSICS
Galaxy Engine includes SPH fluid physics for planetary simulation

![SPH2](https://github.com/user-attachments/assets/9b990a20-732a-4887-8254-68708faae182)

![SPH4](https://github.com/user-attachments/assets/4a348533-0fa5-4d32-9060-66ad1c32b138)

![SPH3](https://github.com/user-attachments/assets/a99bbd1c-6ad8-4676-ba13-e55e8c934c3c)


### REAL TIME PHYSICS
Thanks to the Barnes-Hut algorithm, Galaxy Engine can simulate tens or even hundreds of thousands of particles in real time



![Quadree](https://github.com/user-attachments/assets/92f7841e-356b-403d-b2f9-bd55c8fef2a4)





### RENDERING
Engine Galaxy includes a recorder so you can compile videos showcasing millions of particles

![Record](https://github.com/user-attachments/assets/7835b2e4-11e0-4906-ad1c-29d2412107e9)

Result:

![RecordResult](https://github.com/user-attachments/assets/496aea86-12f2-4b91-a680-4f9d29c36a44)




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
- [FFmpeg](https://ffmpeg.org/): On Windows, this is included with Galaxy Engine; on Linux you will need to either compile it yourself or install it with a package manager.
- [CMake](https://cmake.org/): On Windows, you can install it from [their website](https://cmake.org/download/) or the [Chocolatey cmake package](https://community.chocolatey.org/packages/cmake), and on Linux you can install the `cmake` package with your package manager.
- A C++ compiler: Any compiler is probably gonna work, but MSVC is known to have issues outside debug builds, and Clang is recommended as it's known to produce a faster binary than GCC for this project.
  - [Clang](https://clang.llvm.org/): On Linux, you may install the `clang` package from a package manager. On Windows, it can be installed from [their website](https://clang.llvm.org/get_started.html), the [Chocolatey llvm package](https://community.chocolatey.org/packages/llvm), or from the Visual Studio Installer: 

    ![image](https://github.com/user-attachments/assets/b46a0e7d-188e-43a3-bf7e-fb3edced233a)

### Basic instructions
These instructions assume you have already met the above requirements.

- Clone or download this repo
- **Linux only:** Build FFmpeg for this project if you don't intend to install it. (If you intend to have your code under LGPL, you should build the LGPL version of FFmpeg)
- Build the project with CMake
- After doing this you should have the executable file inside the build folder
- The last step is running the executable file from the same working directory as the "Textures" folder (otherwise the particles won't be visible). This applies to the "Shaders" folder as well
