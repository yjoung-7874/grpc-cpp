project(async_server_tests)

find_package(GTest REQUIRED)

message(NOTICE "----------------- TEST_GRPC_ASYNC_SERVER_PROTO -----------------")

set(test_proto_loc ${CMAKE_CURRENT_SOURCE_DIR}/proto)
get_filename_component(test_proto "${test_proto_loc}/test_api.proto" ABSOLUTE)
get_filename_component(test_proto_path "${test_proto}" PATH)
message(STATUS "test_proto_path: ${test_proto_path}")

set(test_api_proto_srcs "${test_proto_loc}/test_api.pb.cc")
set(test_api_grpc_srcs "${test_proto_loc}/test_api.grpc.pb.cc")
set(test_api_proto_hdrs "${test_proto_loc}/test_api.pb.h")
set(test_api_grpc_hdrs "${test_proto_loc}/test_api.grpc.pb.h")

add_custom_command(
    OUTPUT
    "${test_api_proto_srcs}"
    "${test_api_proto_hdrs}"
    "${test_api_grpc_srcs}"
    "${test_api_grpc_hdrs}"
    COMMAND
    ${_PROTOBUF_PROTOC}
    ARGS
    --grpc_out "${test_proto_loc}"
    --cpp_out "${test_proto_loc}"
    -I ${test_proto_loc}
    --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
    "${test_proto}"
    DEPENDS
    "${test_proto}"
)

add_library(grpc_test_proto STATIC
    ${test_api_proto_srcs}
    ${test_api_grpc_srcs}
)

target_link_libraries(grpc_test_proto ${_PROTOBUF} ${_GRPC} ${_ABSL})
target_include_directories(grpc_test_proto PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/proto>
    $<INSTALL_INTERFACE:include>
)

install(
    FILES
    ${test_api_proto_hdrs}
    ${test_api_grpc_hdrs}
    DESTINATION
    "include/server/test/proto"
)

install(
    TARGETS grpc_test_proto
    ARCHIVE DESTINATION "lib/server/test/proto"
    LIBRARY DESTINATION "lib/server/test/proto"
    INCLUDES DESTINATION "include"
)

message(NOTICE "----------------- GRPC_ASYNC_CALLBACK_SERVER_TEST -----------------")
add_executable(async_callback_server_test
    src/async_callback/test.cpp
    src/async_callback/server.cpp
    api/test_api_async_callback.cpp
)
target_link_libraries(async_callback_server_test PUBLIC grpc_server_async_callback grpc_test_proto)
target_include_directories(async_callback_server_test PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

message(NOTICE "----------------- GRPC_SYNC_SERVER_TEST -----------------")
add_executable(sync_server_test
    src/sync/test.cpp
    src/sync/server.cpp
    api/test_api_sync.cpp
)
target_link_libraries(sync_server_test PRIVATE grpc_server_sync grpc_test_proto)
target_include_directories(sync_server_test PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

install(
  TARGETS async_callback_server_test sync_server_test
  RUNTIME DESTINATION "bin/server/test"
)
