# SFML 3.1 Game

A cross-platform game built with SFML 3.1 and modern CMake.

## Prerequisites

- **CMake** >= 3.16
- **SFML** >= 3.1 (headers + static libraries)
- A C++17 compiler (Clang, GCC, or MSVC)

### Installing SFML 3.1

**macOS (Homebrew):**
```bash
brew install sfml
```

**Linux (apt):**
```bash
sudo apt install libsfml-dev
```

**Windows:**
Download from [sfml-dev.org](https://www.sfml-dev.org/download.php) and set `CMAKE_PREFIX_PATH` to the install location.

## Build & Run

```bash
# Configure
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build

# Run
./build/game
```

On Windows (Visual Studio):
```bash
cmake -B build
cmake --build build --config Release
.\build\Release\game.exe
```

## Project Structure

```
├── CMakeLists.txt          # Build configuration
├── README.md
└── src/
    ├── main.cpp            # Entry point
    ├── Game.hpp/.cpp       # Core game loop (fixed timestep)
    ├── Scene.hpp/.cpp      # Abstract scene interface
    ├── GameScene.hpp/.cpp  # Demo gameplay scene
    └── ResourceManager.hpp # Header-only resource cache
```

## Controls (Demo)

| Key | Action |
|---|---|
| Arrow keys / WASD | Move the circle |
| Escape | Quit |

## License

MIT
