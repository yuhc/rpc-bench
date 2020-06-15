project(flatbuffers-bench C CXX)

set(CMAKE_CXX_STANDARD 11)

include(ExternalProject)

# Builds absl project from the git submodule.
ExternalProject_Add(absl
  PREFIX absl
  SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../third_party/abseil-cpp"
  CMAKE_CACHE_ARGS
        -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=TRUE
        -DCMAKE_INSTALL_PREFIX:PATH=${EXTERNAL_BINARY_DIR}/absl
)

# Builds c-ares project from the git submodule.
ExternalProject_Add(c-ares
  PREFIX c-ares
  SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../third_party/cares/cares"
  CMAKE_CACHE_ARGS
        -DCARES_SHARED:BOOL=OFF
        -DCARES_STATIC:BOOL=ON
        -DCARES_STATIC_PIC:BOOL=ON
        -DCMAKE_INSTALL_PREFIX:PATH=${EXTERNAL_BINARY_DIR}/c-ares
)

# Builds protobuf project from the git submodule.
ExternalProject_Add(protobuf_3_6
  PREFIX protobuf_3_6
  GIT_REPOSITORY https://github.com/google/protobuf.git
  GIT_TAG v3.6.1
  SOURCE_DIR "${EXTERNAL_BINARY_DIR}/src/protobuf_3_6"
  SOURCE_SUBDIR "cmake"
  CMAKE_CACHE_ARGS
        -Dprotobuf_BUILD_TESTS:BOOL=OFF
        -Dprotobuf_WITH_ZLIB:BOOL=OFF
        -Dprotobuf_MSVC_STATIC_RUNTIME:BOOL=OFF
        -DCMAKE_INSTALL_PREFIX:PATH=${EXTERNAL_BINARY_DIR}/protobuf_3_6
)

# Builds zlib project from the git submodule.
ExternalProject_Add(zlib
  PREFIX zlib
  SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../third_party/zlib"
  CMAKE_CACHE_ARGS
        -DCMAKE_INSTALL_PREFIX:PATH=${EXTERNAL_BINARY_DIR}/zlib
)

set(_FINDPACKAGE_PROTOBUF_CONFIG_DIR "${EXTERNAL_BINARY_DIR}/protobuf_3_6/lib/cmake/protobuf")

# if OPENSSL_ROOT_DIR is set, propagate that hint path to the external projects with OpenSSL dependency.
set(_CMAKE_ARGS_OPENSSL_ROOT_DIR "")
if (OPENSSL_ROOT_DIR)
  set(_CMAKE_ARGS_OPENSSL_ROOT_DIR "-DOPENSSL_ROOT_DIR:PATH=${OPENSSL_ROOT_DIR}")
endif()

# Builds gRPC based on locally checked-out sources and set arguments so that all the dependencies
# are correctly located.
ExternalProject_Add(grpc_1_15
  PREFIX grpc_1_15
  GIT_REPOSITORY https://github.com/grpc/grpc.git
  GIT_TAG v1.15.1
  GIT_SUBMODULES_RECURSE ON
  SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../third_party/grpc_1_15"
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
        -DCMAKE_INSTALL_PREFIX:PATH=${EXTERNAL_BINARY_DIR}/grpc_1_15
  DEPENDS c-ares protobuf_3_6 zlib absl
)

# Builds flatbuffers project from the git submodule.
ExternalProject_Add(flatbuffers
  PREFIX flatbuffers
  SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../third_party/flatbuffers"
  CMAKE_CACHE_ARGS
        -DGRPC_INSTALL_PATH=${EXTERNAL_BINARY_DIR}/grpc
        -DPROTOBUF_DOWNLOAD_PATH=${EXTERNAL_BINARY_DIR}/src/protobuf
        -DCMAKE_INSTALL_PREFIX:PATH=${EXTERNAL_BINARY_DIR}/flatbuffers
  DEPENDS protobuf_3_6 grpc_1_15
)

# Build the protobuf-bench projects itself using a CMakeLists.txt that assumes all the dependencies
# have already been installed.
# Even though it is not really an "external project" from perspective of this build,
# we are still importing it using ExternalProject_Add because that allows us to use find_package()
# to locate all the dependencies (if we were building it directly in this build,
# we would have needed to manually import the libraries as opposed to reusing targets exported by
# gRPC and protobuf).
ExternalProject_Add(flatbuffers-bench
  PREFIX flatbuffers-bench
  SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/.."
  BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/.."
  INSTALL_COMMAND ""
  CMAKE_CACHE_ARGS
        -DProtobuf_DIR:PATH=${_FINDPACKAGE_PROTOBUF_CONFIG_DIR}
        -Dc-ares_DIR:PATH=${EXTERNAL_BINARY_DIR}/c-ares/lib/cmake/c-ares
        -DZLIB_ROOT:STRING=${EXTERNAL_BINARY_DIR}/zlib
        -Dabsl_DIR:STRING=${EXTERNAL_BINARY_DIR}/absl/lib/cmake/absl
        ${_CMAKE_ARGS_OPENSSL_ROOT_DIR}
        -DgRPC_DIR:PATH=${EXTERNAL_BINARY_DIR}/grpc_1_15/lib/cmake/grpc
        -DFlatbuffers_DIR:PATH=${EXTERNAL_BINARY_DIR}/flatbuffers/lib/cmake/flatbuffers
  DEPENDS protobuf_3_6 grpc_1_15
)
