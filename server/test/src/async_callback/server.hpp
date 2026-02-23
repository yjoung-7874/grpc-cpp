#ifndef GRPC_ASYNC_CALLBACK_SERVER_HPP_
#define GRPC_ASYNC_CALLBACK_SERVER_HPP_

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
#include "api/test_api_async_callback.hpp"
#include "server/async_callback/base.hpp"

class AsyncCallbackServer : public GrpcAsyncCallbackServerBase {
 public:
  explicit AsyncCallbackServer(const std::string& address);
  ~AsyncCallbackServer();

  void InitServices() override;
};

#endif  // GRPC_ASYNC_CALLBACK_SERVER_HPP_
