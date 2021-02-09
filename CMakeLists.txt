cmake_minimum_required(VERSION 3.0.0)
project(NESSimulator VERSION 0.1.0)

set (CMAKE_CXX_STANDARD 17)

add_library(nes_simulator_lib
    m6502.cpp
    Bus.cpp
    )

add_subdirectory(demo)

add_executable(NESSimulator main.cpp)

target_link_libraries(NESSimulator
    PRIVATE
        nes_simulator_lib
)
