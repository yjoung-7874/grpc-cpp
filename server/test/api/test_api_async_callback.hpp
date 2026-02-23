#ifndef UNICON_COMMUNICATION_TEST_API_ASYNC_CB_IMPL_H_
#define UNICON_COMMUNICATION_TEST_API_ASYNC_CB_IMPL_H_

#include <grpcpp/grpcpp.h>
#include "proto/test_api.grpc.pb.h"
#include "server/async_callback/base.hpp"

class TestApi_A_AsyncCallbackImpl final : public TestApi_A::CallbackService {
 public:
  grpc::ServerUnaryReactor* Test(grpc::CallbackServerContext* ctx, const TestRequest* req, TestReply* reply) override;
  grpc::ServerWriteReactor<TestReply>* TestStream(grpc::CallbackServerContext* ctx, const TestRequest* req) override;
  grpc::ServerBidiReactor<TestRequest, TestReply>* TestBidiStream(grpc::CallbackServerContext* ctx) override;
};

class TestApi_B_AsyncCallbackImpl final : public TestApi_B::CallbackService {
 public:
  grpc::ServerUnaryReactor* Test(grpc::CallbackServerContext* ctx, const TestRequest* req, TestReply* reply) override;
  grpc::ServerWriteReactor<TestReply>* TestStream(grpc::CallbackServerContext* ctx, const TestRequest* req) override;
  grpc::ServerBidiReactor<TestRequest, TestReply>* TestBidiStream(grpc::CallbackServerContext* ctx) override;
};

#endif  // UNICON_COMMUNICATION_TEST_API_ASYNC_CB_IMPL_H_