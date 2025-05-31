set(GLM_VERSION 1.0.1)
set(GLM_URL_BASE https://github.com/g-truc/glm/releases/download)

FetchContent_Declare(glm-fetch URL ${GLM_URL_BASE}/${GLM_VERSION}/glm-1.0.1-light.zip)
FetchContent_MakeAvailable(glm-fetch)

add_library(glm-lib INTERFACE)

target_link_libraries(glm-lib INTERFACE glm)

target_include_directories(glm-lib INTERFACE ${glm-fetch_SOURCE_DIR})

