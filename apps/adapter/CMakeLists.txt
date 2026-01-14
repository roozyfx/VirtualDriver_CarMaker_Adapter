set(GENERATOR_BINARY adapter_test)

add_executable(${GENERATOR_BINARY} test_harness.cpp)
target_compile_options(${GENERATOR_BINARY} PRIVATE 
    -Wall -Wextra -Wpedantic
    $<$<CONFIG:Debug>:-g;-O0;-fsanitize=address,undefined;-Wfloat-equal;-fno-omit-frame-pointer>
    $<$<CONFIG:Release>:-O3;-march=native;-DNDEBUG>)

target_link_libraries(${GENERATOR_BINARY} PRIVATE sil_carmaker_adapter)
