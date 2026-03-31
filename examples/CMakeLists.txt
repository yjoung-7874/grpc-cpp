add_custom_command(
    OUTPUT
    "${CMAKE_CURRENT_BINARY_DIR}/example.pb.cc"
    "${CMAKE_CURRENT_BINARY_DIR}/example.pb.h"
    "${CMAKE_CURRENT_BINARY_DIR}/example.grpc.pb.cc"
    "${CMAKE_CURRENT_BINARY_DIR}/example.grpc.pb.h"
    COMMAND ${_PROTOBUF_PROTOC}
    ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
         --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
         -I "${CMAKE_CURRENT_SOURCE_DIR}/proto"
         --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
         "${CMAKE_CURRENT_SOURCE_DIR}/proto/example.proto"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/proto/example.proto"
)

add_library(example_proto STATIC
    "${CMAKE_CURRENT_BINARY_DIR}/example.pb.cc"
    "${CMAKE_CURRENT_BINARY_DIR}/example.grpc.pb.cc"
)
target_link_libraries(example_proto PUBLIC ${_PROTOBUF} ${_GRPC} ${_ABSL})
target_include_directories(example_proto PUBLIC "${CMAKE_CURRENT_BINARY_DIR}")

# Synchronous Examples
add_executable(sync_server sync/server_main.cpp)
target_link_libraries(sync_server PRIVATE grpc_wrapper example_proto)

add_executable(sync_client sync/client_main.cpp)
target_link_libraries(sync_client PRIVATE grpc_wrapper example_proto)

# Async Callback Examples
add_executable(async_cb_server async_callback/server_main.cpp)
target_link_libraries(async_cb_server PRIVATE grpc_wrapper example_proto)

add_executable(async_cb_client async_callback/client_main.cpp)
target_link_libraries(async_cb_client PRIVATE grpc_wrapper example_proto)
