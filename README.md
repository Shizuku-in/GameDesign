# Survivor-like

A Vampire Survivors-style "bullet heaven" 2D game built with SFML 3.1 and modern CMake (C++20).

Move with WASD, weapons auto-fire at the nearest enemies, collect XP gems, level up, choose upgrades — survive as long as you can.

## Prerequisites

- **CMake** >= 3.16
- **SFML** >= 3.1 (headers + static libraries)
- A C++20 compiler (GCC 13+, Clang 17+, MSVC 2022+)
- **tmxlite** — bundled in `third_party/`, clone with:
  ```bash
  mkdir -p third_party && cd third_party
  git clone https://github.com/fallahn/tmxlite.git
  ```

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

### Map Editor (optional)

Maps are created with [Tiled](https://www.mapeditor.org/):
```bash
sudo apt install tiled   # Linux
brew install tiled       # macOS
```

## Build & Run

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/game
```

On Windows (Visual Studio):
```bash
cmake -B build
cmake --build build --config Release
.\build\Release\game.exe
```

Assets are automatically copied to the build directory via CMake `POST_BUILD`.

## Controls

| Key | Action |
|-----|--------|
| WASD | Move |
| Arrow keys / 1–3 | Select upgrade (level-up screen) |
| Enter / Space | Confirm upgrade / Start game |
| Escape | Pause / Quit to title |

## Project Structure

```
├── CMakeLists.txt
├── README.md
├── README-zh.md
├── CLAUDE.md                 # Architecture & coding conventions
├── .clang-format             # Code style rules (LLVM, C++20)
├── .clang-tidy               # Static analysis checks
├── assets/
│   ├── fonts/                # UI font
│   ├── sounds/
│   │   ├── bgm/              # Music (.ogg)
│   │   └── sfx/              # Sound effects (.wav)
│   ├── sprites/
│   │   ├── character/elf/    # Player sprites (idle/movement/portrait/skill)
│   │   └── enemies/          # Enemy sprite sheets
│   ├── tilesets/             # Tile images for maps
│   └── maps/                 # Tiled .tmx files
├── third_party/
│   └── tmxlite/              # Tiled map parser (bundled)
└── src/
    ├── main.cpp
    ├── core/                 # Engine layer
    │   ├── Game.hpp/.cpp         # Game loop, scene management, resource loading
    │   ├── Scene.hpp/.cpp        # Abstract scene base class
    │   ├── Pool.hpp              # Generic object pool (freelist + generation handles)
    │   ├── ResourceManager.hpp   # Resource cache template
    │   └── Random.hpp/.cpp       # Mersenne Twister singleton
    ├── data/                 # Pure data definitions
    │   ├── Constants.hpp         # Universal tuning values + colors
    │   ├── EntityTypes.hpp       # Enemy, Projectile, XPGem, DamageText structs
    │   └── PlayerState.hpp       # Player state struct
    ├── math/                 # Math utilities
    │   └── Collision.hpp         # constexpr circle-circle collision
    ├── audio/                # Audio
    │   └── SoundPlayer.hpp/.cpp  # sf::Sound pool with interval protection
    ├── graphics/             # Renderers
    │   ├── SpriteSheet.hpp       # Sprite sheet loader
    │   ├── TilemapRenderer.hpp/.cpp  # Tiled TMX → VertexArray tile rendering
    │   └── WorldRenderer.hpp/.cpp  # Entity + damage text rendering
    ├── gameplay/             # Data-driven definition tables
    │   ├── WeaponDefs.hpp/.cpp   # Weapon table + level scaling + factory
    │   ├── EnemyDefs.hpp/.cpp    # Enemy table
    │   ├── CharacterDefs.hpp/.cpp   # Character table (stats + sprites)
    │   ├── MapDefs.hpp/.cpp      # Map table (spawn params + tile paths)
    │   └── UpgradeDefs.hpp/.cpp  # Upgrade table + random generation
    ├── systems/              # Runtime gameplay systems
    │   ├── IWeaponBehavior.hpp   # Strategy interface (fire / tickAoE)
    │   ├── WeaponBehaviors.hpp/.cpp  # 5 weapon behavior classes
    │   ├── WeaponSystem.hpp/.cpp # 6-slot weapon manager
    │   ├── CollisionSystem.hpp/.cpp  # Spatial-hash collision + cleanup
    │   └── SpawningSystem.hpp/.cpp   # Wave spawning + difficulty scaling
    ├── ui/                   # Screen-space UI
    │   ├── HUD.hpp/.cpp          # HP/XP bars, level, timer, weapon list
    │   ├── PauseMenu.hpp/.cpp    # Pause overlay
    │   └── UpgradeUI.hpp/.cpp    # Level-up choice overlay
    └── scenes/               # Game scenes
        ├── PlayScene.hpp/.cpp      # Main gameplay loop
        ├── TitleScene.hpp/.cpp     # Title screen
        └── GameOverScene.hpp/.cpp  # Death/score screen
```

## Adding Content (Data-Driven)

All game content follows the same table-driven pattern. Each only needs 2–3 steps:

| Content | Steps |
|---------|-------|
| New weapon | enum → WeaponDef entry → behavior class |
| New enemy | enum → EnemyDef entry |
| New character | enum → CharacterDef entry (stats + sprites) |
| New map | enum → MapDef entry → create .tmx in Tiled |
| New stat boost | one row in UpgradeDef table |

## Code Formatting

```bash
cmake --build build --target format        # format all source files
cmake --build build --target format-check  # check only (CI)
```

Style: `.clang-format` (LLVM, 4-space, 100-column, C++20).

## Lint & Static Analysis

Two complementary tools are configured — run both before committing:

```bash
cmake --build build --target lint       # clang-tidy (C++ best practices, bug patterns)
cmake --build build --target lint-fix   # clang-tidy auto-fix where possible
cmake --build build --target cppcheck   # cppcheck (memory, undefined behavior, performance)
```

Install dependencies (Ubuntu):

```bash
sudo apt install clang-tidy-19 cppcheck
```

- **clang-tidy** checks are defined in `.clang-tidy` — covers `bugprone-*`, `cert-*`, `modernize-*`, `performance-*`, `readability-*` tuned for the project's style (struct members without `m_`, lowercase `0.f`, etc.)
- **cppcheck** runs `--enable=all` over `src/`, ignores `third_party/`
- Both require `cmake -B build` first to generate `compile_commands.json`

## License

[MIT](LICENSE)
