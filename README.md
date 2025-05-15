# Galaxy Engine

## About

Join Galaxy Engine's [Discord community!](https://discord.gg/Xd5JUqNFPM)

Galaxy Engine is a project made for learning purposes by [NarcisCalin](https://github.com/NarcisCalin). It is made with C++ and it uses the [Raylib](https://github.com/raysan5/raylib) library.

Galaxy Engine uses libraries from the [FFmpeg](https://github.com/FFmpeg/FFmpeg) project under the LGPLv2.1

Galaxy Engine is available for free and its code is open source.

Special thanks to [Crisosphinx](https://github.com/crisosphinx) for helping me improve my code.

Special thanks to [SergioCelso](https://github.com/SCelso) for making the learning process a little easier and also for helping me implement the initial version of the Barnes-Hut quadtree.

Special thanks to [Emily](https://github.com/Th3T3chn0G1t) for helping me on multiple occasions, including setting up the CMake configuration and assisting with various parts of the project.



---
![ManyGalaxiesGif](https://github.com/user-attachments/assets/70b784ac-22ab-4261-9cf6-5e651492cf6f)


![Brush](https://github.com/user-attachments/assets/b68eb4cb-9ce4-4dfb-b366-0607598cebcc)

![BigBangBanner](https://github.com/user-attachments/assets/a3e2bef5-d9b0-4175-91f6-7edd0d532a44)

## Features
---
### INTERACTIVITY
Galaxy Engine was built with interactivity in mind



https://github.com/user-attachments/assets/def3a318-fcf7-47c3-9366-42e9edcef53a



https://github.com/user-attachments/assets/4c33d196-cec2-4538-a5ca-67de1621603e




### CUSTOMIZATION
There are multiple parameters to allow you to achieve the look you want






https://github.com/user-attachments/assets/f30c097a-3455-424c-832e-dae0d1360cb3






### DYNAMIC CAMERA
Follow individual particles or entire clusters



https://github.com/user-attachments/assets/514ef7b0-d4d0-485e-9f92-d3fc6ae9c2b5





https://github.com/user-attachments/assets/720aaec9-8558-4717-bc7f-d6c590a3d4c8






### SUBDIVISION
Add more quality to the simulation easily by subdividing the particles




https://github.com/user-attachments/assets/b60d88f4-22f4-497d-8818-72e12e3a4934





### REAL TIME PHYSICS
Thanks to the Barnes-Hut algorithm, Galaxy Engine can simulate tens or even hundreds of thousands of particles in real time




https://github.com/user-attachments/assets/dc8823d6-69be-4934-b9d1-f04cd039b41b





### RENDERING
Engine Galaxy includes a recorder so you can compile videos showcasing millions of particles




https://github.com/user-attachments/assets/9b51780c-7dff-4e53-965d-e5f29677669d





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
