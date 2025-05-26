cmake_minimum_required(VERSION 3.14)
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
cmake_policy(SET CMP0077 NEW)

# Set the project name
project(GalaxyEngine)

# Windows icon resource setup
if(WIN32)
    set(RC_FILE ${CMAKE_CURRENT_SOURCE_DIR}/resources.rc)
    set(RC_OBJ ${CMAKE_CURRENT_BINARY_DIR}/resources.obj)
    set(ICON_PATH ${CMAKE_CURRENT_SOURCE_DIR}/src/GalaxyEngineIcon.ico)

    # Compile the .rc file using llvm-rc
    add_custom_command(OUTPUT ${RC_OBJ}
        COMMAND llvm-rc /fo ${RC_OBJ} ${RC_FILE}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        DEPENDS ${RC_FILE} ${ICON_PATH}
        COMMENT "Compiling icon resource: ${RC_FILE}"
    )

    add_custom_target(icon_resource ALL DEPENDS ${RC_OBJ})
endif()

# Download and build raylib
set(BUILD_EXAMPLES OFF)
include(FetchContent)
FetchContent_Declare(
    raylib
    URL https://github.com/raysan5/raylib/archive/refs/tags/5.5.tar.gz
)
FetchContent_MakeAvailable(raylib)

# Detect operating system
if(WIN32)
    set(IS_WINDOWS TRUE)
elseif(UNIX AND NOT APPLE)
    set(IS_LINUX TRUE)
elseif(APPLE)
    set(IS_MACOS TRUE)
endif()

# FFmpeg library configuration
set(FFMPEG_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/ffmpeg" CACHE PATH "FFmpeg directory")

if(IS_WINDOWS)
    # Windows-specific FFmpeg setup
    set(FFMPEG_INCLUDE_DIR "${FFMPEG_DIR}/include" CACHE PATH "Directory containing FFmpeg headers")
    set(FFMPEG_LIBRARY_DIR "${FFMPEG_DIR}/lib" CACHE PATH "Directory containing FFmpeg libraries")
    set(FFMPEG_DLL_DIR "${FFMPEG_DIR}/bin" CACHE PATH "Directory containing FFmpeg DLLs")
    
    # Find FFmpeg libraries (Windows uses .lib files for dynamic linking)
    find_library(AVCODEC_LIBRARY avcodec PATHS ${FFMPEG_LIBRARY_DIR} REQUIRED)
    find_library(AVFORMAT_LIBRARY avformat PATHS ${FFMPEG_LIBRARY_DIR} REQUIRED)
    find_library(AVUTIL_LIBRARY avutil PATHS ${FFMPEG_LIBRARY_DIR} REQUIRED)
    find_library(SWSCALE_LIBRARY swscale PATHS ${FFMPEG_LIBRARY_DIR} REQUIRED)
    find_library(SWRESAMPLE_LIBRARY swresample PATHS ${FFMPEG_LIBRARY_DIR} REQUIRED)
    
    # Windows setup doesn't use pkg-config
    set(FFMPEG_USE_PKG_CONFIG FALSE)
else()
    # Linux and macOS - try to use system FFmpeg
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(FFMPEG
            libavcodec
            libavformat
            libavutil
            libswscale
            libswresample
        )
        
        if(FFMPEG_FOUND)
            set(FFMPEG_USE_PKG_CONFIG TRUE)
            message(STATUS "Found system FFmpeg via pkg-config")
            message(STATUS "  Include dirs: ${FFMPEG_INCLUDE_DIRS}")
            message(STATUS "  Libraries: ${FFMPEG_LIBRARIES}")
            message(STATUS "  Library dirs: ${FFMPEG_LIBRARY_DIRS}")
        else()
            set(FFMPEG_USE_PKG_CONFIG FALSE)
            message(STATUS "System FFmpeg not found via pkg-config, looking for libraries directly")
        endif()
    else()
        set(FFMPEG_USE_PKG_CONFIG FALSE)
        message(STATUS "pkg-config not found, looking for FFmpeg libraries directly")
    endif()
    
    # If pkg-config failed or wasn't found, try manual library detection
    if(NOT FFMPEG_USE_PKG_CONFIG)
        # Try system directories first
        find_library(AVCODEC_LIBRARY avcodec)
        find_library(AVFORMAT_LIBRARY avformat)
        find_library(AVUTIL_LIBRARY avutil)
        find_library(SWSCALE_LIBRARY swscale)
        find_library(SWRESAMPLE_LIBRARY swresample)
        
        # If any library was not found in system path, try custom path
        if(NOT AVCODEC_LIBRARY OR NOT AVFORMAT_LIBRARY OR NOT AVUTIL_LIBRARY OR 
           NOT SWSCALE_LIBRARY OR NOT SWRESAMPLE_LIBRARY)
            message(STATUS "Some FFmpeg libraries not found in system path, using custom path")
            set(FFMPEG_INCLUDE_DIR "${FFMPEG_DIR}/include" CACHE PATH "Directory containing FFmpeg headers")
            set(FFMPEG_LIBRARY_DIR "${FFMPEG_DIR}/lib" CACHE PATH "Directory containing FFmpeg libraries")
            
            # Find FFmpeg libraries
            find_library(AVCODEC_LIBRARY avcodec PATHS ${FFMPEG_LIBRARY_DIR} REQUIRED)
            find_library(AVFORMAT_LIBRARY avformat PATHS ${FFMPEG_LIBRARY_DIR} REQUIRED)
            find_library(AVUTIL_LIBRARY avutil PATHS ${FFMPEG_LIBRARY_DIR} REQUIRED)
            find_library(SWSCALE_LIBRARY swscale PATHS ${FFMPEG_LIBRARY_DIR} REQUIRED)
            find_library(SWRESAMPLE_LIBRARY swresample PATHS ${FFMPEG_LIBRARY_DIR} REQUIRED)
        else()
            message(STATUS "Found system FFmpeg libraries:")
            message(STATUS "  AVCODEC: ${AVCODEC_LIBRARY}")
            message(STATUS "  AVFORMAT: ${AVFORMAT_LIBRARY}")
            message(STATUS "  AVUTIL: ${AVUTIL_LIBRARY}")
            message(STATUS "  SWSCALE: ${SWSCALE_LIBRARY}")
            message(STATUS "  SWRESAMPLE: ${SWRESAMPLE_LIBRARY}")
        endif()
    endif()
endif()

# Create an interface target for FFmpeg
add_library(FFmpeg INTERFACE)

if(FFMPEG_USE_PKG_CONFIG)
    # Use pkg-config results
    target_include_directories(FFmpeg INTERFACE ${FFMPEG_INCLUDE_DIRS})
    target_link_directories(FFmpeg INTERFACE ${FFMPEG_LIBRARY_DIRS})
    target_link_libraries(FFmpeg INTERFACE ${FFMPEG_LIBRARIES})
else()
    # Use libraries found with find_library
    if(IS_LINUX OR IS_MACOS)
        # Find the include directory if not already set
        if(NOT FFMPEG_INCLUDE_DIR)
            find_path(FFMPEG_INCLUDE_DIR libavcodec/avcodec.h
                HINTS /usr/include /usr/local/include)
            if(FFMPEG_INCLUDE_DIR)
                message(STATUS "Found FFmpeg include directory: ${FFMPEG_INCLUDE_DIR}")
            else()
                message(FATAL_ERROR "FFmpeg include directory not found. Please install FFmpeg development packages.")
            endif()
        endif()
    endif()

    target_include_directories(FFmpeg INTERFACE ${FFMPEG_INCLUDE_DIR})
    target_link_libraries(FFmpeg INTERFACE
        ${AVCODEC_LIBRARY}
        ${AVFORMAT_LIBRARY}
        ${AVUTIL_LIBRARY}
        ${SWSCALE_LIBRARY}
        ${SWRESAMPLE_LIBRARY}
    )
endif()

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
    Physics/SPH.cpp
    UI/brush.cpp
    UI/controls.cpp
    UI/rightClickSettings.cpp
    UI/UI.cpp
    UX/camera.cpp
    UX/screenCapture.cpp
)

set(IMGUI_DIR "${CMAKE_CURRENT_LIST_DIR}/external/imgui")

set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/rlImGui.cpp
)

# Prepend the source directory to each source file path
list(TRANSFORM GALAXYENGINE_SOURCES PREPEND ${CMAKE_CURRENT_LIST_DIR}/src/)

# Combine all sources into one list for your executable
set(ALL_SOURCES ${GALAXYENGINE_SOURCES} ${IMGUI_SOURCES})

# Declare the executable with all sources
if(WIN32)
    add_executable(GalaxyEngine ${ALL_SOURCES} ${RC_OBJ})
    
    # For Clang on Windows, we need to use -Xlinker to pass options to the linker
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_link_options(GalaxyEngine PRIVATE -Xlinker /ENTRY:mainCRTStartup)
    else()
        # For MSVC or other compilers
        target_link_options(GalaxyEngine PRIVATE /ENTRY:mainCRTStartup)
    endif()
else()
    add_executable(GalaxyEngine ${ALL_SOURCES})
endif()


# Set C++ version
target_compile_features(GalaxyEngine PRIVATE cxx_std_17)

# Link raylib to the executable
target_link_libraries(GalaxyEngine PUBLIC raylib)

# Link FFmpeg to the executable
target_link_libraries(GalaxyEngine PUBLIC FFmpeg)

# Find OpenMP
find_package(OpenMP)

# If OpenMP is found natively, link it
if(OpenMP_CXX_FOUND)
    message(STATUS "OpenMP found natively")
    target_link_libraries(GalaxyEngine PUBLIC ${OpenMP_CXX_LIBRARIES})
else()
    # Apply manual fixes based on compiler and OS
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        if(IS_WINDOWS)
            message(WARNING "OpenMP not found, applying manual workaround for Clang on Windows")
            
            target_compile_options(GalaxyEngine PUBLIC -fopenmp)
            target_link_options(GalaxyEngine PUBLIC -fopenmp)
            
            # Link the manually provided OpenMP library
            target_link_directories(GalaxyEngine PUBLIC "${CMAKE_SOURCE_DIR}/lib")
            target_link_libraries(GalaxyEngine PUBLIC libomp.lib)
        else()
            message(WARNING "OpenMP not found, applying manual workaround for Clang on Unix")
            
            target_compile_options(GalaxyEngine PUBLIC -fopenmp)
            target_link_options(GalaxyEngine PUBLIC -fopenmp)
            
            # On Linux/macOS we don't need to explicitly link libomp
        endif()
    endif()
endif()

# Copy FFmpeg DLLs to output directory (Windows-only)
if(IS_WINDOWS)
    file(GLOB FFMPEG_DLLS "${FFMPEG_DLL_DIR}/*.dll")
    foreach(DLL_FILE ${FFMPEG_DLLS})
        add_custom_command(TARGET GalaxyEngine POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${DLL_FILE}"
            $<TARGET_FILE_DIR:GalaxyEngine>
        )
    endforeach()
    message(STATUS "Using FFmpeg libraries from: ${FFMPEG_LIBRARY_DIR}")
    message(STATUS "Using FFmpeg DLLs from: ${FFMPEG_DLL_DIR}")
else()
    message(STATUS "Using system FFmpeg libraries")
endif()

# Set runtime path for shared libraries on Linux
if(IS_LINUX)
    set_target_properties(GalaxyEngine PROPERTIES
        INSTALL_RPATH "$ORIGIN/../lib:$ORIGIN"
        BUILD_WITH_INSTALL_RPATH TRUE
    )
endif()