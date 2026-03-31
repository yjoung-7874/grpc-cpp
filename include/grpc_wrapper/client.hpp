#ifndef GRPC_WRAPPER_CLIENT_HPP_
#define GRPC_WRAPPER_CLIENT_HPP_

#include <grpcpp/grpcpp.h>
#include <string>
#include <memory>

namespace grpc_wrapper {

class GrpcClient {
 public:
  // Create a channel targeting the specified address (e.g. "localhost:50051").
  // Defaults to InsecureChannelCredentials.
  explicit GrpcClient(const std::string& target_address);
  ~GrpcClient();

  // Get the underlying gRPC channel.
  // The generated stub can be initialized using this channel.
  std::shared_ptr<grpc::Channel> GetChannel() const;

  // Wait until the channel is in READY state, or timeout_ms milliseconds pass.
  // Returns true if the channel is ready, false on timeout.
  bool WaitForReady(int timeout_ms = 5000);

 private:
  std::shared_ptr<grpc::Channel> channel_;
  std::string target_address_;
};

} // namespace grpc_wrapper

#endif // GRPC_WRAPPER_CLIENT_HPP_
