cmake_minimum_required(VERSION 3.0.0)
project(NESSimulator VERSION 0.1.0)

set (CMAKE_CXX_STANDARD 17)

include(CTest)
enable_testing()

add_library(nes_simulator_lib
    m6502.cpp
    Bus.cpp
    )

# OpenGL
find_package(X11 REQUIRED)
find_package(OpenGL REQUIRED COMPONENTS GLX OpenGL)
find_package(PNG REQUIRED)
set(OpenGL_GL_PREFERENCE LEGACY)

add_executable(NESSimulator DemoM6502.cpp)

target_link_libraries(NESSimulator
    PRIVATE
        nes_simulator_lib
        OpenGL::GL
        OpenGL::GLX
        PNG::PNG
        pthread
        X11
    )

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
