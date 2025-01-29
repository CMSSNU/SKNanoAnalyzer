# Install script for directory: /Users/choij/workspace/SKNanoAnalyzer/Analyzers

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/Users/choij/workspace/SKNanoAnalyzer/install")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set path to fallback-tool for dependency-resolution.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/Library/Developer/CommandLineTools/usr/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/Users/choij/workspace/SKNanoAnalyzer/Analyzers/libAnalyzers.dylib")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libAnalyzers.dylib" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libAnalyzers.dylib")
    execute_process(COMMAND /usr/bin/install_name_tool
      -delete_rpath "/opt/homebrew/Cellar/root/6.32.08/lib/root"
      -delete_rpath "/Users/choij/workspace/SKNanoAnalyzer/AnalyzerTools"
      -delete_rpath "/Users/choij/workspace/SKNanoAnalyzer/external/KinematicFitter"
      -delete_rpath "/Users/choij/workspace/SKNanoAnalyzer/DataFormats"
      -delete_rpath "/Users/choij/mamba/envs/Nano/lib/python3.12/site-packages/correctionlib/lib"
      -delete_rpath "/Users/choij/mamba/envs/Nano/lib"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libAnalyzers.dylib")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/Library/Developer/CommandLineTools/usr/bin/strip" -x "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libAnalyzers.dylib")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  include("/Users/choij/workspace/SKNanoAnalyzer/Analyzers/CMakeFiles/Analyzers.dir/install-cxx-module-bmi-Release.cmake" OPTIONAL)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE FILE FILES
    "/Users/choij/workspace/SKNanoAnalyzer/Analyzers/libAnalyzers.rootmap"
    "/Users/choij/workspace/SKNanoAnalyzer/Analyzers/libAnalyzers_rdict.pcm"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE FILE FILES
    "/Users/choij/workspace/SKNanoAnalyzer/Analyzers/include/AnalyzerCore.h"
    "/Users/choij/workspace/SKNanoAnalyzer/Analyzers/include/ExampleRun.h"
    "/Users/choij/workspace/SKNanoAnalyzer/Analyzers/include/FullHadronicTriggerTnP.h"
    "/Users/choij/workspace/SKNanoAnalyzer/Analyzers/include/GeneratorStudy.h"
    "/Users/choij/workspace/SKNanoAnalyzer/Analyzers/include/GetEffLumi.h"
    "/Users/choij/workspace/SKNanoAnalyzer/Analyzers/include/MeasureJetTaggingEff.h"
    "/Users/choij/workspace/SKNanoAnalyzer/Analyzers/include/SKNanoLoader.h"
    )
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "/Users/choij/workspace/SKNanoAnalyzer/Analyzers/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
