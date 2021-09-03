project(Function DESCRIPTION "A container template for generic functional operation.")

add_library(${PROJECT_NAME} INTERFACE function.h)

target_link_libraries(${PROJECT_NAME} INTERFACE TMP Tuple)

target_include_directories(${PROJECT_NAME}
                           INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

# add the test stub
add_executable(${PROJECT_NAME}Stub function.cpp)
target_link_libraries(${PROJECT_NAME}Stub PUBLIC ${PROJECT_NAME} NonTrivial)

enable_testing()
add_test(NAME ${PROJECT_NAME}Stub COMMAND ${PROJECT_NAME}Stub)
