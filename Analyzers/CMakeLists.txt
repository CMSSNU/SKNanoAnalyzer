# Determine library extension based on OS
if (APPLE)
    set(LIB_EXT "dylib")
else()
    set(LIB_EXT "so")
endif()

# find LHAPDF
set(LHAPDF_INCLUDE_DIR $ENV{SKNANO_HOME}/external/lhapdf/include)
set(LHAPDF_LIBRARY $ENV{SKNANO_HOME}/external/lhapdf/lib/libLHAPDF.${LIB_EXT})

# include ROOT
include_directories(${ROOT_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../DataFormats/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../AnalyzerTools/include)
include_directories(${LHAPDF_INCLUDE_DIR})

# Generate the ROOT dictionary before defining the library
file(GLOB HEADERS include/*.h)
ROOT_GENERATE_DICTIONARY(G__Analyzers 
    ${HEADERS}
    LINKDEF ${CMAKE_CURRENT_SOURCE_DIR}/include/AnalyzersLinkDef.hpp
)

# Define the library with source files and the generated dictionary
file(GLOB SOURCES src/*.cc)
add_library(Analyzers SHARED
    ${SOURCES}
    G__Analyzers.cxx
)

# Specify where the library should look for header files
target_include_directories(Analyzers PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link ROOT libraries
target_link_libraries(Analyzers ${ROOT_LIBRARIES})
target_link_libraries(Analyzers ${LHAPDF_LIBRARY})
target_link_libraries(Analyzers DataFormats)
target_link_libraries(Analyzers AnalyzerTools)

# Specify where to install the library
install(TARGETS Analyzers DESTINATION lib)

# Iinstall .rootmap and .pcm files to the lib
set(ROOTMAP "${CMAKE_CURRENT_BINARY_DIR}/libAnalyzers.rootmap")
set(PCM "${CMAKE_CURRENT_BINARY_DIR}/libAnalyzers_rdict.pcm")
message(STATUS "ROOTMAP: ${ROOTMAP}")
message(STATUS "PCM: ${PCM}")
install(FILES ${ROOTMAP} ${PCM} DESTINATION lib)
