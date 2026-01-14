#pragma once
#include <cstdint>
#include <memory>
#include <string>

#include "MockCarMakerInterface.h"
#include "sil_minimal.grpc.pb.h"

class SilCarMakerAdapter : public MockCarMakerInterface {
 public:
  explicit SilCarMakerAdapter(const std::string& address);
  ~SilCarMakerAdapter() override;

  int init() override;
  int testRunStart() override;
  int calc(double dt, double simTime) override;
  void readInputs(uint32_t cycleNo) override;
  void writeOutputs(uint32_t cycleNo) override;
  int testRunEnd() override;
  void cleanup() override;

 private:
  std::string server_address_;
  std::unique_ptr<sil::VirtualDriverService::Stub> stub_;
};