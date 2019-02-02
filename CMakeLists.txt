cmake_minimum_required(VERSION 3.4.1)

project(flv-stream-builder)

set(CMAKE_CXX_STANDARD 11)
set(CXX_STANDARD_REQUIRED)

file(GLOB_RECURSE SRC_FILES
    "src/flv_stream_builder.hpp"
    "test/test.cpp"
)

add_executable(test
    ${SRC_FILES}
)
