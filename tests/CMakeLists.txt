include(FetchContent)
FetchContent_Declare(
  googletest
  # version 1.17.0
  URL https://github.com/google/googletest/archive/52eb8108c5bdec04579160ae17225d66034bd723.zip 
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
# Prevent gtest from adding its own tests
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)


MACRO(add_test test_name)
  add_executable(${test_name} ${test_name}.cpp)
  target_link_libraries(${test_name} GTest::gtest_main)
  target_include_directories(${test_name} PRIVATE ../include)
  include(GoogleTest)
  gtest_discover_tests(${test_name})
ENDMACRO()

# add_test()
