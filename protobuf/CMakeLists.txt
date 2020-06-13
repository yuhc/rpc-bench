project(protobuf-bench C CXX)

set(CMAKE_CXX_STANDARD 11)
find_package(Threads REQUIRED)

# Build protobuf
set(EXTERNAL_BINARY_DIR ${CMAKE_BINARY_DIR}/third_party)
set(PROTOBUF_BINARY_DIR ${EXTERNAL_DIR}/protobuf)
set(GRPC_BINARY_DIR ${EXTERNAL_DIR}/grpc)

include(ExternalProject)

# Builds absl project from the git submodule.
ExternalProject_Add(absl
  PREFIX absl
  SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/abseil-cpp"
  CMAKE_CACHE_ARGS
        -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=TRUE
        -DCMAKE_INSTALL_PREFIX:PATH=${EXTERNAL_BINARY_DIR}/absl
)

# Builds c-ares project from the git submodule.
ExternalProject_Add(c-ares
  PREFIX c-ares
  SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/cares/cares"
  CMAKE_CACHE_ARGS
        -DCARES_SHARED:BOOL=OFF
        -DCARES_STATIC:BOOL=ON
        -DCARES_STATIC_PIC:BOOL=ON
        -DCMAKE_INSTALL_PREFIX:PATH=${EXTERNAL_BINARY_DIR}/c-ares
)

# Builds protobuf project from the git submodule.
ExternalProject_Add(protobuf
  PREFIX protobuf
  SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/protobuf/cmake"
  CMAKE_CACHE_ARGS
        -Dprotobuf_BUILD_TESTS:BOOL=OFF
        -Dprotobuf_WITH_ZLIB:BOOL=OFF
        -Dprotobuf_MSVC_STATIC_RUNTIME:BOOL=OFF
        -DCMAKE_INSTALL_PREFIX:PATH=${EXTERNAL_BINARY_DIR}/protobuf
)

# Builds zlib project from the git submodule.
ExternalProject_Add(zlib
  PREFIX zlib
  SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/zlib"
  CMAKE_CACHE_ARGS
        -DCMAKE_INSTALL_PREFIX:PATH=${EXTERNAL_BINARY_DIR}/zlib
)

set(_FINDPACKAGE_PROTOBUF_CONFIG_DIR "${EXTERNAL_BINARY_DIR}/protobuf/lib/cmake/protobuf")

# if OPENSSL_ROOT_DIR is set, propagate that hint path to the external projects with OpenSSL dependency.
set(_CMAKE_ARGS_OPENSSL_ROOT_DIR "")
if (OPENSSL_ROOT_DIR)
  set(_CMAKE_ARGS_OPENSSL_ROOT_DIR "-DOPENSSL_ROOT_DIR:PATH=${OPENSSL_ROOT_DIR}")
endif()

# Builds gRPC based on locally checked-out sources and set arguments so that all the dependencies
# are correctly located.
ExternalProject_Add(grpc
  PREFIX grpc
  SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/grpc"
  CMAKE_CACHE_ARGS
        -DgRPC_INSTALL:BOOL=ON
        -DgRPC_BUILD_TESTS:BOOL=OFF
        -DgRPC_PROTOBUF_PROVIDER:STRING=package
        -DgRPC_PROTOBUF_PACKAGE_TYPE:STRING=CONFIG
        -DProtobuf_DIR:PATH=${_FINDPACKAGE_PROTOBUF_CONFIG_DIR}
        -DgRPC_ZLIB_PROVIDER:STRING=package
        -DZLIB_ROOT:STRING=${EXTERNAL_BINARY_DIR}/zlib
        -DgRPC_ABSL_PROVIDER:STRING=package
        -Dabsl_DIR:STRING=${EXTERNAL_BINARY_DIR}/absl/lib/cmake/absl
        -DgRPC_CARES_PROVIDER:STRING=package
        -Dc-ares_DIR:PATH=${EXTERNAL_BINARY_DIR}/c-ares/lib/cmake/c-ares
        -DgRPC_SSL_PROVIDER:STRING=package
        ${_CMAKE_ARGS_OPENSSL_ROOT_DIR}
        -DCMAKE_INSTALL_PREFIX:PATH=${EXTERNAL_BINARY_DIR}/grpc
  DEPENDS c-ares protobuf zlib absl
)

set(_PROTOBUF_LIBPROTOBUF libprotobuf)
set(_REFLECTION grpc++_reflection)
if(CMAKE_CROSSCOMPILING)
  find_program(_PROTOBUF_PROTOC protoc)
else()
  set(_PROTOBUF_PROTOC $<TARGET_FILE:protobuf::protoc>)
endif()
set(_GRPC_GRPCPP grpc++)
if(CMAKE_CROSSCOMPILING)
  find_program(_GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin)
else()
  set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:grpc_cpp_plugin>)
endif()

get_filename_component(bench_proto "bench.proto" ABSOLUTE)
get_filename_component(bench_proto_path "${bench_proto}" PATH)

set(bench_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/bench.pb.cc")
set(bench_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/bench.pb.h")
set(bench_grpc_srcs  "${CMAKE_CURRENT_BINARY_DIR}/bench.grpc.pb.cc")
set(bench_grpc_hdrs  "${CMAKE_CURRENT_BINARY_DIR}/bench.grpc.pb.h")

##Code Generation
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


#EXternalProject_Get_Property(protobuf source_dir)
#include_directories(${source_dir}/src)
#link_directories(${PROTOBUF_BINARY_DIR})
#EXternalProject_Get_Property(grpc source_dir)
#include_directories(${source_dir}/include)
#link_directories(${GRPC_BINARY_DIR})
#
#set(${PROJECT_NAME}_SRCS server.cc)
#
#list(APPEND ${PROJECT_NAME}_SRCS ${${PROJECT_NAME}_PROTO_SRCS} ${${PROJECT_NAME}_PROTO_HDRS})
#
###Executable setup
#set(executable_name ${PROJECT_NAME}_cpp)
#include_directories(${CMAKE_SOURCE_DIR})
#include_directories(${CMAKE_CURRENT_BINARY_DIR})
#find_package(OpenSSL)
#if (NOT OPENSSL_FOUND)
#  message(FATAL_ERROR "Run cmake with -DOPENSSL_ROOT_DIR=/usr/local/opt/openssl -DOPENSSL_LIBRARIES=/usr/local/opt/openssl/lib")
#endif()
#message(STATUS OpenSSL ${OPENSSL_VERSION} ${OPENSSL_SSL_LIBRARY})
#add_executable(${executable_name}
#  ${${PROJECT_NAME}_SRCS}
#  ${${PROJECT_NAME}_PROTOS}
#)
#target_link_libraries(${executable_name}
#  ${GRPC_BINARY_DIR}/libgrpc++_reflection.a
#  ${GRPC_BINARY_DIR}/libgrpc++.a
#  ${PROTOBUF_BINARY_DIR}/libprotobuf.a
#  ${GRPC_BINARY_DIR}/libgrpc.a
#  ${GRPC_BINARY_DIR}/libgpr.a
#  ${GRPC_BINARY_DIR}/libaddress_sorting.a
#  ${GRPC_BINARY_DIR}/libupb.a
#  cares
#  Threads::Threads
#  ${OPENSSL_SSL_LIBRARIES}
#  ${OPENSSL_CRYPTO_LIBRARIES}
#)
