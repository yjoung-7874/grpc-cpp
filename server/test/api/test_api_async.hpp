#ifndef GRPC_TEST_API_ASYNC_H_
#define GRPC_TEST_API_ASYNC_H_

#include <grpcpp/grpcpp.h>
#include <memory>
#include <string>

#include "proto/test_api.grpc.pb.h"
#include "server/async/base.hpp"

using namespace unicon::proto::test;

class TestApiAsyncImpl final : public TestApi::AsyncService {
 public:
  TestApiAsyncImpl();
  void StartUnary(grpc::ServerContext* ctx, TestRequest* req, grpc::ServerAsyncResponseWriter<TestReply>* responder,
                  grpc::ServerCompletionQueue* cq1, grpc::ServerCompletionQueue* cq2, void* tag);

  void StartServerStream(grpc::ServerContext* ctx, TestRequest* req, grpc::ServerAsyncWriter<TestReply>* writer,
                         grpc::ServerCompletionQueue* cq1, grpc::ServerCompletionQueue* cq2, void* tag);

  void StartBidi(grpc::ServerContext* ctx, grpc::ServerAsyncReaderWriter<TestReply, TestRequest>* stream,
                 grpc::ServerCompletionQueue* cq1, grpc::ServerCompletionQueue* cq2, void* tag);

  void HandleUnary(const TestRequest& req, TestReply& res);
  void HandleServerStream(const TestRequest& req, grpc::ServerAsyncWriter<TestReply>* writer,
                          GrpcAsyncServerBase::ServerStreamCallData<TestRequest, TestReply>* call_data);
  void HandleBidi(TestRequest&& req, TestReply& res);
};

#endif  // GRPC_TEST_API_ASYNC_H_