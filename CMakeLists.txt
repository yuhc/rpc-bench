project(rpc-bench)
cmake_minimum_required(VERSION 3.17)

set(EXTERNAL_BINARY_DIR ${CMAKE_BINARY_DIR}/third_party)

add_subdirectory(protobuf/superbuild)
