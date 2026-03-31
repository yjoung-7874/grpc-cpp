#include "grpc_wrapper/server.hpp"
#include "example.grpc.pb.h"
#include <iostream>

using grpc::CallbackServerContext;
using grpc::ServerUnaryReactor;
using example::EchoRequest;
using example::EchoResponse;

class AsyncCallbackEchoServiceImpl final : public example::EchoService::CallbackService {
 public:
  ServerUnaryReactor* Echo(CallbackServerContext* context, 
                           const EchoRequest* request, 
                           EchoResponse* reply) override {
    std::string prefix("Async Callback Echo received: ");
    reply->set_reply(prefix + request->message());

    ServerUnaryReactor* reactor = context->DefaultReactor();
    reactor->Finish(grpc::Status::OK);
    return reactor;
  }
};

int main(int argc, char** argv) {
  grpc_wrapper::GrpcServer server;
  
  AsyncCallbackEchoServiceImpl service;
  server.AddListeningPort("0.0.0.0:50052"); // using different port for this example
  server.RegisterService(&service);
  
  if (server.Start()) {
    std::cout << "Async Callback server is running..." << std::endl;
    server.Wait();
  }
  
  return 0;
}
