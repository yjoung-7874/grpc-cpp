#ifndef GRPC_WRAPPER_SERVER_HPP_
#define GRPC_WRAPPER_SERVER_HPP_

#include <grpcpp/grpcpp.h>
#include <string>
#include <vector>
#include <memory>
#include <thread>
#include <iostream>

namespace grpc_wrapper {

class GrpcServer {
 public:
  GrpcServer();
  ~GrpcServer();

  // Disable copy
  GrpcServer(const GrpcServer&) = delete;
  GrpcServer& operator=(const GrpcServer&) = delete;

  // Add a listening port before starting the server.
  // Defaults to InsecureServerCredentials.
  void AddListeningPort(const std::string& address);

  // Register a service (synchronous, asynchronous, or callback).
  // The service pointer must remain valid for the lifetime of the server.
  void RegisterService(grpc::Service* service);

  // Build and start the gRPC server. Returns true if successful.
  bool Start();

  // Block the current thread waiting for the server to shutdown.
  void Wait();

  // Shutdown the server cleanly.
  void Shutdown();

 private:
  std::vector<std::string> external_addresses_;
  std::vector<grpc::Service*> services_;
  
  std::unique_ptr<grpc::Server> server_;
};

} // namespace grpc_wrapper

#endif // GRPC_WRAPPER_SERVER_HPP_
