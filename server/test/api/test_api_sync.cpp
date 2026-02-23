#include "test_api_sync.hpp"

TestApi_A_SyncImpl::TestApi_A_SyncImpl() {}

grpc::Status TestApi_A_SyncImpl::Test(grpc::ServerContext* context, const TestRequest* request, TestReply* reply) {
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  reply->set_message("Hello " + request->name());
  return grpc::Status::OK;
}

grpc::Status TestApi_A_SyncImpl::TestStream(grpc::ServerContext* context, const TestRequest* request,
                                            grpc::ServerWriter<TestReply>* writer) {
  for (int i = 0; i < 50; ++i) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    TestReply reply;
    reply.set_message("Hello " + request->name() + " - message " + std::to_string(i + 1));
    writer->Write(reply);
  }
  return grpc::Status::OK;
}

grpc::Status TestApi_A_SyncImpl::TestBidiStream(grpc::ServerContext* context,
                                                grpc::ServerReaderWriter<TestReply, TestRequest>* stream) {
  TestRequest request;
  while (stream->Read(&request)) {
    TestReply reply;
    reply.set_message("Hello " + request.name());
    stream->Write(reply);
  }
  return grpc::Status::OK;
}

TestApi_B_SyncImpl::TestApi_B_SyncImpl() {}

grpc::Status TestApi_B_SyncImpl::Test(grpc::ServerContext* context, const TestRequest* request, TestReply* reply) {
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  reply->set_message("Hello " + request->name());
  return grpc::Status::OK;
}

grpc::Status TestApi_B_SyncImpl::TestStream(grpc::ServerContext* context, const TestRequest* request,
                                            grpc::ServerWriter<TestReply>* writer) {
  for (int i = 0; i < 50; ++i) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    TestReply reply;
    reply.set_message("Hello " + request->name() + " - message " + std::to_string(i + 1));
    writer->Write(reply);
  }
  return grpc::Status::OK;
}

grpc::Status TestApi_B_SyncImpl::TestBidiStream(grpc::ServerContext* context,
                                                grpc::ServerReaderWriter<TestReply, TestRequest>* stream) {
  TestRequest request;
  while (stream->Read(&request)) {
    TestReply reply;
    reply.set_message("Hello " + request.name());
    stream->Write(reply);
  }
  return grpc::Status::OK;
}