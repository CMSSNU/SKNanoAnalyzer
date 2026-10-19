find_package(GTest CONFIG REQUIRED)

# Hand-built SoA fixtures, shared with the AnalyzerFramework suite.  Exported
# as a target so no other module has to name this directory by path.
add_library(SKNanoTestSupport INTERFACE)
target_include_directories(SKNanoTestSupport INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(SKNanoTestSupport INTERFACE DataFormats)

add_executable(sknano_dataformats_tests TauViewTest.cc LeptonViewTest.cc)

target_link_libraries(sknano_dataformats_tests PRIVATE
    SKNanoTestSupport
    GTest::gtest
    GTest::gtest_main)

add_test(NAME dataformats_unit COMMAND sknano_dataformats_tests)
