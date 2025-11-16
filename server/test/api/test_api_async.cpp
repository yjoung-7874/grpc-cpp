#include "test_api_async.hpp"

TestApiAsyncImpl::TestApiAsyncImpl() {}

void TestApiAsyncImpl::StartUnary(grpc::ServerContext* ctx, TestRequest* req, grpc::ServerAsyncResponseWriter<TestReply>* responder,
                             grpc::ServerCompletionQueue* cq1, grpc::ServerCompletionQueue* cq2, void* tag) {
  this->RequestTest(ctx, req, responder, cq1, cq2, tag);
}

void TestApiAsyncImpl::StartServerStream(grpc::ServerContext* ctx, TestRequest* req, grpc::ServerAsyncWriter<TestReply>* writer,
                                    grpc::ServerCompletionQueue* cq1, grpc::ServerCompletionQueue* cq2, void* tag) {
  this->RequestTestStream(ctx, req, writer, cq1, cq2, tag);
}

void TestApiAsyncImpl::StartBidi(grpc::ServerContext* ctx, grpc::ServerAsyncReaderWriter<TestReply, TestRequest>* stream,
                            grpc::ServerCompletionQueue* cq1, grpc::ServerCompletionQueue* cq2, void* tag) {
  this->RequestTestBidiStream(ctx, stream, cq1, cq2, tag);
}

void TestApiAsyncImpl::HandleUnary(const TestRequest& req, TestReply& res) {
  std::this_thread::sleep_for(std::chrono::seconds(3));
  res.set_message("Hello " + req.name());
}

void TestApiAsyncImpl::HandleServerStream(const TestRequest& req, grpc::ServerAsyncWriter<TestReply>* writer,
                                     GrpcAsyncServerBase::ServerStreamCallData<TestRequest, TestReply>* call_data) {
  static int count = 10;
  static int i = 0;
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  if (i >= count) {
    call_data->Finish();
    return;
  }

  TestReply reply;

  reply.set_message("Hello " + req.name() + " - message " + std::to_string(i + 1));
  ++i;

  call_data->WriteNext(reply);
}

void TestApiAsyncImpl::HandleBidi(TestRequest&& req, TestReply& res) { res.set_message("Hello " + req.name()); }