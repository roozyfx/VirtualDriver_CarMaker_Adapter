// test_main.cpp
#include <chrono>
#include <iostream>
#include <thread>

#include "MockCarMakerInterface.h"
#include "SilCarMakerAdapter.h"

int main() {
  // Create the adapter instance
  SilCarMakerAdapter adapter("localhost:50051", 1);

  // Initialize
  if (adapter.init() != 0) {
    std::cerr << "Init failed\n";
    return 1;
  }

  // Start test run
  if (adapter.testRunStart() != 0) {
    std::cerr << "Test run start failed\n";
    return 1;
  }

  // Simulation loop
  const double dt = 0.01;  // 10ms timestep
  double simTime = 0.0;
  uint32_t cycleNo = 0;

  for (int i = 0; i < 1000; ++i) {  // Run for 10 seconds
    // Update vehicle state based on previous control commands
    updateMockVehicle(dt);

    // Adapter reads inputs (applies control commands)
    adapter.readInputs(cycleNo);

    // Adapter writes outputs (sends ego state)
    adapter.writeOutputs(cycleNo);

    // Adapter calculation step
    adapter.calc(dt, simTime);

    simTime += dt;
    cycleNo++;

    // Print status every second
    if (i % 100 == 0) {
      std::cout << "Time: " << simTime << "s, Pos: (" << g_vehicleState.pos_x
                << ", " << g_vehicleState.pos_y << ")"
                << ", Vel: " << g_vehicleState.velocity << " m/s\n";
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  // End test run
  adapter.testRunEnd();
  adapter.cleanup();

  return 0;
}