# Determine library extension based on OS
if (APPLE)
    set(LIB_EXT "dylib")
else()
    set(LIB_EXT "so")
endif()

# find LHAPDF
set(LHAPDF_INCLUDE_DIR $ENV{SKNANO_HOME}/external/lhapdf/include)
set(LHAPDF_LIBRARY $ENV{SKNANO_HOME}/external/lhapdf/lib/libLHAPDF.${LIB_EXT})

# include ROOT and LHAPDF
include_directories(${ROOT_INCLUDE_DIRS})
include_directories(${LHAPDF_INCLUDE_DIR})

# Generate the ROOT dictionary before defining the library
file(GLOB HEADERS include/*.h)
ROOT_GENERATE_DICTIONARY(G__AnalyzerTools
    ${HEADERS}
    LINKDEF ${CMAKE_CURRENT_SOURCE_DIR}/include/AnalyzerToolsLinkDef.hpp
)

# Define the library with source files and the generated dictionary
file(GLOB SOURCES src/*.cc)
add_library(AnalyzerTools SHARED
    ${SOURCES}
    G__AnalyzerTools.cxx
)

# Specify where the library should look for header files
target_include_directories(AnalyzerTools PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link ROOT and lhapdf libraries
target_link_libraries(AnalyzerTools ${ROOT_LIBRARIES})
target_link_libraries(AnalyzerTools ${LHAPDF_LIBRARY})
target_link_libraries(AnalyzerTools DataFormats)

# Specify where to install the library
install(TARGETS AnalyzerTools DESTINATION lib)

# Iinstall .rootmap and .pcm files to the lib
set(ROOTMAP "${CMAKE_CURRENT_BINARY_DIR}/libAnalyzerTools.rootmap")
set(PCM "${CMAKE_CURRENT_BINARY_DIR}/libAnalyzerTools_rdict.pcm")
message(STATUS "ROOTMAP: ${ROOTMAP}")
message(STATUS "PCM: ${PCM}")
install(FILES ${ROOTMAP} ${PCM} DESTINATION lib)
