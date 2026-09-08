cmake_minimum_required(VERSION 3.20)
if(POLICY CMP0144)
    cmake_policy(SET CMP0144 NEW)
endif()
if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif()

project(SKNanoAnalyzer VERSION 1.0.0 LANGUAGES CXX)

include(CTest)
include(${CMAKE_SOURCE_DIR}/cmake/SKNanoAnalysisModule.cmake)

# Once a dependency checkout exists, configure from that pinned checkout.
# This avoids an implicit network update on every build and still permits the
# initial population when the source directory is absent.
set(FETCHCONTENT_UPDATES_DISCONNECTED ON CACHE BOOL
    "Do not update already-populated FetchContent dependencies" FORCE)

# Apply one language contract to every subdirectory and generated dictionary.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(BOOST_ROOT $ENV{CONDA_PREFIX})
set(Boost_NO_SYSTEM_PATHS ON)
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

# find packages
find_package(ROOT REQUIRED)
find_package(Python3 REQUIRED COMPONENTS Interpreter)
find_package(Boost REQUIRED)
find_package(hwy CONFIG REQUIRED)
find_package(Torch REQUIRED)
if(NOT correctionlib_DIR)
    execute_process(
        COMMAND "${Python3_EXECUTABLE}" -c
                "import pathlib, correctionlib; print(pathlib.Path(correctionlib.__file__).parent / 'cmake')"
        RESULT_VARIABLE SKNANO_CORRECTIONLIB_DISCOVERY_RESULT
        OUTPUT_VARIABLE SKNANO_CORRECTIONLIB_DISCOVERY_PATH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET)
    if(SKNANO_CORRECTIONLIB_DISCOVERY_RESULT EQUAL 0 AND
       EXISTS "${SKNANO_CORRECTIONLIB_DISCOVERY_PATH}")
        set(correctionlib_DIR "${SKNANO_CORRECTIONLIB_DISCOVERY_PATH}")
    endif()
endif()
find_package(correctionlib REQUIRED)
find_package(absl CONFIG REQUIRED)
find_package(VecCore QUIET)
find_package(Vc QUIET)

include(${ROOT_USE_FILE})
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

option(SKNANO_USE_NATIVE_ARCH "Build with -march=native for same-machine local runs" OFF)
option(SKNANO_ENABLE_LTO "Enable interprocedural optimization in Release builds" OFF)
# Baseline ISA for portable builds, e.g. x86-64-v3 for AVX2/FMA.  Applied after
# the toolchain's own -march so it wins; leave empty to keep the toolchain
# default (conda ships -march=nocona, i.e. SSE3).  Every machine that loads the
# resulting libraries must support the selected level.
set(SKNANO_ARCH "" CACHE STRING
    "Target ISA level passed to -march (empty: toolchain default)")
set(SKNANO_NUMERIC_MODE "StrictReference" CACHE STRING
    "Numeric policy: StrictReference or FastValidated")
set_property(CACHE SKNANO_NUMERIC_MODE PROPERTY STRINGS StrictReference FastValidated)
set(SKNANO_PGO_PHASE "OFF" CACHE STRING "PGO phase: OFF, GENERATE, or USE")
set_property(CACHE SKNANO_PGO_PHASE PROPERTY STRINGS OFF GENERATE USE)
set(SKNANO_PGO_PROFILE "${CMAKE_BINARY_DIR}/pgo-data" CACHE PATH
    "GCC profile directory or Clang merged .profdata path")

if(NOT SKNANO_NUMERIC_MODE STREQUAL "StrictReference" AND
   NOT SKNANO_NUMERIC_MODE STREQUAL "FastValidated")
    message(FATAL_ERROR "SKNANO_NUMERIC_MODE must be StrictReference or FastValidated")
endif()
if(NOT SKNANO_PGO_PHASE MATCHES "^(OFF|GENERATE|USE)$")
    message(FATAL_ERROR "SKNANO_PGO_PHASE must be OFF, GENERATE, or USE")
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    add_compile_options(
        $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CONFIG:Release>>:-O3>
        $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CONFIG:RelWithDebInfo>>:-O3>
        $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CONFIG:MinSizeRel>>:-O3>
    )
    if(SKNANO_NUMERIC_MODE STREQUAL "FastValidated")
        add_compile_options(
            $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CONFIG:Release>>:-ffast-math>
            $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CONFIG:RelWithDebInfo>>:-ffast-math>
            $<$<AND:$<COMPILE_LANGUAGE:CXX>,$<CONFIG:MinSizeRel>>:-ffast-math>
        )
        add_compile_definitions(SKNANO_FAST_VALIDATED=1)
    else()
        add_compile_definitions(SKNANO_STRICT_REFERENCE=1)
    endif()
    if(SKNANO_USE_NATIVE_ARCH)
        add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-march=native>)
    elseif(SKNANO_ARCH)
        add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-march=${SKNANO_ARCH}>)
    endif()
    if((SKNANO_USE_NATIVE_ARCH OR SKNANO_ARCH) AND
       CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        # abseil's raw_hash_set.h includes <bmi2intrin.h> directly whenever
        # __BMI2__ is defined, and GCC refuses that header outside
        # <x86gprintrin.h>.  BMI2 is integer bit-manipulation nothing here
        # uses, so drop that one feature and keep the rest of the ISA level.
        add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-mno-bmi2>)
    endif()

    if(SKNANO_PGO_PHASE STREQUAL "GENERATE")
        if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            add_compile_options(-fprofile-generate=${SKNANO_PGO_PROFILE}
                                -fprofile-prefix-path=${CMAKE_BINARY_DIR})
            add_link_options(-fprofile-generate=${SKNANO_PGO_PROFILE})
        else()
            add_compile_options(-fprofile-instr-generate=${SKNANO_PGO_PROFILE}/default_%m.profraw)
            add_link_options(-fprofile-instr-generate=${SKNANO_PGO_PROFILE}/default_%m.profraw)
        endif()
    elseif(SKNANO_PGO_PHASE STREQUAL "USE")
        if(NOT EXISTS "${SKNANO_PGO_PROFILE}")
            message(FATAL_ERROR "SKNANO_PGO_PROFILE does not exist: ${SKNANO_PGO_PROFILE}")
        endif()
        if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            add_compile_options(-fprofile-use=${SKNANO_PGO_PROFILE}
                                -fprofile-prefix-path=${CMAKE_BINARY_DIR}
                                -fprofile-correction)
            add_link_options(-fprofile-use=${SKNANO_PGO_PROFILE} -fprofile-correction)
        else()
            add_compile_options(-fprofile-instr-use=${SKNANO_PGO_PROFILE})
            add_link_options(-fprofile-instr-use=${SKNANO_PGO_PROFILE})
        endif()
    endif()
endif()

if(SKNANO_ENABLE_LTO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT SKNANO_IPO_SUPPORTED OUTPUT SKNANO_IPO_ERROR)
    if(NOT SKNANO_IPO_SUPPORTED)
        message(FATAL_ERROR "LTO is not supported: ${SKNANO_IPO_ERROR}")
    endif()
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELWITHDEBINFO TRUE)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        # GCC LTO misdiagnoses ROOT dictionary's signed nElements sentinel.
        add_link_options(-Wno-alloc-size-larger-than)
    endif()
endif()

if(VecCore_FOUND)
    message(STATUS "VecCore detected: ${VecCore_VERSION}")
    add_compile_definitions(ROOT_USE_VECCORE)
    link_libraries(VecCore::VecCore)
endif()

if(Vc_FOUND)
    message(STATUS "Vc detected: ${Vc_VERSION}")
    add_compile_definitions(ROOT_USE_VC)
    link_libraries(Vc::Vc)
endif()

# Add subdirectories
add_subdirectory(external/KinematicFitter)
add_subdirectory(external/RoccoR)
add_subdirectory(SKNanoCore)
add_subdirectory(DataFormats)
add_subdirectory(AnalyzerTools)
add_subdirectory(AnalyzerFramework)
add_subdirectory(CommonAnalyzers)

set(_sknano_default_analysis_module_dirs "")
file(GLOB _sknano_candidate_module_dirs
    LIST_DIRECTORIES true
    "${CMAKE_SOURCE_DIR}/*_Analyzers")
foreach(_sknano_candidate_dir IN LISTS _sknano_candidate_module_dirs)
    if(IS_DIRECTORY "${_sknano_candidate_dir}"
       AND EXISTS "${_sknano_candidate_dir}/CMakeLists.txt")
        list(APPEND _sknano_default_analysis_module_dirs "${_sknano_candidate_dir}")
    endif()
endforeach()
set(SKNANO_ANALYSIS_MODULE_DIRS "${_sknano_default_analysis_module_dirs}"
    CACHE STRING "Semicolon-separated external SKNano analysis module directories")
set(_sknano_module_index 0)
foreach(_sknano_module_dir IN LISTS SKNANO_ANALYSIS_MODULE_DIRS)
    if(NOT IS_ABSOLUTE "${_sknano_module_dir}")
        set(_sknano_module_dir "${CMAKE_SOURCE_DIR}/${_sknano_module_dir}")
    endif()
    if(NOT EXISTS "${_sknano_module_dir}/CMakeLists.txt")
        message(FATAL_ERROR
            "SKNano analysis module has no CMakeLists.txt: ${_sknano_module_dir}")
    endif()
    math(EXPR _sknano_module_index "${_sknano_module_index} + 1")
    add_subdirectory("${_sknano_module_dir}"
        "${CMAKE_BINARY_DIR}/analysis_modules/${_sknano_module_index}")
endforeach()

get_property(SKNANO_REGISTERED_ANALYZERS GLOBAL
    PROPERTY SKNANO_REGISTERED_ANALYZERS)
list(REMOVE_DUPLICATES SKNANO_REGISTERED_ANALYZERS)
list(SORT SKNANO_REGISTERED_ANALYZERS)
string(REPLACE ";" "\n" SKNANO_ANALYZER_MANIFEST_CONTENT
    "${SKNANO_REGISTERED_ANALYZERS}")
file(GENERATE OUTPUT ${CMAKE_BINARY_DIR}/analyzers.manifest
    CONTENT "${SKNANO_ANALYZER_MANIFEST_CONTENT}\n")
install(FILES ${CMAKE_BINARY_DIR}/analyzers.manifest DESTINATION share/sknano)

if(BUILD_TESTING)
    add_test(NAME analyzer_layer_boundaries
             COMMAND bash ${CMAKE_SOURCE_DIR}/scripts/check_layers.sh)
    add_test(NAME merge_engine
             COMMAND ${Python3_EXECUTABLE}
                     ${CMAKE_SOURCE_DIR}/scripts/tests/test_sknano_merge.py)
    set_tests_properties(merge_engine PROPERTIES SKIP_RETURN_CODE 77)
endif()

# Fetch external dependencies
include(FetchContent)
FetchContent_Declare(
    yaml-cpp
    GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
    GIT_PROGRESS TRUE
    GIT_SHALLOW TRUE
    GIT_TAG master
)
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_PROGRESS TRUE
    GIT_SHALLOW TRUE
    GIT_TAG v3.11.3)

FetchContent_MakeAvailable(yaml-cpp json)
