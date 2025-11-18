#include <grpcpp/grpcpp.h>

#include <iostream>
#include <thread>

#include "server.hpp"

int main(int argc, char** argv) {
  const std::string addr("127.0.0.1:50002");
  AsyncServer server(addr);
  server.NonBlockRun();
  std::this_thread::sleep_for(std::chrono::milliseconds(150));

  auto channel = grpc::CreateChannel(addr, grpc::InsecureChannelCredentials());
  std::unique_ptr<TestApi::Stub> stub = TestApi::NewStub(channel);

  // --- Server Stream Group ---
  std::thread stream_group([&]() {
    std::vector<std::thread> stream_threads;

    for (int i = 0; i < 5; i++) {
      stream_threads.emplace_back([&, i]() {
        TestRequest sreq;
        sreq.set_name("StreamUser");
        grpc::ClientContext sctx;

        auto start = std::chrono::steady_clock::now();
        auto prev = start;

        std::unique_ptr<grpc::ClientReader<TestReply>> reader(stub->TestStream(&sctx, sreq));

        TestReply srep;
        while (reader->Read(&srep)) {
          auto now = std::chrono::steady_clock::now();

          auto from_start_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
          auto from_prev_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - prev).count();

          std::cout << "[Server Stream][" << i << "] "
                    << "Δprev=" << from_prev_ms << " ms, "
                    << "Δstart=" << from_start_ms << " ms : " << srep.message() << std::endl;

          prev = now;
        }

        auto sstatus = reader->Finish();
        if (!sstatus.ok()) {
          std::cerr << "[Server Stream][" << i << "] failed: " << sstatus.error_message() << std::endl;
        }
      });
    }

    for (auto& t : stream_threads) t.join();
  });
  stream_group.detach();

  // --- Unary Group ---
  std::thread unary_group([&]() {
    std::vector<std::thread> unary_threads;

    for (int i = 0; i < 1000; i++) {
      unary_threads.emplace_back([&, i]() {
        TestRequest req;
        req.set_name("World");
        TestReply rep;
        grpc::ClientContext ctx;

        auto start = std::chrono::steady_clock::now();

        auto status = stub->Test(&ctx, req, &rep);

        auto end = std::chrono::steady_clock::now();
        auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        if (status.ok()) {
          std::cout << "[Unary][" << i << "] +" << elapsed_ms << " ms : " << rep.message() << std::endl;
        } else {
          std::cout << "[Unary][" << i << "] failed: " << status.error_message() << std::endl;
        }
      });
    }

    for (auto& t : unary_threads) t.join();
  });

  unary_group.join();
  std::cout << "All load threads done.\n";

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
  //   bidi_thread.join();

  return 0;
}
