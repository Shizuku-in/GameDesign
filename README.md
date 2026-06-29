# Survivor-like

A Vampire Survivors-style "bullet heaven" 2D game built with SFML 3.1 and modern CMake.

Move with WASD, weapons auto-fire at the nearest enemies, collect XP gems, level up, choose upgrades — survive as long as you can.

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

Assets (`fonts/`) are automatically copied to the build directory via CMake POST_BUILD.

## Controls

| Key | Action |
|-----|--------|
| WASD / Arrow keys | Move |
| Arrow keys / 1–3 | Select upgrade (level-up screen) |
| Enter / Space | Confirm upgrade / Start game |
| Escape | Quit |

## Project Structure

```
├── CMakeLists.txt
├── README.md
├── CLAUDE.md                # Architecture & coding conventions
├── .clang-format            # Code style rules (WebKit-based)
├── assets/
│   └── fonts/
│       └── DejaVuSans.ttf   # UI font
├── docs/
│   ├── design-doc.md        # Full game design document
│   └── wsl-setup-guide.md
├── scripts/
│   └── pre-commit           # Git hook for clang-format
└── src/
    ├── main.cpp
    ├── core/                # Engine layer
    │   ├── Game.hpp/.cpp        # Game loop, scene management, font loading
    │   ├── Scene.hpp/.cpp       # Abstract scene base class
    │   ├── Pool.hpp             # Generic object pool (freelist + generation handles)
    │   └── ResourceManager.hpp  # Resource cache template
    ├── data/                # Pure data definitions (header-only)
    │   ├── Constants.hpp        # All tuning values
    │   ├── EntityTypes.hpp      # Enemy, Projectile, XPGem structs
    │   ├── PlayerState.hpp      # Player state struct
    │   └── Collision.hpp        # Circle-circle collision helpers
    ├── systems/             # Gameplay systems
    │   ├── WeaponDefs.hpp/.cpp  # Weapon table + level-scaling
    │   ├── WeaponSystem.hpp/.cpp# Auto-attack + targeting + projectile spawning
    │   ├── UpgradeDefs.hpp/.cpp # Random upgrade generation & application
    │   └── HUD.hpp/.cpp         # HP/XP bars, level, timer, weapon list
    └── scenes/              # Game scenes
        ├── PlayScene.hpp/.cpp   # Main gameplay loop
        ├── TitleScene.hpp/.cpp  # Title screen
        └── GameOverScene.hpp/.cpp # Death/score screen
```

## Code Formatting

This project uses **clang-format** to enforce a consistent code style. Rules are in `.clang-format` at the repo root.

### Install clang-format

| Platform | Command |
|---|---|
| macOS | `brew install clang-format` |
| Linux | `sudo apt install clang-format` |
| Windows | Download from [LLVM releases](https://github.com/llvm/llvm-project/releases) |

### Format before committing

```bash
cmake --build build --target format        # format all source files in-place
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
