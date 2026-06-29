# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build & Run

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release   # configure
cmake --build build                          # build
./build/game                                 # run
```

On Windows (MSVC):
```bash
cmake -B build
cmake --build build --config Release
.\build\Release\game.exe
```

There are no tests or linters yet.

## Architecture

### Game Loop — Accumulator-Based Fixed Timestep

`Game::run()` (`src/Game.cpp`) drives the loop. It uses Glenn Fiedler's "Fix Your Timestep" pattern:

- **Fixed update step**: 1/60 s (`TIME_PER_FRAME`)
- **Accumulator cap**: 1/15 s (`TIME_PER_FRAME_MAX`) — prevents spiral of death; at most 4 updates per render
- **Render**: once per outer-loop iteration, no interpolation currently
- **Framerate**: uncapped (`setFramerateLimit(0)`), timing is self-managed

Each iteration: process events → accumulate frame time → drain accumulator with fixed-step updates → render once.

### Scene System

`Scene` (`src/Scene.hpp`) is an abstract base with three virtual methods:

- `handleEvent(const sf::Event&)` — default no-op; override for input
- `update(sf::Time dt)` — called at fixed 60 Hz
- `render(sf::RenderWindow&)` — draw; the caller (`Game::render()`) handles `display()`

`Game` owns the current scene via `std::unique_ptr<Scene>` and delegates to it. Scenes receive a `Game&` reference for window access and scene switching via `changeScene()`.

`GameScene` (`src/GameScene.cpp`) is the current demo implementation. When adding new game states (menus, pause, levels), subclass `Scene`.

### Resource Manager

`ResourceManager<T>` (`src/ResourceManager.hpp`) is a header-only template that caches `std::shared_ptr<T>` in an `std::unordered_map`. Key behavior:

- `load(key, args...)` — constructs `T(args...)` on first access, caches, returns the shared_ptr. Variadic args forward to the resource constructor (e.g., `sf::Texture`'s filename constructor).
- `get(key)` — throws `std::runtime_error` if not found
- `has(key)`, `remove(key)`, `clear()` — standard map operations

### CMake Configuration

- **C++17** required (`CMAKE_CXX_STANDARD 17`) — SFML 3 headers mandate it
- **Static linking**: `SFML_STATIC_LIBRARIES ON` must be set before `find_package`
- **SFML components**: `System Window Graphics` — SFML::Graphics transitively links Window + System + platform deps (freetype, harfbuzz, OpenGL, etc.)
- **macOS**: `CMAKE_FIND_FRAMEWORK LAST` prevents CMake from preferring frameworks over static `.a` libs
- **Warnings**: `-Wall -Wextra -Wpedantic` on Clang/GCC, `/W4` on MSVC

## SFML 3.1 API Conventions

This project uses SFML 3.1 — the API differs from SFML 2.x. When adding code, follow these patterns:

**Event handling:**
```cpp
while (const std::optional event = window.pollEvent()) {
    if (event->is<sf::Event::Closed>()) { /* ... */ }
    if (const auto* kp = event->getIf<sf::Event::KeyPressed>()) {
        kp->code == sf::Keyboard::Key::Escape;
    }
    if (const auto* resized = event->getIf<sf::Event::Resized>()) { /* ... */ }
}
```

**Window creation:**
```cpp
sf::RenderWindow window(sf::VideoMode({w, h}), "Title", sf::Style::Default, sf::State::Windowed);
```

**Input polling** (for continuous movement, do this in `update()`, not in `handleEvent()`):
```cpp
sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right);
```

**Colors** — static members exist: `sf::Color::Red`, `sf::Color::Cyan`, `sf::Color::Black`, `sf::Color::Transparent`, etc.

**Enums** are scoped: `sf::Keyboard::Key::Escape`, `sf::Style::Default`, `sf::State::Windowed`, `sf::Event::KeyPressed`.

**Window size**: `window.getSize()` returns `sf::Vector2u`. RenderTarget also has `setView(sf::View(...))` for resize handling.

## Adding a New Scene

1. Create `NewScene.hpp` / `NewScene.cpp` in `src/`
2. Inherit from `Scene`, implement `update()` and `render()` (and optionally `handleEvent()`)
3. Add `src/NewScene.cpp` to the `add_executable` list in `CMakeLists.txt`
4. Call `m_game.changeScene(std::make_unique<NewScene>(m_game))` to switch to it
