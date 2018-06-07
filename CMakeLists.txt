cmake_minimum_required(VERSION 3.11.3 FATAL_ERROR)
project(ice VERSION 0.1.0 LANGUAGES CXX)

option(BUILD_APPLICATION "Build application" OFF)
option(BUILD_BENCHMARK "Build benchmark" OFF)
option(BUILD_TESTS "Build tests" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  add_compile_options(/await)
else()
  add_compile_options(-fcoroutines-ts)
endif()

file(GLOB_RECURSE src src/ice/*.hpp src/ice/*.cpp)

add_library(ice STATIC ${src})
target_include_directories(ice PUBLIC src)

if(UNIX)
  find_package(Threads REQUIRED)
  target_link_libraries(ice PUBLIC Threads::Threads)
endif()

find_package(unofficial-date CONFIG REQUIRED)
target_link_libraries(ice PRIVATE unofficial::date::date)

if(BUILD_APPLICATION)
  file(GLOB main_src src/*.hpp src/*.cpp)
  add_executable(main ${main_src})
  target_link_libraries(main PRIVATE ice)
  set_target_properties(main PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
endif()

if(BUILD_BENCHMARK)
  file(GLOB benchmark_src src/benchmark/*.hpp src/benchmark/*.cpp)
  add_executable(benchmark ${benchmark_src})
  find_package(benchmark REQUIRED)
  target_link_libraries(benchmark PRIVATE benchmark::benchmark ice)
endif()

if(BUILD_TESTS)
  enable_testing()
  include(GoogleTest)
  file(GLOB tests_src src/tests/*.hpp src/tests/*.cpp)
  add_executable(tests ${tests_src})
  find_package(GTest REQUIRED)
  target_link_libraries(tests PRIVATE GTest::GTest GTest::Main ice)
  gtest_add_tests(TARGET tests WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()
