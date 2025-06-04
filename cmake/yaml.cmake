include(FetchContent)

FetchContent_Declare(
  yaml-cpp
  GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
  GIT_TAG yaml-cpp-0.8.0
)
FetchContent_MakeAvailable(yaml-cpp)

add_library(yaml ALIAS yaml-cpp::yaml-cpp)

target_link_libraries(GalaxyEngine PUBLIC yaml)