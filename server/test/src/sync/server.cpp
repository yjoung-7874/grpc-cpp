#include "server.hpp"

#include <iostream>

SyncServer::SyncServer(const std::string& address) : GrpcSyncServerBase(address) {}
SyncServer::~SyncServer() = default;

void SyncServer::InitServices() {
  if (!service_list_.empty()) {
    service_list_.clear();
  }
  auto test_api = std::make_unique<TestApiSyncImpl>();
  service_list_.push_back(std::move(test_api));
}