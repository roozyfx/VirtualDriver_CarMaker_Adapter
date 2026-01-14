// MockCarMakerInterface.cpp
#include <cmath>

#include "MockCarMakerInterface.h"

MockVehicleState g_vehicleState;
MockControlInputs g_controlInputs;

void updateMockVehicle(double dt) {
  double v = g_vehicleState.velocity;
  double heading = g_vehicleState.heading;

  v += g_controlInputs.accel_cmd * dt;
  if (v < 0.0) v = 0.0;

  // TODO Unused variable
  // double wheelbase = 2.5;
  g_vehicleState.yaw_rate = v * g_controlInputs.curv_cmd;
  g_vehicleState.heading += g_vehicleState.yaw_rate * dt;

  g_vehicleState.pos_x += v * cos(heading) * dt;
  g_vehicleState.pos_y += v * sin(heading) * dt;
  g_vehicleState.velocity = v;
}