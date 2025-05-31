set(GLM_VERSION 1.0.1)
set(GLM_URL_BASE https://github.com/g-truc/glm/releases/download)

FetchContent_Declare(glm URL ${GLM_URL_BASE}/${GLM_VERSION}/glm-1.0.1-light.zip)
FetchContent_MakeAvailable(glm)
