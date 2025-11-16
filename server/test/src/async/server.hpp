#ifndef GRPC_ASYNC_SERVER_HPP_
#define GRPC_ASYNC_SERVER_HPP_

#include <grpcpp/grpcpp.h>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <utility>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/log/check.h"
#include "absl/strings/str_format.h"

#include "server/async/base.hpp"
#include "api/test_api_async.hpp"

class AsyncServer : public GrpcAsyncServerBase {
 public:
  AsyncServer(const std::string& address);
  ~AsyncServer();

  void InitServices() override;
  void InitRpcs() override;
  int GetServiceCQCount(grpc::Service* svc) override;
};


#endif  // GRPC_ASYNC_SERVER_HPP_