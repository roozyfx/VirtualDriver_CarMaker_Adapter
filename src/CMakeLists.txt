set(PROJECT_LIBRARY_NAME sil_carmaker_adapter)

# TODO Add checks for Windows
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ ")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Weffc++")
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_compile_definitions("DEBUG")
endif()

set(src 
    MockCarMakerInterface.cpp
    SilCarMakerAdapter.cpp
)

add_library(${PROJECT_LIBRARY_NAME} STATIC ${src})

target_compile_options(${PROJECT_LIBRARY_NAME} PRIVATE 
    -Wall -Wextra -Wpedantic
    $<$<CONFIG:Debug>:-g;-O0;-fsanitize=address,undefined;-Wfloat-equal;-fno-omit-frame-pointer>
    $<$<CONFIG:Release>:-O3;-march=native;-DNDEBUG>)

find_package(Threads REQUIRED)

# First try to find user-compiled Protobuf, then system installation
set(CMAKE_FIND_PACKAGE_PREFER_CONFIG TRUE)
find_package(Protobuf)
unset(CMAKE_FIND_PACKAGE_PREFER_CONFIG)

if(Protobuf_FOUND)
  set(PROTO_OUTPUT_DIR ${CMAKE_BINARY_DIR}/protobuf)
  file(MAKE_DIRECTORY ${PROTO_OUTPUT_DIR})

  protobuf_generate(
    TARGET ${PROJECT_LIBRARY_NAME}
    LANGUAGE cpp
    PROTOC_OUT_DIR ${PROTO_OUTPUT_DIR}
    IMPORT_DIRS protos/
    PROTOS 
    protos/sil_minimal.proto
  )
  protobuf_generate(
    TARGET ${PROJECT_LIBRARY_NAME}
    LANGUAGE grpc
    PROTOC_OUT_DIR ${PROTO_OUTPUT_DIR} 
    PLUGIN protoc-gen-grpc=$<TARGET_FILE:gRPC::grpc_cpp_plugin>
    PLUGIN_OPTIONS generate_mock_code=true
    GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc
    IMPORT_DIRS protos/
    PROTOS 
    protos/sil_minimal.proto
  )
else()
  message(FATAL_ERROR "Protobuf not found. Please install Protobuf or set USE_SYSTEM_PROTOBUF to OFF to build Protobuf from source.")
endif()

# TODO Turn this off by default and make the fetch option work

if(USE_SYSTEM_GRPC)
  # Find system-installed gRPC
  find_package(gRPC CONFIG REQUIRED)
else()
  include(FetchContent)
  FetchContent_Declare(
    gRPC
    GIT_REPOSITORY https://github.com/grpc/grpc
    GIT_TAG        v1.76.0
  )
  set(FETCHCONTENT_QUIET OFF)
  FetchContent_MakeAvailable(gRPC)
endif()

target_include_directories(${PROJECT_LIBRARY_NAME} PUBLIC ${PROTO_OUTPUT_DIR} ../include)

if(CMAKE_BUILD_TYPE MATCHES "Debug")
    target_link_options(${PROJECT_LIBRARY_NAME} BEFORE PUBLIC -fsanitize=undefined PUBLIC -fsanitize=address)
endif()

target_link_libraries(${PROJECT_LIBRARY_NAME} PUBLIC Threads::Threads gRPC::grpc++ protobuf::libprotobuf)