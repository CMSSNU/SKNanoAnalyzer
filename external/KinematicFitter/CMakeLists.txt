# Generate the ROOT dictionary before defining the library
file(GLOB HEADERS include/*.h)

# Define the library with source files and the generated dictionary
file(GLOB SOURCES NanoGardenerModules/KinematicFitter/src/*.C)
add_library(KinFitter SHARED
    ${SOURCES}
)

# Specify where the library should look for header files
target_include_directories(KinFitter PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${ROOT_INCLUDE_DIRS}>
    $<INSTALL_INTERFACE:include>
)
# Link ROOT libraries
target_link_libraries(KinFitter ${ROOT_LIBRARIES})

ROOT_GENERATE_DICTIONARY(G__KinFitter
    ${HEADERS}
    MODULE KinFitter
    LINKDEF include/KinFitter_LinkDef.hpp
)

# Specify where to install the library
install(TARGETS KinFitter DESTINATION lib)

# Iinstall .rootmap and .pcm files to the lib
set(ROOTMAP "${CMAKE_CURRENT_BINARY_DIR}/libKinFitter.rootmap")
set(PCM "${CMAKE_CURRENT_BINARY_DIR}/libKinFitter_rdict.pcm")
message(STATUS "ROOTMAP: ${ROOTMAP}")
message(STATUS "PCM: ${PCM}")
install(FILES ${ROOTMAP} ${PCM} DESTINATION lib)
