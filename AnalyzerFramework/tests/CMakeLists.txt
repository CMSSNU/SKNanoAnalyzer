find_package(GTest CONFIG REQUIRED)

add_executable(sknano_framework_tests
    SelectTauIndicesTest.cc
    OverlapRemovalTest.cc
    FriendSourceTest.cc)
    WeightIndexTest.cc
    LeptonCorrectionFlowTest.cc
    OutputPathCollisionTest.cc)
    TauEnergyScaleTest.cc)

target_link_libraries(sknano_framework_tests PRIVATE
    AnalyzerFramework
    SKNanoTestSupport
    ROOT::ROOTNTuple
    GTest::gtest
    GTest::gtest_main)

add_test(NAME framework_unit COMMAND sknano_framework_tests)
