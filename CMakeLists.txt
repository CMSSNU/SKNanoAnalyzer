cmake_minimum_required(VERSION 3.20)
project(SKNanoAnalyzer VERSION 0.1.0)

# Find ROOT packages
find_package(ROOT REQUIRED COMPONENTS)
include(${ROOT_USE_FILE})

# correctionlibs
find_package(correctionlib REQUIRED)

# Add subdirectories
add_subdirectory(DataFormats)
add_subdirectory(AnalyzerTools)
add_subdirectory(Analyzers)
