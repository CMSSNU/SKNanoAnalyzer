cmake_minimum_required(VERSION 3.20)
project(SKNanoAnalyzer VERSION 0.1.0)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 20)

# Find ROOT packages
find_package(ROOT REQUIRED COMPONENTS RIO Net)
include(${ROOT_USE_FILE})

# Add subdirectories
add_subdirectory(DataFormats)
add_subdirectory(Analyzers)
