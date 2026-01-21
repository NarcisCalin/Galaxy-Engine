cmake_minimum_required(VERSION 3.26..3.30 FATAL_ERROR)

set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
cmake_policy(SET CMP0077 NEW)

cmake_policy(SET CMP0135 NEW)

project(GalaxyEngine)

if(MSVC OR CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    add_compile_options("$<$<CONFIG:Debug>:/Od;/RTC1;/MDd>")
    add_compile_options("$<$<CONFIG:Release>:/O2;/MD>")
endif()

include(FetchContent)

include(${CMAKE_CURRENT_LIST_DIR}/cmake/ffmpeg.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/glm.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/openmp.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/raylib.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/imgui.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/yaml.cmake)

set(
    GALAXYENGINE_SOURCES
    main.cpp
    globalLogic.cpp
    parameters.cpp
    Particles/particleSelection.cpp
    Particles/particlesSpawning.cpp
    Particles/particleSubdivision.cpp
    Particles/particleTrails.cpp
    Physics/morton.cpp
    Physics/physics.cpp
    Physics/quadtree.cpp
    Physics/slingshot.cpp
    Physics/SPH.cpp
    Physics/light.cpp
    Sound/sound.cpp
    UI/brush.cpp
    UI/controls.cpp
    UI/rightClickSettings.cpp
    UI/UI.cpp
    UX/camera.cpp
    UX/saveSystem.cpp
    UX/screenCapture.cpp
    UX/randNum.cpp
    resources.rc)

list(TRANSFORM GALAXYENGINE_SOURCES PREPEND ${CMAKE_CURRENT_LIST_DIR}/GalaxyEngine/src/)

add_executable(GalaxyEngine ${GALAXYENGINE_SOURCES})

target_precompile_headers(GalaxyEngine PUBLIC ${CMAKE_CURRENT_LIST_DIR}/GalaxyEngine/include/pch.h)

target_include_directories(GalaxyEngine PUBLIC ${CMAKE_CURRENT_LIST_DIR}/GalaxyEngine/include)

target_compile_features(GalaxyEngine PRIVATE cxx_std_20)
set_target_properties(
    GalaxyEngine PROPERTIES 
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
    VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/GalaxyEngine
)

target_link_libraries(GalaxyEngine PUBLIC raylib-lib ffmpeg imgui glm-lib openmp yaml-cpp::yaml-cpp)

if(WIN32)
    add_custom_command(
        TARGET GalaxyEngine POST_BUILD 
        COMMAND ${CMAKE_COMMAND} -E copy_directory 
        ${ffmpeg-fetch_SOURCE_DIR}/bin 
        $<TARGET_FILE_DIR:GalaxyEngine>
    )
endif()