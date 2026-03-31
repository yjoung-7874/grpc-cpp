#include "grpc_wrapper/client.hpp"
#include "example.grpc.pb.h"
#include <iostream>
#include <mutex>
#include <condition_variable>

using grpc::ClientContext;
using grpc::Status;
using example::EchoRequest;
using example::EchoResponse;

int main(int argc, char** argv) {
  grpc_wrapper::GrpcClient client("localhost:50052");

  if (!client.WaitForReady(5000)) {
    std::cerr << "Async Callback Client: Channel not ready." << std::endl;
    return -1;
  }

  auto stub = example::EchoService::NewStub(client.GetChannel());

  EchoRequest request;
  request.set_message("Hello from Async Callback Client!");
  EchoResponse reply;
  ClientContext context;

  std::mutex mu;
  std::condition_variable cv;
  bool done = false;

  std::cout << "Sending: " << request.message() << std::endl;

  // The Callback API for client
  stub->async()->Echo(&context, &request, &reply,
                      [&mu, &cv, &done, &reply](Status status) {
                        if (status.ok()) {
                          std::cout << "Received: " << reply.reply() << std::endl;
                        } else {
                          std::cerr << "RPC failed: " << status.error_code() << ": " << status.error_message() << std::endl;
                        }
                        std::lock_guard<std::mutex> lock(mu);
                        done = true;
                        cv.notify_one();
                      });

  // Wait for the async RPC to complete
  std::unique_lock<std::mutex> lock(mu);
  cv.wait(lock, [&done] { return done; });

  return 0;
}
