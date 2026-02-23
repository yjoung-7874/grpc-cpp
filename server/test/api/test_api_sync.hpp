#ifndef UNICON_COMMUNICATION_TEST_API_SYNC_H_
#define UNICON_COMMUNICATION_TEST_API_SYNC_H_

#include <grpcpp/grpcpp.h>
#include <memory>
#include <string>

#include "proto/test_api.grpc.pb.h"
#include "server/sync/base.hpp"

// using namespace unicon::proto::test;

class TestApi_A_SyncImpl final : public TestApi_A::Service {
 public:
  TestApi_A_SyncImpl();

  grpc::Status Test(grpc::ServerContext* context, const TestRequest* request, TestReply* reply) override;
  grpc::Status TestStream(grpc::ServerContext* context, const TestRequest* request, grpc::ServerWriter<TestReply>* writer) override;
  grpc::Status TestBidiStream(grpc::ServerContext* context, grpc::ServerReaderWriter<TestReply, TestRequest>* stream) override;
};

class TestApi_B_SyncImpl final : public TestApi_B::Service {
 public:
  TestApi_B_SyncImpl();

  grpc::Status Test(grpc::ServerContext* context, const TestRequest* request, TestReply* reply) override;
  grpc::Status TestStream(grpc::ServerContext* context, const TestRequest* request, grpc::ServerWriter<TestReply>* writer) override;
  grpc::Status TestBidiStream(grpc::ServerContext* context, grpc::ServerReaderWriter<TestReply, TestRequest>* stream) override;
};


#endif  // UNICON_COMMUNICATION_TEST_API_SYNC_H_