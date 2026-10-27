add_library(FriendBuilder STATIC FriendBuilder.cc)
target_include_directories(FriendBuilder PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(FriendBuilder PUBLIC
    ROOT::ROOTNTuple ROOT::RIO ROOT::Core nlohmann_json::nlohmann_json)
set_target_properties(FriendBuilder PROPERTIES POSITION_INDEPENDENT_CODE ON)

add_executable(sknano_friend main.cc)
target_link_libraries(sknano_friend PRIVATE FriendBuilder)
install(TARGETS sknano_friend DESTINATION bin)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
