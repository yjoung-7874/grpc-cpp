#ifndef GRPC_SYNC_SERVER_BASE_HPP_
#define GRPC_SYNC_SERVER_BASE_HPP_

#include <memory>
#include <string>
#include <vector>
#include <thread>
#include <grpcpp/grpcpp.h>
#include <grpcpp/impl/service_type.h>

class GrpcSyncServerBase {
 public:
  GrpcSyncServerBase(const std::string& address);
  virtual ~GrpcSyncServerBase() { Shutdown(); }

  void Init();
  bool BuildAndStartServer();
  void NonBlockRun();
  void Run();
  void Shutdown();

 protected:
  std::vector<std::unique_ptr<grpc::Service>> service_list_;
  virtual void InitServices() = 0;

 private:
  std::string server_address_;
  std::unique_ptr<grpc::Server> server_;
  std::thread wait_thread_;
};

#endif  // GRPC_SYNC_SERVER_BASE_HPP_