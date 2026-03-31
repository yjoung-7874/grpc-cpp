# grpc-cpp-wrapper

A lightweight, composition-based C++ wrapper library for gRPC. It simplifies server initialization, service registration, and client channel management using modern C++ design.

## Features

- **Unified Server Management**: Manage synchronous and asynchronous (Callback API) gRPC services in a single `GrpcServer` class. No more restrictive inheritance hierarchies.
- **Simplified Client Management**: Easily connect and manage channels using `GrpcClient`, including robust `WaitForReady` polling to handle startup order complexities.
- **Modern C++ API**: Intuitive and straightforward classes that wrap `grpc::ServerBuilder` and channel creation.

## Directory Structure

- `include/grpc_wrapper/`: Header files for the wrapper classes (`server.hpp`, `client.hpp`).
- `src/`: Implementation of the wrapper classes.
- `examples/`: Complete working examples for both Synchronous and Asynchronous (Callback) gRPC implementations.
- `cmake/`: CMake helper scripts for dependency locators.

## Getting Started

### 1. Integrate via CMake
Add this library as a subdirectory in your CMake project and link against `grpc_wrapper`.

```cmake
add_subdirectory(grpc-cpp-wrapper)
target_link_libraries(your_target PRIVATE grpc_wrapper)
```

### 2. Example: Server Usage
Register your custom RPC services to the `GrpcServer`.

```cpp
#include "grpc_wrapper/server.hpp"
#include "your_service.grpc.pb.h"

int main() {
    grpc_wrapper::GrpcServer server;
    
    YourServiceImpl sync_service;
    YourAsyncCallbackServiceImpl async_service;

    server.AddListeningPort("0.0.0.0:50051");
    // You can register multiple services of different types easily!
    server.RegisterService(&sync_service);
    server.RegisterService(&async_service);
    
    if (server.Start()) {
        server.Wait(); // Block and wait for shutdown
    }
    return 0;
}
```

### 3. Example: Client Usage
Connect to a gRPC server and establish a channel gracefully.

```cpp
#include "grpc_wrapper/client.hpp"
#include "your_service.grpc.pb.h"

int main() {
    grpc_wrapper::GrpcClient client("localhost:50051");

    // Wait until the channel is ready or until timeout
    if (!client.WaitForReady(5000)) {
        return -1;
    }

    auto stub = YourService::NewStub(client.GetChannel());
    // ... use the stub
    return 0;
}
```

## Examples

To understand how to write synchronous and asynchronous services using this wrapper, please look inside the `examples/` directory.
- `examples/sync/`: Simple synchronous ping-pong style implementation.
- `examples/async_callback/`: Modern asynchronous implementation using gRPC's C++ Callback API.
