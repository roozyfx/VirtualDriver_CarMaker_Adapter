#pragma once
#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>

#include <cstdint>
#include <memory>
#include <mutex>
#include <string>

#include "MockCarMakerInterface.h"
#include "sil_minimal.grpc.pb.h"

class SilCarMakerAdapter : public MockCarMakerInterface {
 public:
  explicit SilCarMakerAdapter(const std::string& address, const uint64_t egoId);
  ~SilCarMakerAdapter() override = default;

  int init() override;
  int testRunStart() override;
  int calc(double dt, double simTime) override;
  void readInputs(uint32_t cycleNo) override;
  void writeOutputs(uint32_t cycleNo) override;
  int testRunEnd() override;
  void cleanup() override;

 private:
  int handleStatus(const grpc::Status& status);

 private:
  mutable std::mutex mutex_;
  std::string server_address_;
  uint64_t egoId_;
  std::shared_ptr<grpc::Channel> channel_{nullptr};
  std::unique_ptr<sil::VirtualDriverService::Stub> stub_{nullptr};
  bool initialized_{false};
};