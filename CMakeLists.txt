cmake_minimum_required(VERSION 3.15)
project(grpc-cpp-wrapper CXX)

set(CMAKE_CXX_STANDARD 17)

# Include common settings/gRPC finding from the provided scripts
include(cmake/common.cmake)

# Define the unified grpc_wrapper library
add_library(grpc_wrapper STATIC
  src/server.cpp
  src/client.cpp
)

target_link_libraries(grpc_wrapper PUBLIC ${_PROTOBUF} ${_GRPC} ${_ABSL})
target_include_directories(grpc_wrapper PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

install(
  DIRECTORY "include/grpc_wrapper/"
  DESTINATION "include/grpc_wrapper"
)

install(
  TARGETS grpc_wrapper
  ARCHIVE DESTINATION "lib"
  LIBRARY DESTINATION "lib"
  INCLUDES DESTINATION "include"
)

# Enable examples
add_subdirectory(examples)
