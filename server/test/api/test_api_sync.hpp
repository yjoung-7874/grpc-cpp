#ifndef GRPC_TEST_API_SYNC_H_
#define GRPC_TEST_API_SYNC_H_

#include <grpcpp/grpcpp.h>
#include <memory>
#include <string>

#include "proto/test_api.grpc.pb.h"
#include "server/sync/base.hpp"

using namespace unicon::proto::test;

class TestApiSyncImpl final : public TestApi::Service {
 public:
  TestApiSyncImpl();

  grpc::Status Test(grpc::ServerContext* context, const TestRequest* request, TestReply* reply) override;
  grpc::Status TestStream(grpc::ServerContext* context, const TestRequest* request, grpc::ServerWriter<TestReply>* writer) override;
  grpc::Status TestBidiStream(grpc::ServerContext* context, grpc::ServerReaderWriter<TestReply, TestRequest>* stream) override;
};

#endif  // GRPC_TEST_API_SYNC_H_