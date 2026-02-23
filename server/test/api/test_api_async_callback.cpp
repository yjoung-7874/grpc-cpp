#include "test_api_async_callback.hpp"
#include <thread>
#include <chrono>
#include <iostream>

grpc::ServerUnaryReactor* TestApi_A_AsyncCallbackImpl::Test(grpc::CallbackServerContext* ctx, const TestRequest* req,
                                                            TestReply* reply) {
  auto* reactor = ctx->DefaultReactor();

  std::thread([req, reply, reactor]() {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    reply->set_message("Hello, " + req->name());

    reactor->Finish(grpc::Status::OK);
  }).detach();

  return reactor;
}

grpc::ServerWriteReactor<TestReply>* TestApi_A_AsyncCallbackImpl::TestStream(grpc::CallbackServerContext* ctx,
                                                                             const TestRequest* req) {
  class StreamReactor : public grpc::ServerWriteReactor<TestReply> {
   public:
    StreamReactor(const std::string& name) : name_(name), count_(0), write_in_flight_(false) {
      worker_ = std::thread([this]() { this->ProduceLoop(); });
      worker_.detach();
    }

   private:
    void ProduceLoop() {
      for (int i = 1; i <= 50; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        {
          std::lock_guard<std::mutex> lk(mu_);
          reply_.set_message("Stream msg " + std::to_string(i) + " for " + name_);
        }

        while (true) {
          bool expected = false;
          if (write_in_flight_.compare_exchange_strong(expected, true)) {
            StartWrite(&reply_);
            break;
          } else {
            std::this_thread::yield();
          }
        }
      }

      Finish(grpc::Status::OK);
    }

    void OnWriteDone(bool ok) override {
      if (!ok) {
        write_in_flight_.store(false);
        Finish(grpc::Status::CANCELLED);
        return;
      }

      write_in_flight_.store(false);
    }

    void OnDone() override { delete this; }

    std::string name_;
    TestReply reply_;
    int count_;
    std::atomic<bool> write_in_flight_;
    std::thread worker_;
    std::mutex mu_;
  };

  return new StreamReactor(req->name());
}

grpc::ServerBidiReactor<TestRequest, TestReply>* TestApi_A_AsyncCallbackImpl::TestBidiStream(grpc::CallbackServerContext* ctx) {
  class BidiReactor : public grpc::ServerBidiReactor<TestRequest, TestReply> {
   public:
    BidiReactor() { StartRead(&read_msg_); }

   private:
    void OnReadDone(bool ok) override {
      if (!ok) {
        Finish(grpc::Status::OK);
        return;
      }

      TestReply reply;
      reply.set_message("Echo: " + read_msg_.name());
      StartWrite(&reply);
    }

    void OnWriteDone(bool ok) override {
      if (!ok) {
        Finish(grpc::Status::CANCELLED);
        return;
      }
      StartRead(&read_msg_);
    }

    void OnDone() override { delete this; }

    TestRequest read_msg_;
  };

  return new BidiReactor();
}

grpc::ServerUnaryReactor* TestApi_B_AsyncCallbackImpl::Test(grpc::CallbackServerContext* ctx, const TestRequest* req,
                                                            TestReply* reply) {
  auto* reactor = ctx->DefaultReactor();

  std::thread([req, reply, reactor]() {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    reply->set_message("Hello, " + req->name());

    reactor->Finish(grpc::Status::OK);
  }).detach();

  return reactor;
}

grpc::ServerWriteReactor<TestReply>* TestApi_B_AsyncCallbackImpl::TestStream(grpc::CallbackServerContext* ctx,
                                                                             const TestRequest* req) {
  class StreamReactor : public grpc::ServerWriteReactor<TestReply> {
   public:
    StreamReactor(const std::string& name) : name_(name), count_(0), write_in_flight_(false) {
      worker_ = std::thread([this]() { this->ProduceLoop(); });
      worker_.detach();
    }

   private:
    void ProduceLoop() {
      for (int i = 1; i <= 50; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        {
          std::lock_guard<std::mutex> lk(mu_);
          reply_.set_message("Stream msg " + std::to_string(i) + " for " + name_);
        }

        while (true) {
          bool expected = false;
          if (write_in_flight_.compare_exchange_strong(expected, true)) {
            StartWrite(&reply_);
            break;
          } else {
            std::this_thread::yield();
          }
        }
      }

      Finish(grpc::Status::OK);
    }

    void OnWriteDone(bool ok) override {
      if (!ok) {
        write_in_flight_.store(false);
        Finish(grpc::Status::CANCELLED);
        return;
      }

      write_in_flight_.store(false);
    }

    void OnDone() override { delete this; }

    std::string name_;
    TestReply reply_;
    int count_;
    std::atomic<bool> write_in_flight_;
    std::thread worker_;
    std::mutex mu_;
  };

  return new StreamReactor(req->name());
}

grpc::ServerBidiReactor<TestRequest, TestReply>* TestApi_B_AsyncCallbackImpl::TestBidiStream(grpc::CallbackServerContext* ctx) {
  class BidiReactor : public grpc::ServerBidiReactor<TestRequest, TestReply> {
   public:
    BidiReactor() { StartRead(&read_msg_); }

   private:
    void OnReadDone(bool ok) override {
      if (!ok) {
        Finish(grpc::Status::OK);
        return;
      }

      TestReply reply;
      reply.set_message("Echo: " + read_msg_.name());
      StartWrite(&reply);
    }

    void OnWriteDone(bool ok) override {
      if (!ok) {
        Finish(grpc::Status::CANCELLED);
        return;
      }
      StartRead(&read_msg_);
    }

    void OnDone() override { delete this; }

    TestRequest read_msg_;
  };

  return new BidiReactor();
}