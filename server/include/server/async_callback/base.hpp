#ifndef GRPC_ASYNC_CALLBACK_SERVER_BASE_HPP_
#define GRPC_ASYNC_CALLBACK_SERVER_BASE_HPP_

#include <grpcpp/grpcpp.h>
#include <grpcpp/impl/service_type.h>

#include <atomic>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

using grpc::Server;
using grpc::ServerAsyncReaderWriter;
using grpc::ServerAsyncResponseWriter;
using grpc::ServerAsyncWriter;
using grpc::ServerBuilder;
using grpc::ServerCompletionQueue;
using grpc::ServerContext;
using grpc::Status;

class GrpcAsyncCallbackServerBase {
 public:
  explicit GrpcAsyncCallbackServerBase(const std::string& address);
  virtual ~GrpcAsyncCallbackServerBase() { Shutdown(); };

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

#endif  // GRPC_ASYNC_CALLBACK_SERVER_BASE_HPP_
