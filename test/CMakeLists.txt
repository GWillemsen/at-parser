project(AtParser)
cmake_minimum_required(VERSION 3.16)

include(FetchContent)

FetchContent_Declare(
  doctest
  GIT_REPOSITORY https://github.com/doctest/doctest.git
  GIT_TAG        ae7a13539fb71f270b87eb2e874fbac80bc8dda2 # release-1.10.0
)

FetchContent_MakeAvailable(doctest)

# Expose required variable (DOCTEST_INCLUDE_DIR) to parent scope
FetchContent_GetProperties(doctest SOURCE_DIR source_dir)

set(DOCTEST_INCLUDE_DIR ${source_dir}/doctest CACHE INTERNAL "Path to include folder for doctest")

add_executable(test 
    ${CMAKE_SOURCE_DIR}/../src/at_parser.c
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/parser_helpers.h
    ${CMAKE_CURRENT_SOURCE_DIR}/test_argumented_commands.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_query.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_execute.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_multi_buffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_unkown_or_garbage.cpp
)

# or per target
target_include_directories(test PUBLIC ${DOCTEST_INCLUDE_DIR})
target_include_directories(test PUBLIC ${CMAKE_SOURCE_DIR}/../inc)