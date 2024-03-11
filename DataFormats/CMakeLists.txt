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

# Glob the .rootmap and .pcm files and install them
file(GLOB ROOTMAPS ${CMAKE_CURRENT_BINARY_DIR}/*.rootmap)
file(GLOB PCMS ${CMAKE_CURRENT_BINARY_DIR}/*.pcm)
install(FILES ${ROOTMAPS} ${PCMS} DESTINATION lib)
