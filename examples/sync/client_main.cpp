#include "grpc_wrapper/client.hpp"
#include "example.grpc.pb.h"
#include <iostream>

using grpc::ClientContext;
using grpc::Status;
using example::EchoRequest;
using example::EchoResponse;

int main(int argc, char** argv) {
  grpc_wrapper::GrpcClient client("localhost:50051");

  if (!client.WaitForReady(5000)) {
    std::cerr << "Sync Client: Channel not ready." << std::endl;
    return -1;
  }

  // Create stub
  auto stub = example::EchoService::NewStub(client.GetChannel());

  EchoRequest request;
  request.set_message("Hello from Sync Client!");

  EchoResponse reply;
  ClientContext context;

  std::cout << "Sending: " << request.message() << std::endl;
  Status status = stub->Echo(&context, request, &reply);

  if (status.ok()) {
    std::cout << "Received: " << reply.reply() << std::endl;
  } else {
    std::cerr << "RPC failed: " << status.error_code() << ": " << status.error_message() << std::endl;
  }

  return 0;
}
