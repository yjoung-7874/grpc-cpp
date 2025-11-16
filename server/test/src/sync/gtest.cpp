#include <gtest/gtest.h>
#include <grpcpp/grpcpp.h>
#include <thread>
#include <chrono>
#include "server.hpp"

TEST(SyncServerClient, UnaryStreamBidi) {
  const std::string addr("127.0.0.1:50051");
  SyncServer server(addr);
  server.NonBlockRun();
  std::this_thread::sleep_for(std::chrono::milliseconds(150));

  auto channel = grpc::CreateChannel(addr, grpc::InsecureChannelCredentials());
  std::unique_ptr<TestApi::Stub> stub = TestApi::NewStub(channel);

  // --- Server Stream Thread ---
  std::thread server_stream_thread([&]() {
    TestRequest sreq;
    sreq.set_name("StreamUser");
    grpc::ClientContext sctx;
    std::unique_ptr<grpc::ClientReader<TestReply>> reader(stub->TestStream(&sctx, sreq));
    TestReply srep;
    while (reader->Read(&srep)) {
      std::cout << "[Server Stream] " << srep.message() << std::endl;
    }
    auto sstatus = reader->Finish();
    if (!sstatus.ok()) {
      std::cerr << "[Server Stream] failed: " << sstatus.error_message() << std::endl;
    }
  });

  // --- Unary Call Thread ---
  std::thread unary_thread([&]() {
    TestRequest req;
    req.set_name("World");
    TestReply reply;
    grpc::ClientContext ctx;
    auto status = stub->Test(&ctx, req, &reply);
    if (status.ok()) {
      std::cout << "[Unary] reply: " << reply.message() << std::endl;
    } else {
      std::cerr << "[Unary] call failed: " << status.error_message() << std::endl;
    }
  });

  unary_thread.join();
  server_stream_thread.join();
  server.Shutdown();
}