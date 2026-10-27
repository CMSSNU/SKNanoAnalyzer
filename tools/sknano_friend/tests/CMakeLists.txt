find_package(GTest CONFIG REQUIRED)

add_executable(sknano_friend_tests FriendBuilderTest.cc)
target_link_libraries(sknano_friend_tests PRIVATE
    FriendBuilder
    AnalyzerFramework
    GTest::gtest
    GTest::gtest_main)

add_test(NAME friend_builder_unit COMMAND sknano_friend_tests)
