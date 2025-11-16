cmake_minimum_required(VERSION 3.15)

project(grpc_cpp_server C CXX)

message(NOTICE "----------------- GRPC_SERVER_SYNC -----------------")
set(grpc_server_sync_srcs
  src/sync/base.cpp
)

set(grpc_server_sync_libs
  ${_PROTOBUF} ${_GRPC} ${_ABSL}
)

add_library(grpc_server_sync STATIC ${grpc_server_sync_srcs})
target_link_libraries(grpc_server_sync PUBLIC ${grpc_server_sync_libs})
target_include_directories(grpc_server_sync PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

message(NOTICE "----------------- GRPC_SERVER_ASYNC -----------------")

set(grpc_server_async_srcs
  src/async/base.cpp
)

set(grpc_server_async_libs
  ${_PROTOBUF} ${_GRPC} ${_ABSL}
)

add_library(grpc_server_async STATIC ${grpc_server_async_srcs})
target_link_libraries(grpc_server_async PUBLIC ${grpc_server_async_libs})
target_include_directories(grpc_server_async PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

install(
  DIRECTORY "include/"
  DESTINATION "include"
)

install(
  TARGETS grpc_server_sync
  ARCHIVE DESTINATION "lib/grpc-cpp/server/sync"
  LIBRARY DESTINATION "lib/grpc-cpp/server/sync"
  INCLUDES DESTINATION "include"
)

install(
  TARGETS grpc_server_async
  ARCHIVE DESTINATION "lib/grpc-cpp/server/async"
  LIBRARY DESTINATION "lib/grpc-cpp/server/async"
  INCLUDES DESTINATION "include"
)

add_subdirectory(test)