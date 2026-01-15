## An Adapter between VirtualDriver and CarMaker


### ================================================================

Upon successful building, there will be two binaries available:
1) adapter_test, in [build_directory/]apps/adapter/adapter_test :
To test the adapter
2) mock_server, in [build_directory/]apps/mock_server/mock_server :
A very basic gRPC server for test purposes. Only capable of sending fixed values of acceleration and steering which can be passed to it as command-line arguments.
Usage:
`$ ./mock_server 0.87 .2'

TODO 
2) A Docker version of the above 
3) Asynchronous version

### Build
The project uses CMake to build. You can un/set the options to 
- build tests (currently missing), 
- use system gRPC or fetch and build it with the project

To build on a linux environment, using gcc compiler, follow these instructions:
`$ git clone https://github.com/roozyfx/VirtualDriver_CarMaker_Adapter.git some_dir`
`$ cd some_dir`
`$ mkdir build`
`$ cmake -DCMAKE_BUILD_TYPE:STRING=Release -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE -DCMAKE_C_COMPILER:FILEPATH=$(which gcc) -DCMAKE_CXX_COMPILER:FILEPATH=$(which g++) --no-warn-unused-cli -S . -B ./build -G Ninja`

### Dependencies
- C++23
- cmake
- Google Protobuf (https://protobuf.dev/)
- gRPC (https://grpc.io/)
TODO!
- Toml++ (https://marzer.github.io/tomlplusplus/)
- Google Test (https://github.com/google/googletest)
