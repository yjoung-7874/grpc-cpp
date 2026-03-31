#include "grpc_wrapper/client.hpp"
#include <iostream>
#include <chrono>

namespace grpc_wrapper {

GrpcClient::GrpcClient(const std::string& target_address) 
    : target_address_(target_address) {
  grpc::ChannelArguments args;
  args.SetMaxReceiveMessageSize(-1); // Unlimited
  args.SetMaxSendMessageSize(-1);    // Unlimited

  channel_ = grpc::CreateCustomChannel(
      target_address_, grpc::InsecureChannelCredentials(), args);
}

GrpcClient::~GrpcClient() {
  // Shared pointer handles cleanup
}

std::shared_ptr<grpc::Channel> GrpcClient::GetChannel() const {
  return channel_;
}

bool GrpcClient::WaitForReady(int timeout_ms) {
  if (!channel_) return false;

  auto deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(timeout_ms);
  
  // Try to connect to the channel
  auto state = channel_->GetState(true /* try_to_connect */);
  while (state != GRPC_CHANNEL_READY) {
    if (!channel_->WaitForStateChange(state, deadline)) {
      std::cerr << "GrpcClient: Failed to connect to " << target_address_ 
                << " within " << timeout_ms << "ms. Current state: " << state << std::endl;
      return false;
    }
    state = channel_->GetState(true);
  }
  return true;
}

} // namespace grpc_wrapper
