#include "grpc_wrapper/server.hpp"
#include "example.grpc.pb.h"
#include <iostream>

using grpc::ServerContext;
using grpc::Status;
using example::EchoRequest;
using example::EchoResponse;

class SyncEchoServiceImpl final : public example::EchoService::Service {
 public:
  Status Echo(ServerContext* context, const EchoRequest* request,
              EchoResponse* reply) override {
    std::string prefix("Sync Echo received: ");
    reply->set_reply(prefix + request->message());
    return Status::OK;
  }
};

int main(int argc, char** argv) {
  grpc_wrapper::GrpcServer server;
  
  SyncEchoServiceImpl service;
  server.AddListeningPort("0.0.0.0:50051");
  server.RegisterService(&service);
  
  if (server.Start()) {
    std::cout << "Sync server is running..." << std::endl;
    server.Wait();
  }
  
  return 0;
}
