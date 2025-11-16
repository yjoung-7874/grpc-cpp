#include <iostream>
#include <thread>
#include <grpcpp/grpcpp.h>
#include "server.hpp"

int main(int argc, char** argv) {
  const std::string addr("127.0.0.1:50001");
  AsyncServer server(addr);
  server.NonBlockRun();
  std::this_thread::sleep_for(std::chrono::milliseconds(150));

  auto channel = grpc::CreateChannel(addr, grpc::InsecureChannelCredentials());
  std::unique_ptr<TestApi::Stub> stub = TestApi::NewStub(channel);

  // --- Server Stream Thread ---
  std::vector<std::thread> server_stream_threads;
  for (int i = 0; i < 5; i++)
    server_stream_threads.emplace_back([&, i]() {
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
  for (auto& t : server_stream_threads) t.join();

  // --- Massive Unary Call Thread ---
  std::vector<std::thread> unary_threads;
  for (int i = 0; i < 100; i++) {
    unary_threads.emplace_back([&, i]() {
      TestRequest req;
      req.set_name("World");
      TestReply rep;
      grpc::ClientContext ctx;
      auto status = stub->Test(&ctx, req, &rep);
      if (status.ok()) {
        std::cout << "[Unary] reply " << i << ": " << rep.message() << std::endl;
      } else {
        std::cout << "[Unary] failed: " << status.error_message() << std::endl;
      }
    });
  }
  for (auto& t : unary_threads) t.join();

  //   // --- Bidi Stream Thread ---
  //   std::thread bidi_thread([&]() {
  //     grpc::ClientContext bctx;
  //     auto bidi = stub->TestBidiStream(&bctx);

  //     // Send a few requests and read replies concurrently
  //     for (int i = 0; i < 5; ++i) {
  //       TestRequest breq;
  //       breq.set_name("BidiUser" + std::to_string(i));
  //       bidi->Write(breq);

  //       TestReply brep;
  //       if (bidi->Read(&brep)) {
  //         std::cout << "[Bidi] reply: " << brep.message() << std::endl;
  //       }

  //       std::this_thread::sleep_for(std::chrono::milliseconds(100));
  //     }

  //     bidi->WritesDone();

  //     TestReply finalRep;
  //     while (bidi->Read(&finalRep)) {
  //       std::cout << "[Bidi] final reply: " << finalRep.message() << std::endl;
  //     }

  //     auto bstatus = bidi->Finish();
  //     if (!bstatus.ok()) {
  //       std::cerr << "[Bidi] failed: " << bstatus.error_message() << std::endl;
  //     }
  //   });

  return 0;
}
