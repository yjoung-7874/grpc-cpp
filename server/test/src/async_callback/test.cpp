#include <iostream>
#include <thread>
#include <vector>
#include <algorithm>
#include <mutex>
#include <atomic>
#include <grpcpp/grpcpp.h>
#include "server.hpp"

using SERVER_TYPE = AsyncCallbackServer;

long long percentile(std::vector<long long>& data, double p) {
  if (data.empty()) return 0;
  size_t idx = static_cast<size_t>(p * (data.size() - 1));
  return data[idx];
}

void atomic_max(std::atomic<long long>& target, long long value) {
  long long prev = target.load(std::memory_order_relaxed);
  while (prev < value && !target.compare_exchange_weak(prev, value, std::memory_order_release, std::memory_order_relaxed)) {
  }
}

int main(int argc, char** argv) {
  const std::string addr("127.0.0.1:50001");

  const int STREAM_MESSAGE_COUNT = 50;
  const int STREAM_CONCURRENCY = 900;
  const int UNARY_CONCURRENCY = 900;

  SERVER_TYPE server(addr);
  server.NonBlockRun();
  std::this_thread::sleep_for(std::chrono::milliseconds(150));

  auto channel = grpc::CreateChannel(addr, grpc::InsecureChannelCredentials());
  std::unique_ptr<TestApi_A::Stub> stub_a = TestApi_A::NewStub(channel);
  std::unique_ptr<TestApi_B::Stub> stub_b = TestApi_B::NewStub(channel);

  std::atomic<long long> stream_max_prev{0};
  std::atomic<long long> unary_max_latency{0};

  std::vector<long long> stream_prev_list;
  std::vector<long long> unary_latency_list;
  stream_prev_list.reserve(STREAM_CONCURRENCY * STREAM_MESSAGE_COUNT);
  unary_latency_list.reserve(UNARY_CONCURRENCY);

  std::mutex stream_mutex;
  std::mutex unary_mutex;

  auto global_start = std::chrono::steady_clock::now();

  std::vector<std::thread> all_threads;

  for (int i = 0; i < STREAM_CONCURRENCY; i++) {
    all_threads.emplace_back([&, i]() {
      TestRequest sreq;
      sreq.set_name("GROUP A");
      grpc::ClientContext sctx;

      auto prev = std::chrono::steady_clock::now();
      std::unique_ptr<grpc::ClientReader<TestReply>> reader(stub_a->TestStream(&sctx, sreq));
      TestReply srep;
      while (reader->Read(&srep)) {
        auto now = std::chrono::steady_clock::now();
        long long delta_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - prev).count();
        atomic_max(stream_max_prev, delta_ms);

        {
          std::lock_guard<std::mutex> lg(stream_mutex);
          stream_prev_list.push_back(delta_ms);
        }

        prev = now;
      }
    });
  }

  for (int i = 0; i < UNARY_CONCURRENCY; i++) {
    all_threads.emplace_back([&, i]() {
      TestRequest req;
      req.set_name("GROUP B");
      TestReply rep;
      grpc::ClientContext ctx;

      auto start = std::chrono::steady_clock::now();
      auto status = stub_b->Test(&ctx, req, &rep);
      auto now = std::chrono::steady_clock::now();

      long long latency_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
      atomic_max(unary_max_latency, latency_ms);

      {
        std::lock_guard<std::mutex> lg(unary_mutex);
        unary_latency_list.push_back(latency_ms);
      }
    });
  }

  for (auto& t : all_threads) t.join();

  auto global_end = std::chrono::steady_clock::now();
  long long total_ms = std::chrono::duration_cast<std::chrono::milliseconds>(global_end - global_start).count();

  {
    std::lock_guard<std::mutex> lg(stream_mutex);
    std::sort(stream_prev_list.begin(), stream_prev_list.end());
  }
  {
    std::lock_guard<std::mutex> lg(unary_mutex);
    std::sort(unary_latency_list.begin(), unary_latency_list.end());
  }

  long long stream_p50 = percentile(stream_prev_list, 0.50);
  long long stream_p90 = percentile(stream_prev_list, 0.90);
  long long stream_p95 = percentile(stream_prev_list, 0.95);
  long long stream_p99 = percentile(stream_prev_list, 0.99);

  long long unary_p50 = percentile(unary_latency_list, 0.50);
  long long unary_p90 = percentile(unary_latency_list, 0.90);
  long long unary_p95 = percentile(unary_latency_list, 0.95);
  long long unary_p99 = percentile(unary_latency_list, 0.99);

  std::cout << "\n===========================\n";
  std::cout << "        FINAL STATS        \n";
  std::cout << "===========================\n";
  std::cout << "Stream Max Δprev     : " << stream_max_prev.load() << " ms\n";
  std::cout << "Stream Δprev p50     : " << stream_p50 << " ms\n";
  std::cout << "Stream Δprev p90     : " << stream_p90 << " ms\n";
  std::cout << "Stream Δprev p95     : " << stream_p95 << " ms\n";
  std::cout << "Stream Δprev p99     : " << stream_p99 << " ms\n\n";

  std::cout << "Unary  Max Latency   : " << unary_max_latency.load() << " ms\n";
  std::cout << "Unary Latency p50    : " << unary_p50 << " ms\n";
  std::cout << "Unary Latency p90    : " << unary_p90 << " ms\n";
  std::cout << "Unary Latency p95    : " << unary_p95 << " ms\n";
  std::cout << "Unary Latency p99    : " << unary_p99 << " ms\n\n";

  std::cout << "TOTAL time           : " << total_ms << " ms\n";
  std::cout << "===========================\n";

  return 0;
}
