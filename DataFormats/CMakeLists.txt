# Define the library with source files and the generated dictionary
file(GLOB SOURCES src/*.cc)
add_library(DataFormats SHARED
    ${SOURCES}
    #${CMAKE_CURRENT_BINARY_DIR}/G__DataFormats.cxx
)

# Specify where the library should look for header files
target_include_directories(DataFormats PRIVATE
    $<BUILD_INTERFACE:${ROOT_INCLUDE_DIRS}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Generate the ROOT dictionary before defining the library
file(GLOB HEADERS include/*.h)
ROOT_GENERATE_DICTIONARY(G__DataFormats
    ${HEADERS}
    MODULE DataFormats
    LINKDEF include/DataFormatsLinkDef.hpp
)


# Link ROOT libraries
target_link_libraries(DataFormats ${ROOT_LIBRARIES})

# Specify where to install the library
install(TARGETS DataFormats DESTINATION lib)

# Iinstall .rootmap and .pcm files to the lib
set(ROOTMAP "libDataFormats.rootmap")
set(PCM "libDataFormats_rdict.pcm")
message(STATUS "ROOTMAP: ${ROOTMAP}")
message(STATUS "PCM: ${PCM}")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${ROOTMAP}" "${CMAKE_CURRENT_BINARY_DIR}/${PCM}" DESTINATION lib)
