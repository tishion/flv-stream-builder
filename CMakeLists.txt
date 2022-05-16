cmake_minimum_required(VERSION 3.4.1)

project(flv-stream-builder)

set(CMAKE_CXX_STANDARD 11)
set(CXX_STANDARD_REQUIRED)

include_directories(
    "include"
)

file(GLOB_RECURSE SRC_FILES
    "include/flv_stream_builder.hpp"
    "test/test.cpp"
)

add_executable(flv-builder-test
    ${SRC_FILES}
)
