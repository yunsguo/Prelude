cmake_minimum_required(VERSION 3.21)

# set the project name
project(
  NonTrivial
  DESCRIPTION
    "A non_trivial struct for general testing that puts messages out on every step of its life cycle."
)

add_library(${PROJECT_NAME} non_trivial.h non_trivial.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
