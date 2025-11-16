#ifndef GRPC_SYNC_SERVER_HPP_
#define GRPC_SYNC_SERVER_HPP_

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

#include "server/sync/base.hpp"
#include "api/test_api_sync.hpp"

class SyncServer : public GrpcSyncServerBase {
 public:
  SyncServer(const std::string& address);
  ~SyncServer();
  void InitServices() override;
};


#endif  // GRPC_SYNC_SERVER_HPP_