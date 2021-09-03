cmake_minimum_required(VERSION 3.21)

# set the project name
project(
  Prelude
  VERSION 0.1
  DESCRIPTION "A container template for generic functional operation.")

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# control where the static and shared libraries are built so that on windows we
# don't need to tinker with the path to run the executable
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")

add_subdirectory(Function)
add_subdirectory(NonTrivial)
add_subdirectory(TMP)
add_subdirectory(Tuple)

option(PATTERN_MATCH_BY_DATA_TRAITS "implement pattern matching by trait" OFF)
if(PATTERN_MATCH_BY_DATA_TRAITS)
  add_compile_definitions(PATTERN_MATCH_BY_DATA_TRAITS)
endif()

option(NATIVE_SHOW_BY_TO_STRING "lift native types to Show by using std::to_string" OFF)
if(NATIVE_SHOW_BY_TO_STRING)
  add_compile_definitions(NATIVE_SHOW_BY_TO_STRING)
endif()

add_library(Base INTERFACE base.h)
target_link_libraries(Base INTERFACE TMP Function)
target_include_directories(Base INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_library(${PROJECT_NAME} INTERFACE prelude.h)
target_link_libraries(${PROJECT_NAME} INTERFACE Base)
target_include_directories(${PROJECT_NAME}
                           INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

# add the test stub
add_executable(${PROJECT_NAME}Stub prelude.cpp)
target_link_libraries(${PROJECT_NAME}Stub PUBLIC ${PROJECT_NAME})

enable_testing()
add_test(NAME ${PROJECT_NAME}Stub COMMAND ${PROJECT_NAME}Stub)
