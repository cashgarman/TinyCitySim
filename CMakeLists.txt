cmake_minimum_required(VERSION 3.20)

project(TinyCitySim CXX)

# Modern C++ (C++20): Standard version used for new code in maintained Win32 game clients.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SOURCES
    src/main.cpp
    src/Application.cpp
    src/Window.cpp
    src/D3D11Context.cpp
    src/TileGrid.cpp
    src/TileRenderer.cpp
    src/LogPanel.cpp
    src/InputHandler.cpp
)

add_executable(TinyCitySim WIN32 ${SOURCES})

target_include_directories(TinyCitySim PRIVATE src)

target_compile_definitions(TinyCitySim PRIVATE
    UNICODE
    _UNICODE
    NOMINMAX
    WIN32_LEAN_AND_MEAN
)

target_link_libraries(TinyCitySim PRIVATE
    d3d11
    dxgi
    d3dcompiler
    d2d1
    dwrite
    Shcore
)

if(MSVC)
    target_compile_options(TinyCitySim PRIVATE /W4 /permissive-)
endif()

add_custom_command(TARGET TinyCitySim POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/shaders"
        "$<TARGET_FILE_DIR:TinyCitySim>/shaders"
    COMMENT "Copying shaders to output directory"
)
