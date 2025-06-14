set(RL_VERSION 5.5)
set(RL_URL_BASE https://github.com/raysan5/raylib/releases/download)

FetchContent_Declare(raylib
GIT_REPOSITORY https://github.com/raysan5/raylib.git
GIT_TAG master)
FetchContent_MakeAvailable(raylib)

add_library(raylib-lib INTERFACE)

target_include_directories(raylib-lib INTERFACE ${raylib_SOURCE_DIR}/src)
target_link_libraries(raylib-lib INTERFACE raylib)
