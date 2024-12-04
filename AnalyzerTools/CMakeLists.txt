cmake_minimum_required(VERSION 3.12)

# Determine library extension based on OS
if(APPLE)
    set(LIB_EXT "dylib")
elseif(UNIX)
    set(LIB_EXT "so")
else()
    message(FATAL_ERROR "Unsupported platform")
endif()

# Find packages
find_package(ROOT REQUIRED)
find_package(Torch REQUIRED)

# Find libraries with more robust method
find_library(LHAPDF_LIBRARY NAMES LHAPDF PATHS $ENV{LHAPDF_LIB_DIR})
find_library(CORRECTION_LIBRARY NAMES correctionlib PATHS $ENV{CORRECTION_LIB_DIR})
find_library(ONNXRUNTIME_LIBRARY NAMES onnxruntime PATHS $ENV{ONNXRUNTIME_LIB_DIR})
find_library(LIBTORCH_LIBRARY NAMES torch PATHS $ENV{LIBTORCH_LIB_DIR})


# Prepare headers for ROOT dictionary
file(GLOB HEADERS include/*.h)

# Attempt to resolve header conflicts more elegantly
# This is a placeholder - you may need a more sophisticated approach
set(DICTIONARY_HEADERS ${HEADERS})
list(FILTER DICTIONARY_HEADERS EXCLUDE REGEX "MLHelper.h")


# Prepare source files
file(GLOB SOURCES src/*.cc)
list(FILTER SOURCES EXCLUDE REGEX "MLHelper.cc")

# Main library target
add_library(AnalyzerTools SHARED
    ${SOURCES}
    #${CMAKE_CURRENT_BINARY_DIR}/G__AnalyzerTools.cxx
)

# Include directories
target_include_directories(AnalyzerTools PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../DataFormats/include>
    $<BUILD_INTERFACE:${ROOT_INCLUDE_DIRS}>
    $<BUILD_INTERFACE:$ENV{LHAPDF_INCLUDE_DIR}>
    $<BUILD_INTERFACE:$ENV{CORRECTION_INCLUDE_DIR}>
    $<INSTALL_INTERFACE:include>
)

# Generate ROOT dictionary
ROOT_GENERATE_DICTIONARY(G__AnalyzerTools
    ${DICTIONARY_HEADERS}
    MODULE AnalyzerTools
    LINKDEF include/AnalyzerToolsLinkDef.hpp
)


# Link libraries
target_link_libraries(AnalyzerTools PUBLIC 
    ${ROOT_LIBRARIES}
    ${LHAPDF_LIBRARY}
    ${CORRECTION_LIBRARY}
    DataFormats
    yaml-cpp::yaml-cpp
)

# MLHelper library
add_library(MLHelper SHARED 
    src/MLHelper.cc 
    include/MLHelper.h
)

target_include_directories(MLHelper PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:$ENV{LIBTORCH_INCLUDE_DIR}>
    $<BUILD_INTERFACE:$ENV{ONNXRUNTIME_INCLUDE_DIR}>
    $<INSTALL_INTERFACE:include>
)


target_link_libraries(MLHelper PUBLIC 
    ${LIBTORCH_LIBRARY}
    ${ONNXRUNTIME_LIBRARY}
)

# Installation
install(TARGETS AnalyzerTools MLHelper 
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# Install ROOT dictionary files if they exist
set(ROOTMAP "libAnalyzerTools.rootmap")
set(PCM "libAnalyzerTools_rdict.pcm")

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${ROOTMAP}" "${CMAKE_CURRENT_BINARY_DIR}/${PCM}" DESTINATION lib)
