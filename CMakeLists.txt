project(lab1-shell)
cmake_minimum_required(VERSION 3.10)

message(STATUS "This is source dir:" ${PROJECT_SOURCE_DIR})
message(STATUS "This is binary dir:" ${PROJECT_BINARY_DIR})
message(STATUS "This is cmake current source dir: " ${CMAKE_CURRENT_SOURCE_DIR})

# unit test
message(STATUS "CMake Top Directory : Unit Test")
# get .c
file(GLOB TEST_FILES "${PROJECT_SOURCE_DIR}/test/*.c")
# for each
foreach(A_TEST_FILE ${TEST_FILES})
    # get name
    get_filename_component(EXECUTABLE_NAME ${A_TEST_FILE} NAME_WE)
    message(STATUS "Test file name: " ${EXECUTABLE_NAME})
    # add executable
    add_executable(${EXECUTABLE_NAME} ${A_TEST_FILE})
endforeach(A_TEST_FILE ${TEST_FILES})

# shell
message(STATUS "CMake: Shell")
# add src
set(SHELL_SRC ${PROJECT_SOURCE_DIR}/src/shell.c ${PROJECT_SOURCE_DIR}/src/command.c)
add_executable(shell ${SHELL_SRC})

# other nstructs
message(STATUS "CMake: Instruct")
# get subdirectory and file
file(GLOB SRC_DIR "${PROJECT_SOURCE_DIR}/src/*")
# for each subdirectory
foreach(A_SUB_DIR ${SRC_DIR})
    # get name
    get_filename_component(INST_NAME ${A_SUB_DIR} NAME_WE)
    message(STATUS "Subdirectory name: " ${INST_NAME})
    # is a sub dir or not
    if(IS_DIRECTORY ${A_SUB_DIR})   
        # add executable
        add_executable(${INST_NAME} ${A_SUB_DIR}/main.c)
    endif()
endforeach(A_SUB_DIR ${SRC_DIR})
