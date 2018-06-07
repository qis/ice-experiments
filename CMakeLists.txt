cmake_minimum_required(VERSION 3.11.3 FATAL_ERROR)
project(ice VERSION 0.1.0 LANGUAGES CXX)

option(BUILD_APPLICATION "Build application" OFF)
option(BUILD_BENCHMARK "Build benchmark" OFF)
option(BUILD_TESTS "Build tests" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  add_compile_options(/arch:AVX2 /favor:INTEL64 /await /await:heapelide)
  foreach(name RELEASE RELWITHDEBINFO MINSIZEREL)
    string(APPEND CMAKE_CXX_FLAGS_${name} " /O2 /GS- /GL")
    string(APPEND CMAKE_EXE_LINKER_FLAGS_${name} " /LTCG")
    string(APPEND CMAKE_STATIC_LINKER_FLAGS_${name} " /LTCG")
  endforeach()
else()
  add_compile_options(-march=native -mavx2 -fcoroutines-ts)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  string(REPLACE "/GR" "/GR-" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  string(REPLACE "/EHsc" "/EHs-c-" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
else()
  add_compile_options(-fno-rtti -fno-exceptions)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  string(REGEX REPLACE "[/-]W[0-9]" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  add_compile_options(/W4 /permissive- /wd4100)
else()
  add_compile_options(-Wall -Wextra -Wpedantic -Wconversion -Wno-unused-parameter)
  add_compile_options(-fdiagnostics-absolute-paths -fcolor-diagnostics)
endif()

if(WIN32)
  add_definitions(-DWIN32_LEAN_AND_MEAN -DNOMINMAX -DWINVER=0x0A00 -D_WIN32_WINNT=0x0A00)
  add_definitions(-D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE -D_ATL_SECURE_NO_DEPRECATE)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS -D_HAS_EXCEPTIONS=_CPPUNWIND)
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER build)

configure_file(res/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/src/config.h)

file(GLOB_RECURSE src src/ice/*.hpp src/ice/*.cpp)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src/ice PREFIX "ice" FILES ${src})

add_library(ice STATIC ${src})
target_include_directories(ice PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/src>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
  $<INSTALL_INTERFACE:include>)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  target_link_libraries(ice PUBLIC c++experimental)
endif()

if(WIN32)
  target_link_libraries(ice PUBLIC ws2_32 mswsock)
endif()

if(UNIX)
  find_package(Threads REQUIRED)
  target_link_libraries(ice PUBLIC Threads::Threads)
endif()

find_package(OpenSSL REQUIRED)
target_link_libraries(ice PUBLIC OpenSSL::SSL OpenSSL::Crypto)

include(CMakePackageConfigHelpers)

configure_package_config_file(res/config.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/ice-config.cmake
  INSTALL_DESTINATION lib/cmake/ice)

write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/ice-config-version.cmake
  VERSION ${PROJECT_VERSION} COMPATIBILITY SameMajorVersion)

install(TARGETS ice EXPORT ice LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
install(DIRECTORY src/ice DESTINATION include FILES_MATCHING PATTERN "*.hpp")
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/ice-config.cmake DESTINATION lib/cmake/ice)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/ice-config-version.cmake DESTINATION lib/cmake/ice)
install(EXPORT ice FILE ice-targets.cmake NAMESPACE ice:: DESTINATION lib/cmake/ice)

add_library(ice::ice ALIAS ice)

if(BUILD_APPLICATION)
  file(GLOB src src/*.hpp src/*.cpp)
  source_group("" FILES ${src})
  if(WIN32)
    file(GLOB res res/resource.rc res/resource.manifest)
    source_group("res" FILES ${res})
    list(APPEND src ${res})
  endif()
  add_executable(main ${src})
  target_link_libraries(main PRIVATE ice::ice)
  set_target_properties(main PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
  set_target_properties(main PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT main)
  install(TARGETS main RUNTIME DESTINATION bin CONFIGURATIONS Release RelWithDebInfo MinSizeRel)
  if(WIN32)
    install(DIRECTORY $<TARGET_FILE_DIR:${PROJECT_NAME}>/ DESTINATION bin
      CONFIGURATIONS Release RelWithDebInfo MinSizeRel
      FILES_MATCHING PATTERN "*.dll")
  endif()
endif()

if(BUILD_BENCHMARK)
  file(GLOB src src/benchmark/*.hpp src/benchmark/*.cpp)
  source_group("" FILES ${src})
  add_executable(benchmark ${src})
  find_package(benchmark REQUIRED)
  target_link_libraries(benchmark PRIVATE benchmark::benchmark ice::ice)
  set_target_properties(benchmark PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()

if(BUILD_TESTS)
  enable_testing()
  include(GoogleTest)
  file(GLOB src src/tests/*.hpp src/tests/*.cpp)
  source_group("" FILES ${src})
  add_executable(tests ${src})
  find_package(GTest REQUIRED)
  target_link_libraries(tests PRIVATE GTest::GTest GTest::Main ice::ice)
  gtest_add_tests(TARGET tests WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()

#if(WIN32 AND NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tzdata/windowsZones.xml)
#  file(DOWNLOAD "https://unicode.org/repos/cldr/trunk/common/supplemental/windowsZones.xml"
#    ${CMAKE_CURRENT_SOURCE_DIR}/tzdata/windowsZones.xml)
#endif()
#if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tzdata/version)
#  if(NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/tzdata/tzdata.tar.gz)
#    file(DOWNLOAD "https://data.iana.org/time-zones/releases/tzdata2018e.tar.gz"
#      ${CMAKE_CURRENT_BINARY_DIR}/tzdata/tzdata.tar.gz)
#  endif()
#  execute_process(COMMAND ${CMAKE_COMMAND} -E tar xf tzdata.tar.gz
#    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/tzdata)
#  foreach(name africa antarctica asia australasia backward etcetera
#      europe leapseconds northamerica pacificnew southamerica systemv version)
#    file(COPY ${CMAKE_CURRENT_BINARY_DIR}/tzdata/${name} DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/tzdata)
#  endforeach()
#endif()
#install(DIRECTORY tzdata DESTINATION bin)
