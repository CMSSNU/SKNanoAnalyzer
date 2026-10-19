find_package(GTest CONFIG REQUIRED)

add_executable(sknano_tools_tests TauCorrectionTest.cc JetCorrectionTest.cc)

target_link_libraries(sknano_tools_tests PRIVATE
    AnalyzerTools
    SKNanoTestSupport
    GTest::gtest
    GTest::gtest_main)

# No ENVIRONMENT property: ctest inherits the shell it runs in, so the test
# sees whatever setup.sh exported.  Baking the configure-time environment in
# here would silently pin stale paths.  The suite skips itself when the
# corrections it needs are not reachable.
add_test(NAME tools_unit COMMAND sknano_tools_tests)
