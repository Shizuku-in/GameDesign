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

## Code Formatting

This project uses **clang-format** to enforce a consistent code style. Rules are in `.clang-format` at the repo root.

### Install clang-format

| Platform | Command |
|---|---|
| macOS | `brew install clang-format` |
| Linux | `sudo apt install clang-format` |
| Windows | Download from [LLVM releases](https://github.com/llvm/llvm-project/releases) |

macOS also bundles clang-format with Xcode CLI Tools (no extra install needed).

### Format before committing

```bash
cmake --build build --target format        # format changed files in-place
cmake --build build --target format-check  # check only (same as CI)
```

### Pre-commit hook (recommended)

```bash
git config core.hooksPath scripts
```

This runs the check automatically before every commit and blocks unformatted code.

### Editor integration

Most editors auto-detect `.clang-format` files:

- **VS Code**: install "C/C++" extension → format on save (`"editor.formatOnSave": true`)
- **CLion / IntelliJ**: enabled by default for C++ files
- **Vim / Neovim**: `:!clang-format -i %`
- **Emacs**: `clang-format-buffer` (via `clang-format.el`)

## License

[MIT](LICENSE)
