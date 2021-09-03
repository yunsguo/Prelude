cmake_minimum_required(VERSION 3.21)

# set the project name
project(Tuple DESCRIPTION "A header file for tuple declaration.")

add_library(${PROJECT_NAME} INTERFACE tuple.h)
target_link_libraries(${PROJECT_NAME} INTERFACE TMP)
target_include_directories(${PROJECT_NAME}
                           INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

option(USE_SHAREDTUPLE "Build using shared libraries" OFF)
if(USE_SHAREDTUPLE)
  add_compile_definitions(USE_SHAREDTUPLE)
  add_library(SharedTuple INTERFACE shared_tuple.h)
  target_link_libraries(SharedTuple INTERFACE TMP)
  target_include_directories(SharedTuple INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
  target_link_libraries(${PROJECT_NAME} INTERFACE SharedTuple)
endif()

# add the test stub
add_executable(${PROJECT_NAME}Stub tuple.cpp)
target_link_libraries(${PROJECT_NAME}Stub PUBLIC ${PROJECT_NAME} NonTrivial)

enable_testing()
add_test(NAME ${PROJECT_NAME}Stub COMMAND ${PROJECT_NAME}Stub)
