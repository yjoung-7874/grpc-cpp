#include "server/sync/base.hpp"

#include <iostream>
#include <vector>

GrpcSyncServerBase::GrpcSyncServerBase(const std::string& address) : server_address_(address) {}

void GrpcSyncServerBase::Init() {
  InitServices();
  if (service_list_.empty()) {
    std::cout << "No service registered..." << std::endl;
    return;
  }
}

bool GrpcSyncServerBase::BuildAndStartServer() {
  if (server_address_.empty()) {
    std::cout << "Empty server address." << std::endl;
    return false;
  }

  grpc::ServerBuilder builder;
  builder.AddListeningPort(server_address_, grpc::InsecureServerCredentials());

  for (const auto& service : service_list_) {
    builder.RegisterService(service.get());
  }

  server_ = builder.BuildAndStart();

  if (server_) {
    std::cout << "Server started and listening on " << server_address_ << std::endl;
    return true;
  } else {
    std::cout << "Failed to start server on " << server_address_ << std::endl;
    return false;
  }
}

void GrpcSyncServerBase::NonBlockRun() {
  Init();
  if (!BuildAndStartServer()) {
    std::cerr << "Server failed to start." << std::endl;
    return;
  }
  // Launch background waiter
  wait_thread_ = std::thread([this]() { server_->Wait(); });
}

void GrpcSyncServerBase::Run() {
  Init();
  if (!BuildAndStartServer()) {
    std::cerr << "Server failed to start." << std::endl;
    return;
  }
  server_->Wait();
}

void GrpcSyncServerBase::Shutdown() {
  if (server_) {
    server_->Shutdown();
    std::cout << "Server shutdown on " << server_address_ << std::endl;
  }
  if (wait_thread_.joinable()) {
    wait_thread_.join();
  }
}