cmake_minimum_required(VERSION 3.16)

include(FetchContent)

FetchContent_Declare(
  at_parser_doctest
  GIT_REPOSITORY https://github.com/doctest/doctest.git
  GIT_TAG        ae7a13539fb71f270b87eb2e874fbac80bc8dda2 # release-1.10.0
)

FetchContent_MakeAvailable(at_parser_doctest)

# Expose required variable (DOCTEST_INCLUDE_DIR) to parent scope
FetchContent_GetProperties(at_parser_doctest SOURCE_DIR source_dir)

set(DOCTEST_INCLUDE_DIR ${source_dir}/doctest CACHE INTERNAL "Path to include folder for doctest")

add_executable(at_parser_test 
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/parser_helpers.h
    ${CMAKE_CURRENT_SOURCE_DIR}/test_argumented_commands.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_query.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_execute.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_multi_buffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_unkown_or_garbage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_removing_handler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_userdata.cpp
)

target_link_libraries(at_parser_test PUBLIC ${PROJECT_NAME})

target_include_directories(at_parser_test PUBLIC ${DOCTEST_INCLUDE_DIR})
target_include_directories(at_parser_test PUBLIC ${CMAKE_SOURCE_DIR}/../inc)