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

// Initialize the gRPC client and establish connection
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
                     std::chrono::seconds(3)};
  if (!channel_->WaitForConnected(timeout)) {
    std::cerr << "Failed to connect to gRPC server at " << server_address_
              << std::endl;
    return -1;
  }
  return 0;
}

// Prepare for communication and send initial state
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

  sil::InitReply initReply;
  auto status{stub_->init(&context, request, &initReply)};

  if (handleStatus(status) != 0) {
    return -1;
  }
  if (!initReply.status().success()) {
    std::cerr << "Initialization failed: " << initReply.status().error_message()
              << std::endl;
    return -1;
  }
  initialized_ = true;
  return 0;
}

int SilCarMakerAdapter::calc([[maybe_unused]] double dt,
                             [[maybe_unused]] double simTime) {
  // Nothing to do here; work is done in readInputs and writeOutputs
  return initialized_ ? 0 : -1;
}

void SilCarMakerAdapter::readInputs(uint32_t /*cycleNo*/) {
  std::lock_guard l(mutex_);
  if (!initialized_ || !stub_) return;

  if (!havePendingTickReply_) {
    // TODO Think of a better way to handle this situation.
    // Nothing to do, so return
    return;
  }
  havePendingTickReply_ = false;

  if (!lastTickReply_.status().success()) {
    std::cerr << "Tick reply not successful: "
              << lastTickReply_.status().error_message() << std::endl;
  }

  // Apply control command from the stored reply
  if (lastTickReply_.has_control()) {
    g_controlInputs.accel_cmd = lastTickReply_.control().accel_cmd();
    g_controlInputs.curv_cmd = lastTickReply_.control().curv_cmd();
  } else {
    std::cerr << "Tick reply missing control command" << std::endl;
  }
}

void SilCarMakerAdapter::writeOutputs([[maybe_unused]] uint32_t cycleNo) {
  std::lock_guard l(mutex_);
  if (!initialized_ || !stub_) return;

  grpc::ClientContext context;
  // TODO Read from config
  context.set_deadline(std::chrono::system_clock::now() +
                       std::chrono::milliseconds(200));

  sil::TickRequest request;

  auto now{std::chrono::system_clock::now()};
  auto now_us{std::chrono::duration_cast<std::chrono::microseconds>(
                  now.time_since_epoch())
                  .count()};
  request.set_timestamp_us(static_cast<std::uint64_t>(now_us));

  auto* egoState = request.mutable_ego_state();
  egoState->set_pos_x(static_cast<float>(g_vehicleState.pos_x));
  egoState->set_pos_y(static_cast<float>(g_vehicleState.pos_y));
  egoState->set_velocity(static_cast<float>(g_vehicleState.velocity));
  egoState->set_heading(static_cast<float>(g_vehicleState.heading));
  egoState->set_yaw_rate(static_cast<float>(g_vehicleState.yaw_rate));

  sil::TickReply reply;
  auto status{stub_->tick(&context, request, &reply)};
  handleStatus(status);
  // Store the last reply and status for retrieval in readInputs
  lastTickStatus_ = status;
  lastTickReply_ = std::move(reply);
  havePendingTickReply_ = true;
}

int SilCarMakerAdapter::testRunEnd() {
  if (!initialized_ || !stub_) return -1;

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
  g_controlInputs = MockControlInputs{};
  g_vehicleState = MockVehicleState{};
  initialized_ = false;
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