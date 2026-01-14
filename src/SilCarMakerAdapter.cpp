#include <chrono>
#include <iostream>

#include "SilCarMakerAdapter.h"

using grpc::Channel;
using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;
using grpc::Status;

SilCarMakerAdapter::SilCarMakerAdapter(const std::string& address,
                                       const uint64_t egoId)
    : mutex_{}, server_address_{address}, egoId_{egoId} {}

// Initialize the gRPC client and prepare for communication
int SilCarMakerAdapter::init() {
  std::lock_guard l(mutex_);
  channel_ =
      grpc::CreateChannel(server_address_, grpc::InsecureChannelCredentials());
  if (!channel_) {
    std::cerr << "Failed to create gRPC channel to " << server_address_
              << std::endl;
    return -1;
  }
  stub_ = sil::VirtualDriverService::NewStub(channel_);
  if (!stub_) {
    std::cerr << "Failed to create gRPC stub for VirtualDriverService"
              << std::endl;
    return -1;
  }

  const auto timeout{std::chrono::system_clock::now() +
                     std::chrono::seconds(2)};
  if (!channel_->WaitForConnected(timeout)) {
    std::cerr << "Failed to connect to gRPC server at " << server_address_
              << std::endl;
    return -1;
  }
  return 0;
}

// Connect to gRPC server and prepare for communication
int SilCarMakerAdapter::testRunStart() {
  if (!stub_) return -1;

  grpc::ClientContext context;
  sil::InitRequest request;
  request.set_ego_id(egoId_);
  // TODO Set initial state appropriately. Use config file or parameters.
  sil::EgoState* initialState = request.mutable_initial_state();
  initialState->set_pos_x(0.0);
  initialState->set_pos_y(0.0);
  initialState->set_velocity(0.0);
  initialState->set_heading(0.0);
  initialState->set_yaw_rate(0.0);

  sil::InitReply response;
  auto status{stub_->init(&context, request, &response)};

  if (handleStatus(status) != 0) {
    return -1;
  }
  if (!response.status().success()) {
    std::cerr << "Initialization failed: " << response.status().error_message()
              << std::endl;
    return -1;
  }
  initialized_ = true;
  return 0;
}

int SilCarMakerAdapter::calc(double /*dt*/, double /*simTime*/) {
  // TODO calculation updates
  return 0;
}

void SilCarMakerAdapter::readInputs(uint32_t /*cycleNo*/) {
  if (!stub_) return;

  grpc::ClientContext context;
  sil::TickRequest request;
  sil::TickReply response;

  stub_->tick(&context, request, &response);

  if (!response.status().success()) {
    std::cerr << "Reading ControlCommand, Tick Reply not successful: "
              << response.status().error_message() << std::endl;
    return;
  }
  const auto accelerationCmd{response.control().accel_cmd()};
  const auto curvatureCmd{response.control().curv_cmd()};
  g_controlInputs.accel_cmd = accelerationCmd;
  g_controlInputs.curv_cmd = curvatureCmd;
}

void SilCarMakerAdapter::writeOutputs(uint32_t /*cycleNo*/) {
  if (!stub_) return;

  grpc::ClientContext context;
  sil::TickRequest request;
  // TODO timestamp and cycleNo
  auto now = std::chrono::system_clock::now();
  auto now_us = std::chrono::duration_cast<std::chrono::microseconds>(
                    now.time_since_epoch())
                    .count();
  request.set_timestamp_us(now_us);
  sil::EgoState* egoState = request.mutable_ego_state();
  egoState->set_pos_x(g_vehicleState.pos_x);
  egoState->set_pos_y(g_vehicleState.pos_y);
  egoState->set_velocity(g_vehicleState.velocity);
  egoState->set_heading(g_vehicleState.heading);
  egoState->set_yaw_rate(g_vehicleState.yaw_rate);
  sil::TickReply response;

  auto status{stub_->tick(&context, request, &response)};
  handleStatus(status);
}

int SilCarMakerAdapter::testRunEnd() {
  if (!stub_) return -1;
  grpc::ClientContext context;
  sil::ShutdownRequest request;
  request.set_reason(sil::ShutdownRequest::FINISHED);
  sil::ShutdownReply response;

  auto status{stub_->shutdown(&context, request, &response)};
  if (handleStatus(status) != 0) {
    return -1;
  }
  return 0;
}

void SilCarMakerAdapter::cleanup() {
  // TODO
}

int SilCarMakerAdapter::handleStatus(const grpc::Status& status) {
  switch (status.error_code()) {
    case grpc::StatusCode::OK:
      return 0;
    case grpc::StatusCode::UNAVAILABLE:
      std::cerr << "gRPC server unavailable: " << status.error_message()
                << std::endl;
      return -1;
    case grpc::StatusCode::DEADLINE_EXCEEDED:
      std::cerr << "gRPC call timed out: " << status.error_message()
                << std::endl;
      return -1;
    default:
      std::cerr << "gRPC error (" << status.error_code()
                << "): " << status.error_message() << std::endl;
      return -1;
  }
  return -1;
}