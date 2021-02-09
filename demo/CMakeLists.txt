# OpenGL
set(OpenGL_GL_PREFERENCE LEGACY)
find_package(OpenGL REQUIRED)

# X11
find_package(X11 REQUIRED)

# PNG
find_package(PNG REQUIRED)

add_executable(Demo6502 Demo6502.cpp)

target_include_directories(Demo6502
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(Demo6502
    PRIVATE
        nes_simulator_lib
        OpenGL::GL
        PNG::PNG
        pthread
        X11
    )
