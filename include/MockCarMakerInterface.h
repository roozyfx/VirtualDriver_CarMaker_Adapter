// MockCarMakerInterface.h
#pragma once

#include <cstdint>

class MockCarMakerInterface {
 public:
  virtual ~MockCarMakerInterface() = default;
  virtual int init() = 0;
  virtual int testRunStart() = 0;
  virtual int calc(double dt, double simTime) = 0;
  virtual void readInputs(uint32_t cycleNo) = 0;
  virtual void writeOutputs(uint32_t cycleNo) = 0;
  virtual int testRunEnd() = 0;
  virtual void cleanup() = 0;
};

struct MockVehicleState {
  double pos_x = 0.0;
  double pos_y = 0.0;
  double velocity = 0.0;
  double heading = 0.0;
  double yaw_rate = 0.0;
};

struct MockControlInputs {
  double accel_cmd = 0.0;
  double curv_cmd = 0.0;
};

void updateMockVehicle(double dt);

extern MockVehicleState g_vehicleState;
extern MockControlInputs g_controlInputs;
