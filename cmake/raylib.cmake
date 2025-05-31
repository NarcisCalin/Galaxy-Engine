set(RL_VERSION 5.5)
set(RL_URL_BASE https://github.com/raysan5/raylib/releases/download)

if(LINUX)
    set(RL_URL ${RL_URL_BASE}/${RL_VERSION}/raylib-${RL_VERSION}_linux_amd64.tar.gz)
elseif(WINDOWS)
    set(RL_URL ${RL_URL_BASE}/${RL_VERSION}/raylib-${RL_VERSION}_win64_msvc16.zip)
endif()

FetchContent_Declare(raylib-fetch URL ${RL_URL})
FetchContent_MakeAvailable(raylib-fetch)

add_library(raylib-lib INTERFACE)

add_dependencies(raylib-lib raylib-fetch)

target_include_directories(raylib-lib INTERFACE ${raylib-fetch_SOURCE_DIR}/include)

target_link_directories(raylib-lib INTERFACE ${raylib-fetch_SOURCE_DIR}/lib)
target_link_libraries(raylib-lib INTERFACE raylib)
