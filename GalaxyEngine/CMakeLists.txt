cmake_minimum_required(VERSION 3.14)

# Set the project name. This is not the executable program's name!
project(GalaxyEngine)

# Try to find a locally installed raylib, but don't quit on fail
find_package(raylib 5.5 QUIET)

# This code downloads raylib into a directory called _deps and adds it as a subdirectory, compiling it with the program when running the build command
include(FetchContent)
if (NOT raylib_FOUND)
    FetchContent_Declare(
        raylib
        URL https://github.com/raysan5/raylib/archive/refs/tags/5.5.tar.gz
        DOWNLOAD_EXTRACT_TIMESTAMP True #This option is not required but suppresses a warning
    )
    FetchContent_MakeAvailable(raylib)
endif()

# We don't want raylib's examples built. This option is picked up by raylib's CMakeLists.txt
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

file(GLOB_RECURSE SRC_FILES "${PROJECT_SOURCE_DIR}/src/*.cpp")

# Here, the executable is declared with its sources. "main", or "main.exe" on windows will be the program's name
add_executable(GalaxyEngine ${SRC_FILES})

# Link raylib to main
target_link_libraries(GalaxyEngine
    raylib
)
# Make main find the <raylib.h> header (and others)
target_include_directories(GalaxyEngine PUBLIC "${raylib_SOURCE_DIR}/src")