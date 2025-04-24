cmake_minimum_required(VERSION 3.14)

set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
cmake_policy(SET CMP0077 NEW)

# Set the project name. This is not the executable program's name!
project(GalaxyEngine)

# This code downloads raylib into a directory called _deps and adds it as a subdirectory, compiling it with the program when running the build command
include(FetchContent)
    FetchContent_Declare(
        raylib
        URL https://github.com/raysan5/raylib/archive/refs/tags/5.5.tar.gz
    )
    FetchContent_MakeAvailable(raylib)

# We don't want raylib's examples built. This option is picked up by raylib's CMakeLists.txt
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

# Collect source files for the executable
set(
    GALAXYENGINE_SOURCES
    main.cpp
    Particles/particleSelection.cpp
    Particles/particlesSpawning.cpp
    Particles/particleSubdivision.cpp
    Particles/particleTrails.cpp
    Physics/collisionGrid.cpp
    Physics/morton.cpp
    Physics/physics.cpp
    Physics/quadtree.cpp
    Physics/slingshot.cpp
    UI/brush.cpp
    UI/button.cpp
    UI/controls.cpp
    UI/rightClickSettings.cpp
    UI/slider.cpp
    UI/UI.cpp
    UX/camera.cpp
    UX/screenCapture.cpp
)

# Prepend the source directory to each source file path
list(TRANSFORM GALAXYENGINE_SOURCES PREPEND ${CMAKE_CURRENT_LIST_DIR}/src/)

# Declare the executable with its sources
add_executable(GalaxyEngine ${GALAXYENGINE_SOURCES})

# Set C++ version
target_compile_features(GalaxyEngine PRIVATE cxx_std_17)      # Ask for C++17

# Link raylib to the executable
target_link_libraries(GalaxyEngine PUBLIC raylib)

# Find OpenMP
find_package(OpenMP)

# If OpenMP is found natively, link it
if(OpenMP_CXX_FOUND)
    message(STATUS "OpenMP found natively")
    target_link_libraries(GalaxyEngine PUBLIC OpenMP::OpenMP_CXX)
else()
    # Apply manual fix for Clang on Windows if OpenMP is not found
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        message(WARNING "OpenMP not found, applying manual workaround for Clang")

        target_compile_options(GalaxyEngine PUBLIC -fopenmp)
        target_link_options(GalaxyEngine PUBLIC -fopenmp)

        # Link the manually provided OpenMP library
        target_link_directories(GalaxyEngine PUBLIC "${CMAKE_SOURCE_DIR}/lib")
        target_link_libraries(GalaxyEngine PUBLIC libomp.lib)
    endif()
endif()