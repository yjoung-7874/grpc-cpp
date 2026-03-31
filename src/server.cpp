#include "grpc_wrapper/server.hpp"

namespace grpc_wrapper {

GrpcServer::GrpcServer() {}

GrpcServer::~GrpcServer() {
  Shutdown();
}

void GrpcServer::AddListeningPort(const std::string& address) {
  external_addresses_.push_back(address);
}

void GrpcServer::RegisterService(grpc::Service* service) {
  if (service != nullptr) {
    services_.push_back(service);
  }
}

bool GrpcServer::Start() {
  if (external_addresses_.empty()) {
    std::cerr << "GrpcServer: No listening port specified." << std::endl;
    return false;
  }
  if (services_.empty()) {
    std::cerr << "GrpcServer: No services registered." << std::endl;
    return false;
  }

  grpc::ServerBuilder builder;
  for (const auto& addr : external_addresses_) {
    builder.AddListeningPort(addr, grpc::InsecureServerCredentials());
  }

  for (auto* svc : services_) {
    builder.RegisterService(svc);
  }

  server_ = builder.BuildAndStart();

  if (server_) {
    for (const auto& addr : external_addresses_) {
      std::cout << "GrpcServer listening on " << addr << std::endl;
    }
    return true;
  } else {
    std::cerr << "GrpcServer failed to start." << std::endl;
    return false;
  }
}

void GrpcServer::Wait() {
  if (server_) {
    server_->Wait();
  }
}

void GrpcServer::Shutdown() {
  if (server_) {
    std::cout << "GrpcServer shutting down..." << std::endl;
    server_->Shutdown();
    server_.reset();
  }
}

} // namespace grpc_wrapper
