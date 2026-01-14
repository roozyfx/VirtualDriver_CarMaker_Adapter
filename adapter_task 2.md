# Task: Implement a Minimal CarMaker-to-Virtual Driver Adapter

## Overview

### What is CarMaker?

CarMaker is a professional vehicle dynamics simulation software developed by IPG Automotive. It provides a comprehensive simulation environment for testing and developing automotive systems, including:

- **Vehicle dynamics modeling**: Realistic physics simulation of vehicles, including powertrain, suspension, steering, and braking systems
- **Environment simulation**: Road networks, traffic scenarios, weather conditions, and sensor models
- **Real-time simulation**: Capable of running simulations in real-time or faster-than-real-time for testing and validation
- **Integration capabilities**: Designed to interface with external systems, ECUs, and software-in-the-loop (SiL) components

### What is a Virtual Driver?

A virtual driver (also known as a Software-in-the-Loop or SiL driver) is an external software component that implements autonomous driving or driver assistance algorithms. It makes driving decisions based on:

- **Perception**: Understanding the environment from sensor data (cameras, LiDAR, radar)
- **Planning**: Deciding on the desired path, speed, and maneuvers
- **Control**: Computing control commands (acceleration, steering, braking)

The virtual driver runs independently from the simulation environment and communicates via standardized interfaces.


### Your Task
Implement a minimal adapter that connects to a virtual driver via gRPC using the provided proto files. The adapter sends ego vehicle state and receives control commands.


## What You Need to Do

1. **Implement the adapter class** 
   - Use the provided proto files 
   - Implement `MockCarMakerInterface` (provided below)
   - Use **synchronous gRPC calls** 
   - Send ego state in `tick()` requests
   - Apply control commands from `tick()` replies

2. **Create a simple test program** 
   - Use the provided mock interface
   - Run a basic simulation loop
   - Verify the adapter works

3. **Write brief documentation**
   - Explain key design decisions (1-2 pages)
   - Document assumptions

## Provided Proto File

Create a file `sil_minimal.proto` with the following content:

```protobuf
syntax = "proto3";

package sil;

// Ego vehicle state
message EgoState {
  float pos_x = 1;        // Position X [m]
  float pos_y = 2;        // Position Y [m]
  float velocity = 3;     // Velocity [m/s]
  float heading = 4;      // Heading angle [rad]
  float yaw_rate = 5;     // Yaw rate [rad/s]
}

// Control commands from virtual driver
message ControlCommand {
  float accel_cmd = 1;    // Acceleration command [m/s²]
  float curv_cmd = 2;     // Curvature command [1/m] (steering)
}

// Status messages
message Status {
  bool success = 1;
  string error_message = 2;
}

// Initialization request
message InitRequest {
  uint64 ego_id = 1;
  EgoState initial_state = 2;
}

// Initialization reply
message InitReply {
  Status status = 1;
}

// Tick request (sent every cycle)
message TickRequest {
  uint64 timestamp_us = 1;  // Timestamp in microseconds
  EgoState ego_state = 2;
}

// Tick reply (control commands)
message TickReply {
  Status status = 1;
  ControlCommand control = 2;
}

// Shutdown request
message ShutdownRequest {
  enum Reason {
    FINISHED = 0;
    ABORTED = 1;
    ERROR = 2;
  }
  Reason reason = 1;
}

// Shutdown reply
message ShutdownReply {
  Status status = 1;
}

// Service definition
service VirtualDriverService {
  rpc init(InitRequest) returns(InitReply);
  rpc tick(TickRequest) returns(TickReply);
  rpc shutdown(ShutdownRequest) returns(ShutdownReply);
}
```

## Provided Mock CarMaker Interface

### MockCarMakerInterface.h

```cpp
// MockCarMakerInterface.h
#ifndef MOCK_CARMAKER_INTERFACE_H
#define MOCK_CARMAKER_INTERFACE_H

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

extern MockVehicleState g_vehicleState;
extern MockControlInputs g_controlInputs;

#endif
```

### MockCarMakerInterface.cpp

```cpp
// MockCarMakerInterface.cpp
#include "MockCarMakerInterface.h"
#include <cmath>

MockVehicleState g_vehicleState;
MockControlInputs g_controlInputs;

void updateMockVehicle(double dt) {
    double v = g_vehicleState.velocity;
    double heading = g_vehicleState.heading;
    
    v += g_controlInputs.accel_cmd * dt;
    if (v < 0.0) v = 0.0;
    
    double wheelbase = 2.5;
    g_vehicleState.yaw_rate = v * g_controlInputs.curv_cmd;
    g_vehicleState.heading += g_vehicleState.yaw_rate * dt;
    
    g_vehicleState.pos_x += v * cos(heading) * dt;
    g_vehicleState.pos_y += v * sin(heading) * dt;
    g_vehicleState.velocity = v;
}
```

## Test Harness Skeleton

Complete this test harness:

```cpp
// test_main.cpp
#include "MockCarMakerInterface.h"
#include "YourAdapter.h"  // Your adapter class
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    // Create your adapter instance
    YourAdapter adapter("localhost:50051");  // Adjust as needed
    
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
            std::cout << "Time: " << simTime 
                      << "s, Pos: (" << g_vehicleState.pos_x 
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
```

## Your Implementation Task

Create `YourAdapter` class that:

1. Inherits from `MockCarMakerInterface`
2. Connects to gRPC server in `testRunStart()`
3. Sends `InitRequest` with initial ego state
4. In `writeOutputs()`: sends `TickRequest` with current ego state
5. In `readInputs()`: receives `TickReply` and applies control to `g_controlInputs`
6. Sends `ShutdownRequest` in `testRunEnd()`

**Use synchronous gRPC** . Example pattern:

```cpp
// Pseudo-code example
grpc::ClientContext context;
sil::TickRequest request;
request.set_timestamp(/* current time */);
// ... fill ego state ...

sil::TickReply reply;
grpc::Status status = stub_->tick(&context, request, &reply);

if (status.ok()) {
    g_controlInputs.accel_cmd = reply.control().accel_cmd();
    g_controlInputs.curv_cmd = reply.control().curv_cmd();
}
```

## Deliverables

1. **Source code:**
   - Your adapter class (`.h` and `.cpp`)
   - Completed test harness
   - `CMakeLists.txt` that builds everything
   - `README.md` with build/run instructions

2. **Brief documentation** (1-2 pages):
   - How your adapter works
   - Key design decisions (why synchronous gRPC, error handling approach, etc.)
   - Assumptions made

## Build Requirements
- C++17
- CMake
- gRPC and Protocol Buffers

## Testing

For a quick mock server test, the server can return fixed control commands.

## Optional Enhancements

If you have extra time or want to demonstrate additional skills, consider:

1. **Asynchronous gRPC Implementation** - Implement the adapter using asynchronous gRPC calls instead of synchronous ones.

2. **Monitoring and Logging** - Add logging and monitoring capabilities.

## Success Criteria

* Code compiles and links  
* Adapter connects to gRPC server  
* Sends ego state in tick requests  
* Receives and applies control commands  
* Handles init and shutdown  
* Brief documentation explains approach  

Focus on a **working minimal solution**. Good luck!
