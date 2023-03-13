project(lab1-shell)
cmake_minimum_required(VERSION 3.10)

message(STATUS "This is cmake current source dir: " ${CMAKE_CURRENT_SOURCE_DIR})
message(STATUS "This is binary dir:" ${PROJECT_BINARY_DIR})

# unit test
message(STATUS "CMake: Unit Test")
# get .c
file(GLOB TEST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/test/*.c")
# for each
foreach(A_TEST_FILE ${TEST_FILES})
    # get name
    get_filename_component(EXECUTABLE_NAME ${A_TEST_FILE} NAME_WE)
    # add executable
    add_executable(${EXECUTABLE_NAME} ${A_TEST_FILE})
endforeach(A_TEST_FILE ${TEST_FILES})

# shell