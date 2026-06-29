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
find_package(SFML 3.1 COMPONENTS System Window Graphics Audio REQUIRED)

# --- Source files ---
add_executable(game
    src/main.cpp
    src/core/Game.cpp
    src/core/Scene.cpp
    src/systems/WeaponDefs.cpp
    src/systems/WeaponSystem.cpp
    src/systems/UpgradeDefs.cpp
    src/systems/HUD.cpp
    src/systems/CollisionSystem.cpp
    src/systems/SpawningSystem.cpp
    src/systems/SoundPlayer.cpp
    src/systems/WorldRenderer.cpp
    src/scenes/PlayScene.cpp
    src/scenes/TitleScene.cpp
    src/scenes/GameOverScene.cpp
    src/scenes/UpgradeUI.cpp
    src/scenes/PauseMenu.cpp
)

# --- Include path for header-only resources ---
target_include_directories(game PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# --- Link SFML (Graphics transitively links Window + System + platform deps) ---
target_link_libraries(game PRIVATE SFML::Graphics SFML::Audio)

# --- Copy assets next to the binary on every build ---
add_custom_command(TARGET game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets" "$<TARGET_FILE_DIR:game>/assets"
    COMMENT "Copying assets to build directory..."
)

# --- Compiler warnings ---
if(MSVC)
    target_compile_options(game PRIVATE /utf-8 /W4)
else()
    target_compile_options(game PRIVATE -Wall -Wextra -Wpedantic)
endif()

# --- Formatting (clang-format) ---
# Search common locations across platforms
if(APPLE)
    execute_process(COMMAND xcrun --find clang-format
                    OUTPUT_VARIABLE XCRUN_CF
                    OUTPUT_STRIP_TRAILING_WHITESPACE
                    ERROR_QUIET)
    if(XCRUN_CF)
        get_filename_component(_clang_format_dir ${XCRUN_CF} DIRECTORY)
        list(APPEND _clang_format_hints ${_clang_format_dir})
    endif()
endif()
if(WIN32)
    # LLVM default install path on Windows
    file(GLOB _llvm_dirs "C:/Program Files/LLVM/bin" "C:/Program Files (x86)/LLVM/bin")
    list(APPEND _clang_format_hints ${_llvm_dirs})
endif()

find_program(CLANG_FORMAT
    NAMES clang-format clang-format-20 clang-format-19 clang-format-18 clang-format-17
    HINTS ${_clang_format_hints}
    DOC "Path to clang-format"
)

if(NOT CLANG_FORMAT)
    message(WARNING "clang-format not found — format/format-check targets disabled. "
                    "Install it via your package manager (apt/brew/LLVM).")
endif()

if(CLANG_FORMAT)
    file(GLOB_RECURSE FORMAT_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/**/*.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/**/*.cpp
    )
    add_custom_target(format
        COMMAND ${CLANG_FORMAT} -i ${FORMAT_SOURCES}
        COMMENT "Formatting source files with clang-format..."
    )
    add_custom_target(format-check
        COMMAND ${CLANG_FORMAT} --dry-run -Werror ${FORMAT_SOURCES}
        COMMENT "Checking source formatting with clang-format..."
    )
endif()
