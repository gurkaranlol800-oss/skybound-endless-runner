cmake_minimum_required(VERSION 3.20)
project(GameEngine CXX)

# Statically link the MSVC runtime so release builds run on machines
# without the VC++ Redistributable installed.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# WIN32 marks this as a windowed (non-console) application.
add_executable(GameEngine WIN32
    src/main.cpp
    src/Window.cpp
    src/Renderer.cpp
    src/Shader.cpp
    src/Texture.cpp
    src/Sprite.cpp
    src/Input.cpp
    src/Animation.cpp
    src/AnimationController.cpp
    src/Tilemap.cpp
    src/Player.cpp
    src/Camera.cpp
    src/PhysicsBody.cpp
    src/Enemy.cpp
    src/Pickup.cpp
    src/EndlessGenerator.cpp
    src/DigitFont.cpp
)

target_include_directories(GameEngine PRIVATE src third_party)

# stb_image is a single-header library; STB_IMAGE_IMPLEMENTATION is defined
# once in Texture.cpp to generate its implementation.
target_compile_definitions(GameEngine PRIVATE _CRT_SECURE_NO_WARNINGS NOMINMAX)

# Direct3D 11 / DXGI / shader compiler libraries, all part of the Windows SDK.
target_link_libraries(GameEngine PRIVATE d3d11 dxgi d3dcompiler)

# Copy shaders and assets next to the built executable so the game can find
# them at runtime using relative paths.
add_custom_command(TARGET GameEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:GameEngine>/shaders
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:GameEngine>/assets
)
