// server_main.cpp
#include <grpcpp/grpcpp.h>

#include <iostream>
#include <memory>
#include <string>

#include "sil_minimal.grpc.pb.h"
#include "sil_minimal.pb.h"

class VirtualDriverServiceImpl final
    : public sil::VirtualDriverService::Service {
 public:
  explicit VirtualDriverServiceImpl(float fixed_accel_cmd, float fixed_curv_cmd)
      : accelCmd_(fixed_accel_cmd), curvCmd_(fixed_curv_cmd) {}

  grpc::Status init(grpc::ServerContext*, const sil::InitRequest* request,
                    sil::InitReply* reply) override {
    // Accept all inits; you can validate request->ego_id(),
    // request->initial_state(), etc.
    auto* st = reply->mutable_status();
    st->set_success(true);
    st->set_error_message("");

    std::cout << "[init] ego_id=" << request->ego_id()
              << " initial(pos_x=" << request->initial_state().pos_x()
              << ", pos_y=" << request->initial_state().pos_y()
              << ", v=" << request->initial_state().velocity()
              << ", heading=" << request->initial_state().heading()
              << ", yaw_rate=" << request->initial_state().yaw_rate() << ")\n";
    return grpc::Status::OK;
  }

  grpc::Status tick(grpc::ServerContext*, const sil::TickRequest* request,
                    sil::TickReply* reply) override {
    // Always respond with fixed control command.
    auto* st = reply->mutable_status();
    st->set_success(true);
    st->set_error_message("");

    auto* ctrl = reply->mutable_control();
    ctrl->set_accel_cmd(accelCmd_);
    ctrl->set_curv_cmd(curvCmd_);

    // Optional logging
    std::cout << "[tick] ts_us=" << request->timestamp_us()
              << " ego(pos_x=" << request->ego_state().pos_x()
              << ", pos_y=" << request->ego_state().pos_y()
              << ", v=" << request->ego_state().velocity()
              << ") -> ctrl(accel=" << accelCmd_ << ", curv=" << curvCmd_
              << ")\n";

    return grpc::Status::OK;
  }

  grpc::Status shutdown(grpc::ServerContext*,
                        const sil::ShutdownRequest* request,
                        sil::ShutdownReply* reply) override {
    auto* st = reply->mutable_status();
    st->set_success(true);
    st->set_error_message("");

    std::cout << "[shutdown] reason=" << request->reason() << "\n";
    return grpc::Status::OK;
  }

 private:
  float accelCmd_;
  float curvCmd_;
};

int main(int argc, char** argv) {
  // Fixed commands (can be overridden by CLI args)
  float accelCmd{0.5f};  // m/s^2
  float curvCmd{0.0f};   // 1/m

  if (argc >= 2) accelCmd = std::stof(argv[1]);
  if (argc >= 3) curvCmd = std::stof(argv[2]);

  const std::string serverAddress{"localhost:50051"};

  VirtualDriverServiceImpl service(accelCmd, curvCmd);

  grpc::ServerBuilder builder;
  builder.AddListeningPort(serverAddress, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);

  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  if (!server) {
    std::cerr << "Failed to start server on " << serverAddress << "\n";
    return 1;
  }

  std::cout << "VirtualDriverService server listening on " << serverAddress
            << " (fixed accel_cmd=" << accelCmd << ", curv_cmd=" << curvCmd
            << ")\n";

  server->Wait();
  return 0;
}
