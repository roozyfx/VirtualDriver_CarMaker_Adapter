set(GENERATOR_BINARY mock_server)

add_executable(${GENERATOR_BINARY} mock_server.cpp)
target_compile_options(${GENERATOR_BINARY} PRIVATE 
    -Wall -Wextra -Wpedantic
    $<$<CONFIG:Debug>:-g;-O0;-fsanitize=address,undefined;-Wfloat-equal;-fno-omit-frame-pointer>
    $<$<CONFIG:Release>:-O3;-march=native;-DNDEBUG>)

target_link_libraries(${GENERATOR_BINARY} PRIVATE sil_carmaker_adapter)
