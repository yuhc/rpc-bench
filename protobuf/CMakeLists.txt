project(protobuf-bench C CXX)

set(CMAKE_CXX_STANDARD 11)
find_package(Threads REQUIRED)

set(protobuf_MODULE_COMPATIBLE TRUE)
find_package(Protobuf CONFIG REQUIRED)
message(STATUS "Using protobuf ${Protobuf_VERSION}")

set(_PROTOBUF_LIBPROTOBUF protobuf::libprotobuf)
set(_REFLECTION gRPC::grpc++_reflection)
if(CMAKE_CROSSCOMPILING)
  find_program(_PROTOBUF_PROTOC protoc)
else()
  set(_PROTOBUF_PROTOC $<TARGET_FILE:protobuf::protoc>)
endif()

# Find gRPC installation
# Looks for gRPCConfig.cmake file installed by gRPC's cmake installation.
find_package(gRPC CONFIG REQUIRED)
message(STATUS "Using gRPC ${gRPC_VERSION}")

set(_GRPC_GRPCPP gRPC::grpc++)
if(CMAKE_CROSSCOMPILING)
  find_program(_GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin)
else()
  set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:gRPC::grpc_cpp_plugin>)
endif()

# Proto file
get_filename_component(bench_proto "bench.proto" ABSOLUTE)
get_filename_component(bench_proto_path "${bench_proto}" PATH)

# Generated sources
set(bench_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/bench.pb.cc")
set(bench_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/bench.pb.h")
set(bench_grpc_srcs  "${CMAKE_CURRENT_BINARY_DIR}/bench.grpc.pb.cc")
set(bench_grpc_hdrs  "${CMAKE_CURRENT_BINARY_DIR}/bench.grpc.pb.h")
add_custom_command(
  OUTPUT "${bench_proto_srcs}" "${bench_proto_hdrs}" "${bench_grpc_srcs}" "${bench_grpc_hdrs}"
  COMMAND ${_PROTOBUF_PROTOC}
  ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
       --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
       -I "${bench_proto_path}"
       --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
       "${bench_proto}"
  DEPENDS "${bench_proto}"
)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

# Executable setup
set(executable_name ${PROJECT_NAME}_cpp)
add_executable(${executable_name}
  server.cc
  ${bench_proto_srcs}
  ${bench_grpc_srcs}
)
target_link_libraries(${executable_name}
  ${_REFLECTION}
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF}
)
