#include "server.hpp"
#include <iostream>

SyncServer::SyncServer(const std::string& address) : GrpcSyncServerBase(address) {}
SyncServer::~SyncServer() = default;

void SyncServer::InitServices() {
  if (!service_list_.empty()) {
    service_list_.clear();
  }
  auto test_api_a = std::make_unique<TestApi_A_SyncImpl>();
  auto test_api_b = std::make_unique<TestApi_B_SyncImpl>();

  service_list_.push_back(std::move(test_api_a));
  service_list_.push_back(std::move(test_api_b));
}