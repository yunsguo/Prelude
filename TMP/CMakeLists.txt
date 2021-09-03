cmake_minimum_required(VERSION 3.21)

# set the project name
project(TMP DESCRIPTION
"A template meta traits programming package in Haskell style for general use.")

add_library(${PROJECT_NAME} INTERFACE tmp.h)
target_include_directories(${PROJECT_NAME}
                           INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
