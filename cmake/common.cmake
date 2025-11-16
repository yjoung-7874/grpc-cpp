cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 17)

option(protobuf_MODULE_COMPATIBLE TRUE)
find_package(protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)
find_package(absl CONFIG REQUIRED)

set(_PROTOBUF protobuf::protobuf)
set(_GRPC grpc::grpc)
set(_ABSL abseil::abseil)

file(GLOB proto_paths "$ENV{HOME}/.conan2/p/b/proto*/p/bin/protoc")
file(GLOB grpc_plugin_paths "$ENV{HOME}/.conan2/p/b/grpc*/p/bin/grpc_cpp_plugin")

list(GET proto_paths 0 _PROTOBUF_PROTOC)
list(GET grpc_plugin_paths 0 _GRPC_CPP_PLUGIN_EXECUTABLE)

message(STATUS "Using protoc: ${_PROTOBUF_PROTOC}")
message(STATUS "Using grpc_cpp_plugin: ${_GRPC_CPP_PLUGIN_EXECUTABLE}")

message(STATUS "Using protobuf           : ${protobuf_VERSION}")
message(STATUS "Using gRPC               : ${gRPC_VERSION}")
message(STATUS "Using absl               : ${absl_VERSION}")

set(_COMMON_LIBS ${_PROTOBUF} ${_GRPC} ${_ABSL})