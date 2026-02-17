FetchContent_Declare(
    imgui-fetch
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        docking
)
FetchContent_MakeAvailable(imgui-fetch)

FetchContent_Declare(
    implot-fetch
    GIT_REPOSITORY https://github.com/epezent/implot.git
    GIT_TAG        v0.17
)
FetchContent_MakeAvailable(implot-fetch)

FetchContent_Declare(
    rlgui-fetch
    GIT_REPOSITORY https://github.com/raylib-extras/rlImGui.git
    GIT_TAG        main
)
FetchContent_MakeAvailable(rlgui-fetch)

add_library(imgui STATIC)

target_include_directories(imgui PUBLIC ${imgui-fetch_SOURCE_DIR})
target_include_directories(imgui PUBLIC ${implot-fetch_SOURCE_DIR})
target_include_directories(imgui PUBLIC ${rlgui-fetch_SOURCE_DIR})

target_sources(imgui PRIVATE 
    ${imgui-fetch_SOURCE_DIR}/imgui.cpp
    ${imgui-fetch_SOURCE_DIR}/imgui_tables.cpp
    ${imgui-fetch_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui-fetch_SOURCE_DIR}/imgui_draw.cpp
    ${imgui-fetch_SOURCE_DIR}/imgui_demo.cpp)

target_sources(imgui PRIVATE 
    ${implot-fetch_SOURCE_DIR}/implot.cpp
    ${implot-fetch_SOURCE_DIR}/implot_items.cpp
)

target_sources(imgui PRIVATE 
    ${rlgui-fetch_SOURCE_DIR}/rlImGui.cpp
)

target_link_libraries(imgui PUBLIC raylib-lib)