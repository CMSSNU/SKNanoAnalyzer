# include ROOT
include_directories(${ROOT_INCLUDE_DIRS})

# Generate the ROOT dictionary before defining the library
ROOT_GENERATE_DICTIONARY(G__Analyzers 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/SKNanoLoader.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/AnalyzerCore.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/TestAnalyzer.h
    LINKDEF ${CMAKE_CURRENT_SOURCE_DIR}/include/AnalyzersLinkDef.h
)

# Define the library with source files and the generated dictionary
add_library(Analyzers SHARED
    src/SKNanoLoader.cc
    src/AnalyzerCore.cc
    src/TestAnalyzer.cc
    G__Analyzers.cxx
)

# Specify where the library should look for header files
target_include_directories(Analyzers PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link ROOT libraries
target_link_libraries(Analyzers ${ROOT_LIBRARIES})
target_link_libraries(Analyzers DataFormats)

# Specify where to install the library
install(TARGETS Analyzers DESTINATION lib)

# Iinstall .rootmap and .pcm files to the lib
set(ROOTMAP "${CMAKE_CURRENT_BINARY_DIR}/libAnalyzers.rootmap")
set(PCM "${CMAKE_CURRENT_BINARY_DIR}/libAnalyzers_rdict.pcm")
message(STATUS "ROOTMAP: ${ROOTMAP}")
message(STATUS "PCM: ${PCM}")
install(FILES ${ROOTMAP} ${PCM} DESTINATION lib)
