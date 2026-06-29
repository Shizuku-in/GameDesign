cmake_minimum_required(VERSION 3.16)
project(game VERSION 0.1.0 LANGUAGES CXX)

# --- C++ standard (SFML 3 requires >= C++17) ---
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --- Static linking with SFML ---
set(SFML_STATIC_LIBRARIES ON)

# --- macOS: prevent framework search from overriding static libs ---
if(APPLE)
    set(CMAKE_FIND_FRAMEWORK LAST)
endif()

# --- Locate SFML 3.1 ---
find_package(SFML 3.1 COMPONENTS System Window Graphics REQUIRED)

# --- Source files ---
add_executable(game
    src/main.cpp
    src/Game.cpp
    src/GameScene.cpp
    src/Scene.cpp
)

# --- Include path for header-only resources ---
target_include_directories(game PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# --- Link SFML (Graphics transitively links Window + System + platform deps) ---
target_link_libraries(game PRIVATE SFML::Graphics)

# --- Compiler warnings ---
if(MSVC)
    target_compile_options(game PRIVATE /W4)
else()
    target_compile_options(game PRIVATE -Wall -Wextra -Wpedantic)
endif()
