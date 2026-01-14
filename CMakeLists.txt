cmake_minimum_required(VERSION 3.30...4.2)

project(carmaker_sildriver VERSION 0.1.0
                        DESCRIPTION "Holon CarMaker-SiLDriver Adapter"
                        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Options
option(BUILD_TESTING "Build tests" ON)

add_subdirectory(src)
add_subdirectory(apps)

include_directories(${CMAKE_BINARY_DIR})

if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests) 
endif()