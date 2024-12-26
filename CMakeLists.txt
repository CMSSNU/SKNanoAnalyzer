cmake_minimum_required(VERSION 3.20)
project(SKNanoAnalyzer VERSION 0.1.0)

# Find ROOT packages
find_package(ROOT REQUIRED COMPONENTS)
find_package(Torch REQUIRED)

include(${ROOT_USE_FILE})

# correctionlibs
find_package(correctionlib REQUIRED)

# Add subdirectories
add_subdirectory(DataFormats)
add_subdirectory(AnalyzerTools)
add_subdirectory(Analyzers)

#External
add_subdirectory(external/KinematicFitter)

# Fetch external dependencies
include(FetchContent)
FetchContent_Declare(
    yaml-cpp
    GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
    GIT_PROGRESS TRUE
    GIT_SHALLOW TRUE
    GIT_TAG 0.8.0
)
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_PROGRESS TRUE
    GIT_SHALLOW TRUE
    GIT_TAG v3.11.3)

FetchContent_MakeAvailable(yaml-cpp json)

#set cxx standard
set(CMAKE_CXX_STANDARD ROOT_CXX_STANDARD)


