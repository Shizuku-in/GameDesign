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

```bash
cmake --build build --target format        # format in-place (all src/**/*.hpp src/**/*.cpp)
cmake --build build --target format-check  # check only (CI-friendly)
```

Formatter rules are in `.clang-format` (based on WebKit style, 4-space indent, 100-col limit).
Include order: project headers → SFML headers → system headers.
Use `cmake --build build --target format` before committing — or enable the pre-commit hook: `git config core.hooksPath scripts`.

## Project: Survivor-like (Vampire Survivors clone)

2D top-down "bullet heaven" game. Player moves with WASD, weapons auto-attack nearest enemies, kill enemies → collect XP gems → level up → choose upgrades, survive as long as possible.

Design doc: `docs/design-doc.md`.

## Directory Structure

```
src/
├── main.cpp
├── core/                       # Engine layer
│   ├── Game.hpp / Game.cpp     # Game loop, scene management, resource loading
│   ├── Scene.hpp / Scene.cpp   # Abstract scene base class
│   ├── Pool.hpp                # Generic freelist object pool with generation-counted handles
│   └── ResourceManager.hpp     # Resource cache template (shared_ptr-based)
├── data/                       # Pure data definitions (all header-only)
│   ├── Constants.hpp           # All tuning constants in Config:: namespace
│   ├── EntityTypes.hpp         # Enemy, Projectile, XPGem plain structs
│   ├── PlayerState.hpp         # Player singleton state struct
│   └── Collision.hpp           # circleCircle() and distanceSq() inline functions
├── systems/                    # Gameplay systems
│   ├── WeaponDefs.hpp / .cpp   # Weapon definition table + level-scaling formula
│   ├── WeaponSystem.hpp / .cpp # Weapon slot management + auto-attack + targeting
│   ├── UpgradeDefs.hpp / .cpp  # Random upgrade generation + application
│   └── HUD.hpp / .cpp          # HP/XP bars, level, timer, weapon list rendering
└── scenes/                     # Game scenes
    ├── PlayScene.hpp / .cpp    # Main gameplay scene (game loop integration)
    ├── TitleScene.hpp / .cpp   # Title screen
    └── GameOverScene.hpp / .cpp # Death screen with stats
```

## Architecture

### Game Loop — Accumulator-Based Fixed Timestep

`Game::run()` (`src/core/Game.cpp`) uses Glenn Fiedler's "Fix Your Timestep" pattern:

- **Fixed update step**: 1/60 s (`TIME_PER_FRAME`)
- **Accumulator cap**: 1/15 s (`TIME_PER_FRAME_MAX`) — prevents spiral of death; at most 4 updates per render
- **Render**: once per outer-loop iteration, no interpolation
- **Framerate**: uncapped (`setFramerateLimit(0)`), timing is self-managed

Each iteration: process events → accumulate frame time → drain accumulator with fixed-step updates → render once → deferred scene swap.

### Deferred Scene Switching

`Game::changeScene()` stores the new scene in `m_pendingScene` rather than swapping immediately. The actual swap happens at the end of the outer-loop iteration in `run()`, when no scene code is on the call stack. This makes it safe to call `changeScene()` from inside `update()` or `handleEvent()`.

The initial scene (TitleScene) is assigned directly to `m_scene` in the constructor — deferred swap is only for runtime transitions.

### Scene System

`Scene` (`src/core/Scene.hpp`) is an abstract base with three virtual methods:

- `handleEvent(const sf::Event&)` — default no-op; override for input
- `update(sf::Time dt)` — called at fixed 60 Hz
- `render(sf::RenderWindow&)` — draw; the caller (`Game::render()`) handles `display()`

Scenes receive a `Game&` reference for window access, font access, and scene switching. No onEnter/onExit lifecycle hooks.

### Object Pool — Pool\<T\>

`Pool<T>` (`src/core/Pool.hpp`) provides contiguous storage with freelist allocation and generation-counted handles (`{idx, gen}`). Key details:

- `acquire()` → pops from freelist or grows, returns `Handle`
- `release(Handle)` → validates generation, marks slot free
- `get(Handle)` → returns `T*` or `nullptr` if handle is stale
- `forEach(Fn)` → iterates all occupied slots, calls `fn(T&)`
- `forEachHandle(Fn)` → iterates all occupied slots, calls `fn(Handle, T&)` — safe to call `release()` from inside the callback
- No `alive` flags on entities — generation counters on slots serve that purpose
- All entity types (Enemy, Projectile, XPGem) are plain structs stored in pools

### Resource Manager

`ResourceManager<T>` (`src/core/ResourceManager.hpp`) caches `std::shared_ptr<T>` in `std::unordered_map`. Used for `sf::Font` via `Game::getFonts()`. Font is loaded once in `Game` constructor (`"default"` → `assets/fonts/DejaVuSans.ttf`).

### PlayScene::update() Order

Called at 60 Hz, each step in sequence:

1. **handleInput()** — WASD/Arrow key polling via `sf::Keyboard::isKeyPressed()`
2. **movePlayer(dt)** — apply velocity, clamp to world bounds, countdown invincibility
3. **WeaponSystem::update()** — each weapon slot: cooldown → find nearest enemy → fire projectiles / tick AoE
4. **updateEnemies(dt)** — each enemy moves toward player position
5. **updateProjectiles(dt)** — regular: move by velocity; orbiting (orbitRadius>0): rotate around player; both: countdown lifetime
6. **updateXPGems(dt)** — countdown magnet timer → accelerate toward player
7. **checkCollisions()** — projectile↔enemy (damage + pierce), player↔enemy (damage + iframes), player↔gem (collect XP); then cleanup all dead entities via forEachHandle + release
8. **updateSpawning(dt)** — wave timer → spawn weighted-random enemies at view edge; boss every 60s; difficulty ramps over time
9. **updateCamera()** — center `sf::View` on player, clamp to world bounds
10. **Death check** — if HP ≤ 0 → deferred switch to GameOverScene
11. **Level-up check** — if XP ≥ threshold → pause game, generate 3 upgrade options

### Weapon System

5 weapon types, enum-dispatched (no polymorphism):

| Weapon | Behavior |
|--------|----------|
| MagicWand | Homing bolt at nearest enemy in range |
| Knife | Fan of piercing knives toward nearest enemy |
| Axe | Orbiting projectiles around player |
| Fireball | Slow projectile, explodes on first hit |
| Garlic | Persistent AoE damage (no projectiles) |

Weapon stats scale with level (1–8): `cooldown *= 0.95^(N-1)`, `damage *= 1.30^(N-1)`, `pierce += (N-1)/3`, `projectileCount += (N-1)/2`.

### Upgrade System

Level-up pauses the game, presents 3 random choices from three categories:
- **NewWeapon** — if player has < 6 weapons and doesn't own this type
- **WeaponUpgrade** — if player owns this weapon and level < maxLevel
- **StatBoost** — always available (Vitality, Swiftness, Armor, Magnet, Greed)

Player selects with arrow keys + Enter, or number keys 1–3 directly.

### Rendering Pipeline

```
window.setView(m_camera)     → draw world entities (world-space)
  - Enemies: red circles (tinted by type)
  - Projectiles: yellow circles
  - XP gems: green circles
  - Player: white circle (flashes when invincible)
window.setView(defaultView)  → draw HUD (screen-space)
if paused                    → draw upgrade overlay
```

World size: 1600×1200, viewport: 800×600, camera follows player.

## Adding a New Scene

1. Create `NewScene.hpp` / `NewScene.cpp` in `src/scenes/`
2. Inherit from `Scene`, implement `update()` and `render()` (and optionally `handleEvent()`)
3. Add `src/scenes/NewScene.cpp` to the `add_executable` list in `CMakeLists.txt`
4. Call `m_game.changeScene(std::make_unique<NewScene>(m_game))` to switch

## Adding a New Weapon

1. Add enum value to `WeaponType` in `src/systems/WeaponDefs.hpp`
2. Add `WeaponDef` entry to `WEAPON_DEFS[]` in `src/systems/WeaponDefs.cpp`
3. Add `case` to `fireWeapon()` switch in `src/systems/WeaponSystem.cpp`
4. Implement the fire method (e.g., `fireNewWeapon()`)
5. If it's an AoE weapon (no projectiles), set `isAOE = true` in the def and implement a tick method

## CMake Configuration

- **C++17** required (`CMAKE_CXX_STANDARD 17`)
- **Static linking**: `SFML_STATIC_LIBRARIES ON` before `find_package`
- **SFML components**: `System Window Graphics`
- **Warnings**: `-Wall -Wextra -Wpedantic` on Clang/GCC, `/W4` on MSVC
- **POST_BUILD**: copies `assets/` next to the binary automatically

## SFML 3.1 API Conventions

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

**Continuous input** (in `update()`, not `handleEvent()`):
```cpp
sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right);
```

**sf::Text** has NO default constructor in SFML 3.1 — must initialize with `sf::Text(font, "", size)` in member initializer list, or store as `std::unique_ptr<sf::Text>`.

**Colors**: `sf::Color::Red`, `sf::Color::Transparent`, etc.

**Enums** are scoped: `sf::Keyboard::Key::Escape`, `sf::Style::Default`, `sf::State::Windowed`.

## Design Conventions

- **Data-oriented**: entities are plain structs in contiguous pools; no Entity base class, no virtual dispatch
- **Enum dispatch**: weapon behaviors use `switch` on `WeaponType` (fixed set, compiler jump-table optimization)
- **No alive flags**: pool generation counters distinguish occupied/free slots
- **Header-only where possible**: Pool, Constants, EntityTypes, PlayerState, Collision — no .cpp needed
- **Zero heap allocation in hot paths**: pools pre-allocate; no `new`/`delete` during gameplay
- **Include order**: project headers → SFML headers → system headers
