#include "server.hpp"

AsyncServer::AsyncServer(const std::string& address) : GrpcAsyncServerBase(address) {}
AsyncServer::~AsyncServer() = default;

void AsyncServer::InitServices() {
  service_list_.clear();
  auto test_api = std::make_unique<TestApiAsyncImpl>();
  service_list_.push_back(std::move(test_api));
}

int AsyncServer::GetServiceCQCount(grpc::Service* svc) {
  if (dynamic_cast<TestApiAsyncImpl*>(svc)) return 1;
  return 1;
}

void AsyncServer::InitRpcs() {
  for (auto& service : service_list_) {
    auto* svc = service.get();

    if (auto test_api_service = dynamic_cast<TestApiAsyncImpl*>(svc)) {
      int cq_count = GetServiceCQCount(svc);

      for (int i = 0; i < cq_count; ++i) {
        grpc::ServerCompletionQueue* cq = PickCQForService(svc);

        // Unary RPC
        StartRpc<TestApiAsyncImpl, TestRequest, TestReply, grpc::ServerAsyncResponseWriter<TestReply>>(
            test_api_service,
            [test_api_service, cq](grpc::ServerContext* ctx, TestRequest* req,
                                   grpc::ServerAsyncResponseWriter<TestReply>* responder, grpc::ServerCompletionQueue*,
                                   grpc::ServerCompletionQueue*,
                                   void* tag) { test_api_service->StartUnary(ctx, req, responder, cq, cq, tag); },
            [test_api_service](const TestRequest& req, TestReply& res) { test_api_service->HandleUnary(req, res); });

        // Server Stream
        StartServerStream<TestApiAsyncImpl, TestRequest, TestReply>(
            test_api_service,
            [test_api_service, cq](grpc::ServerContext* ctx, TestRequest* req, grpc::ServerAsyncWriter<TestReply>* responder,
                                   grpc::ServerCompletionQueue*, grpc::ServerCompletionQueue*,
                                   void* tag) { test_api_service->RequestTestStream(ctx, req, responder, cq, cq, tag); },
            [test_api_service](const TestRequest& req, grpc::ServerAsyncWriter<TestReply>* writer, auto* call_data) {
              test_api_service->HandleServerStream(req, writer, call_data);
            });

        // Bidi Stream
        StartBidi<TestApiAsyncImpl, TestRequest, TestReply>(
            test_api_service,
            [test_api_service, cq](grpc::ServerContext* ctx, grpc::ServerAsyncReaderWriter<TestReply, TestRequest>* responder,
                                   grpc::ServerCompletionQueue*, grpc::ServerCompletionQueue*,
                                   void* tag) { test_api_service->RequestTestBidiStream(ctx, responder, cq, cq, tag); },
            [test_api_service](TestRequest&& req, TestReply& res) { test_api_service->HandleBidi(std::move(req), res); });
      }
    }
  }
}
