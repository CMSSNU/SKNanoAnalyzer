set(HEADERS RoccoR.h)
set(SOURCES RoccoR.cc)
add_library(RoccoR SHARED
    ${SOURCES}
)

target_include_directories(RoccoR PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${ROOT_INCLUDE_DIRS}>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(RoccoR ${ROOT_LIBRARIES})

ROOT_GENERATE_DICTIONARY(G__RoccoR
    ${HEADERS}
    MODULE RoccoR
    LINKDEF RoccoR_LinkDef.hpp
)

install(TARGETS RoccoR DESTINATION lib)

set(ROOTMAP "libRoccoR.rootmap")
set(PCM "libRoccoR_rdict.pcm")
message(STATUS "ROOTMAP: ${ROOTMAP}")
message(STATUS "PCM: ${PCM}")
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${ROOTMAP} ${CMAKE_CURRENT_BINARY_DIR}/${PCM} DESTINATION lib)