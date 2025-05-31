set(IMGUI_VERSION v1.91.9b)
set(IMGUI_URL_BASE https://github.com/ocornut/imgui/archive/refs/tags)

FetchContent_Declare(imgui-fetch URL ${IMGUI_URL_BASE}/${IMGUI_VERSION}/v1.91.9b.zip)
FetchContent_MakeAvailable(imgui-fetch)

set(IMPLOT_URL https://github.com/epezent/implot/archive/refs/heads/master.zip)

FetchContent_Declare(implot-fetch URL ${IMPLOT_URL})
FetchContent_MakeAvailable(implot-fetch)

set(RLGUI_URL https://github.com/raylib-extras/rlImGui/archive/refs/heads/main.zip)

FetchContent_Declare(rlgui-fetch URL ${RLGUI_URL})
FetchContent_MakeAvailable(rlgui-fetch)

add_library(imgui INTERFACE)

add_dependencies(imgui imgui-fetch implot-fetch rlgui-fetch)

target_include_directories(imgui INTERFACE ${imgui-fetch_SOURCE_DIR})
target_include_directories(imgui INTERFACE ${implot-fetch_SOURCE_DIR})
target_include_directories(imgui INTERFACE ${rlgui-fetch_SOURCE_DIR})

target_sources(
	imgui INTERFACE
	${imgui-fetch_SOURCE_DIR}/imgui.cpp
	${imgui-fetch_SOURCE_DIR}/imgui_tables.cpp
	${imgui-fetch_SOURCE_DIR}/imgui_widgets.cpp
	${imgui-fetch_SOURCE_DIR}/imgui_draw.cpp)

target_sources(
	imgui INTERFACE
	${implot-fetch_SOURCE_DIR}/implot.cpp
	${implot-fetch_SOURCE_DIR}/implot_items.cpp)

target_sources(imgui INTERFACE ${rlgui-fetch_SOURCE_DIR}/rlImGui.cpp)

target_link_libraries(imgui INTERFACE raylib)
