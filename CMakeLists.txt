cmake_minimum_required(VERSION 3.0.0)
project(NESSimulator VERSION 0.1.0)

set (CMAKE_CXX_STANDARD 17)

add_library(nes_simulator_lib
    m2C02.cpp
    m6502.cpp
    Bus.cpp
    Catridge.cpp
    )

target_include_directories(nes_simulator_lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/demo/
    )

add_subdirectory(demo)

add_executable(NESSimulator main.cpp)

target_link_libraries(NESSimulator
    PRIVATE
        nes_simulator_lib
)
