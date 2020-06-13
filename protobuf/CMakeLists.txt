project(protobuf-bench C CXX)
set(CMAKE_CXX_STANDARD 11)

# Build protobuf
set(PROTOBUF_BINARY_DIR ${EXTERNAL_DIR}/protobuf)
set(GRPC_BINARY_DIR ${EXTERNAL_DIR}/grpc)

include(ExternalProject)

ExternalProject_Add(protobuf
  PREFIX protobuf
  URL https://github.com/google/protobuf/archive/v3.12.0.tar.gz
  SOURCE_DIR ${PROTOBUF_BINARY_DIR}
  BINARY_DIR ${PROTOBUF_BINARY_DIR}
  CMAKE_CACHE_ARGS
    "-DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}"
    "-Dprotobuf_BUILD_TESTS:BOOL=OFF"
    "-Dprotobuf_BUILD_EXAMPLES:BOOL=OFF"
    "-Dprotobuf_WITH_ZLIB:BOOL=OFF"
    "-DCMAKE_CXX_COMPILER:STRING=${CMAKE_CXX_COMPILER}"
    # other project specific parameters
  SOURCE_SUBDIR cmake
  BUILD_ALWAYS 1
  STEP_TARGETS build
  INSTALL_COMMAND ""
)

ExternalProject_Add(grpc
  PREFIX grpc
  GIT_REPOSITORY https://github.com/grpc/grpc
  GIT_TAG v1.29.1
  GIT_SUBMODULES_RECURSE ON
  GIT_PROGRESS ON
  SOURCE_DIR ${GRPC_BINARY_DIR}
  BINARY_DIR ${GRPC_BINARY_DIR}
  INSTALL_DIR ${PROTOBUF_BINARY_DIR}
  CMAKE_CACHE_ARGS
        -DgRPC_INSTALL:BOOL=OFF
        -DgRPC_BUILD_TESTS:BOOL=OFF
        -DgRPC_PROTOBUF_PROVIDER:STRING=package
        -DProtobuf_DIR:PATH=${PROTOBUF_BINARY_DIR}
  INSTALL_COMMAND ""
  DEPENDS protobuf
)

EXternalProject_Get_Property(protobuf source_dir)
include_directories(${source_dir}/src)
link_directories(${PROTOBUF_BINARY_DIR})
EXternalProject_Get_Property(grpc source_dir)
include_directories(${source_dir}/include)
link_directories(${GRPC_BINARY_DIR})

set(${PROJECT_NAME}_SRCS server.cc)
set(${PROJECT_NAME}_PROTOS ${CMAKE_CURRENT_SOURCE_DIR}/bench.proto)

set(${PROJECT_NAME}_PROTO_SRCS ${CMAKE_CURRENT_BINARY_DIR}/bench.pb.cc
                               ${CMAKE_CURRENT_BINARY_DIR}/bench.grpc.pb.cc)
set(${PROJECT_NAME}_PROTO_HDRS ${CMAKE_CURRENT_BINARY_DIR}/bench.pb.h
                               ${CMAKE_CURRENT_BINARY_DIR}/grpc.pb.h)

##Code Generation
add_custom_command(
  OUTPUT ${${PROJECT_NAME}_PROTO_SRCS} ${${PROJECT_NAME}_PROTO_HDRS}
  COMMAND ${PROTOBUF_BINARY_DIR}/protoc
  ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
       --cpp_out ${CMAKE_CURRENT_BINARY_DIR}
       -I ${CMAKE_CURRENT_SOURCE_DIR}
       --plugin=protoc-gen-grpc="${GRPC_BINARY_DIR}/grpc_cpp_plugin"
       ${${PROJECT_NAME}_PROTOS}
  DEPENDS grpc protobuf ${${PROJECT_NAME}_PROTOS}
)

list(APPEND ${PROJECT_NAME}_SRCS ${${PROJECT_NAME}_PROTO_SRCS} ${${PROJECT_NAME}_PROTO_HDRS})

##Executable setup
set(executable_name ${PROJECT_NAME}_cpp)
include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
find_package(OpenSSL CONFIG)
if (NOT OPENSSL_FOUND)
  message(FATAL_ERROR "Run cmake with -DOPENSSL_ROOT_DIR=/usr/local/opt/openssl -DOPENSSL_LIBRARIES=/usr/local/opt/openssl/lib")
endif()
add_executable(${executable_name}
  ${${PROJECT_NAME}_SRCS}
  ${${PROJECT_NAME}_PROTOS}
)
target_link_libraries(${executable_name}
  ${GRPC_BINARY_DIR}/libgrpc++_reflection.a
  ${GRPC_BINARY_DIR}/libgrpc++.a
  ${PROTOBUF_BINARY_DIR}/libprotobuf.a
  ${GRPC_BINARY_DIR}/libgrpc.a
  ${GRPC_BINARY_DIR}/libgpr.a
  OpenSSL::SSL
  OpenSSL:Crypto
)
