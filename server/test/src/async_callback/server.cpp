#include "server.hpp"

AsyncCallbackServer::AsyncCallbackServer(const std::string& address) : GrpcAsyncCallbackServerBase(address) {}
AsyncCallbackServer::~AsyncCallbackServer() = default;

void AsyncCallbackServer::InitServices() {
  service_list_.clear();
  auto test_api_a = std::make_unique<TestApi_A_AsyncCallbackImpl>();
  auto test_api_b = std::make_unique<TestApi_B_AsyncCallbackImpl>();
  service_list_.push_back(std::move(test_api_a));
  service_list_.push_back(std::move(test_api_b));
}