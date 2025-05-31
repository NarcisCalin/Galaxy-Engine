cmake_minimum_required(VERSION 3.24)

set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
cmake_policy(SET CMP0077 NEW)

cmake_policy(SET CMP0135 NEW)

project(GalaxyEngine)

include(FetchContent)

include(${CMAKE_CURRENT_LIST_DIR}/cmake/ffmpeg.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/glm.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/openmp.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/raylib.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/imgui.cmake)

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
    Physics/SPH.cpp
    UI/brush.cpp
    UI/controls.cpp
    UI/rightClickSettings.cpp
    UI/UI.cpp
    UX/camera.cpp
    UX/screenCapture.cpp
    resources.rc)

list(TRANSFORM GALAXYENGINE_SOURCES PREPEND ${CMAKE_CURRENT_LIST_DIR}/GalaxyEngine/src/)

add_executable(GalaxyEngine ${GALAXYENGINE_SOURCES})

target_compile_features(GalaxyEngine PRIVATE cxx_std_17)

target_link_libraries(GalaxyEngine PUBLIC raylib-lib ffmpeg imgui glm OpenMP::OpenMP_CXX)
