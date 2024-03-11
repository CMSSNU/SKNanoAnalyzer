# include ROOT
include_directories(${ROOT_INCLUDE_DIRS})

# Generate the ROOT dictionary before defining the library
ROOT_GENERATE_DICTIONARY(G__DataFormats 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Particle.h
    LINKDEF ${CMAKE_CURRENT_SOURCE_DIR}/include/DataFormatsLinkDef.h
)

# Define the library with source files and the generated dictionary
add_library(DataFormats SHARED
    src/Particle.cc
    G__DataFormats.cxx
)

# Specify where the library should look for header files
target_include_directories(DataFormats PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link ROOT libraries
target_link_libraries(DataFormats ${ROOT_LIBRARIES})

# Specify where to install the library
install(TARGETS DataFormats DESTINATION lib)

# Iinstall .rootmap and .pcm files to the lib
set(ROOTMAP "${CMAKE_CURRENT_BINARY_DIR}/libDataFormats.rootmap")
set(PCM "${CMAKE_CURRENT_BINARY_DIR}/libDataFormats_rdict.pcm")
message(STATUS "ROOTMAP: ${ROOTMAP}")
message(STATUS "PCM: ${PCM}")
install(FILES ${ROOTMAP} ${PCM} DESTINATION lib)
