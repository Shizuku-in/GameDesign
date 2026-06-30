# CLAUDE.md

Codebase guidance for Claude Code (claude.ai/code).

## Build & Run

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release   # configure
cmake --build build                          # build
./build/game                                 # run
```

Windows (MSVC):
```bash
cmake -B build
cmake --build build --config Release
.\build\Release\game.exe
```

```bash
cmake --build build --target format        # format in-place
cmake --build build --target format-check  # check only (CI-friendly)
```

Formatter: `.clang-format` (WebKit, 4-space, 100-col).
Run `cmake --build build --target format` before committing.

## Project: Survivor-like (Vampire Survivors clone)

2D top-down "bullet heaven". WASD move, weapons auto-fire, kill → XP gems → level up → choose upgrades, survive as long as possible.

Resolution: 1920×1080. World size: per-map (from TMX file via TilemapRenderer).

## Directory Structure

```
src/
├── main.cpp
├── core/          Engine infrastructure
│   ├── Game.hpp/.cpp          Game loop, scene mgmt, resource loading
│   ├── Scene.hpp/.cpp         Abstract scene base (handleEvent/update/render)
│   ├── Pool.hpp               Freelist object pool with generation-counted handles
│   ├── ResourceManager.hpp    Resource cache template (shared_ptr-based)
│   └── Random.hpp/.cpp        Mersenne Twister singleton
├── data/          Pure data definitions
│   ├── Constants.hpp          Universal tuning values + resource paths + colors
│   ├── EntityTypes.hpp        Enemy, Projectile (with ProjMotion union), XPGem, DamageText
│   └── PlayerState.hpp        Player singleton state struct
├── math/          Math utilities
│   └── Collision.hpp          constexpr circleCircle() + distanceSq()
├── audio/         Audio
│   └── SoundPlayer.hpp/.cpp   sf::Sound pool × 24, named play methods, interval protection
├── graphics/      Renderers
│   ├── SpriteSheet.hpp        Horizontal strip sprite sheet loader
│   ├── TilemapRenderer.hpp/.cpp Tiled TMX loader + VertexArray tile rendering
│   └── WorldRenderer.hpp/.cpp Entity rendering (projectiles, gems, enemies, player, damage texts)
├── gameplay/      Data-driven definition tables
│   ├── WeaponDefs.hpp/.cpp    Weapon definition table + getWeaponStats() + createWeapon()
│   ├── CharacterDefs.hpp/.cpp Character definition table (stats + sprites)
│   ├── EnemyDefs.hpp/.cpp     Enemy definition table
│   ├── MapDefs.hpp/.cpp       Map definition table (spawn params + tile paths)
│   └── UpgradeDefs.hpp/.cpp   Upgrade definition table + generateUpgrades() + applyUpgrade()
├── systems/       Runtime gameplay systems
│   ├── IWeaponBehavior.hpp    Strategy interface (fire / tickAoE)
│   ├── WeaponBehaviors.hpp/.cpp 5 weapon behavior classes
│   ├── WeaponSystem.hpp/.cpp 6-slot weapon manager
│   ├── CollisionSystem.hpp/.cpp Spatial-hash collision + cleanup + death loot
│   └── SpawningSystem.hpp/.cpp Wave spawning, boss timer, per-map difficulty
├── ui/            Screen-space UI
│   ├── HUD.hpp/.cpp           HP/XP bars, level, timer, weapon list
│   ├── PauseMenu.hpp/.cpp     Pause overlay (Resume / Quit)
│   └── UpgradeUI.hpp/.cpp     Level-up choice overlay
└── scenes/        Scene subclasses
    ├── PlayScene.hpp/.cpp      Main game (orchestrates all systems)
    ├── TitleScene.hpp/.cpp     Title screen
    └── GameOverScene.hpp/.cpp  Death screen with stats
```

## Architecture

### Data-Driven Design

All entity definitions use the same pattern: enum → struct → `static_assert`-protected array:

| Table | Enum | File |
|-------|------|------|
| Weapons | `WeaponType` | `WeaponDefs.hpp/.cpp` |
| Enemies | `EnemyType` | `EnemyDefs.hpp/.cpp` |
| Characters | `CharacterType` | `CharacterDefs.hpp/.cpp` |
| Maps | `MapType` | `MapDefs.hpp/.cpp` |
| Upgrades | Built dynamically | `UpgradeDefs.hpp/.cpp` |

Adding a new entry = add enum value + add one row to the table. C++20 designated initializers make rows self-documenting.

### Game Loop — Accumulator-Based Fixed Timestep

`Game::run()` (`src/core/Game.cpp`): Glenn Fiedler "Fix Your Timestep".

- **Fixed update**: 1/60 s (`FIXED_DT` in `Constants.hpp`)
- **Accumulator cap**: 1/15 s — at most 4 updates per render
- **Framerate**: uncapped
- **Deferred scene swap**: at end of each outer-loop iteration

### Deferred Scene Switching

`Game::changeScene()` stores in `m_pendingScene`. Swap at loop end — safe to call from `update()` / `handleEvent()`.

### Object Pool — Pool\<T\>

`Pool<T>` (`src/core/Pool.hpp`): contiguous freelist + generation handles (`{idx, gen}`).

- `acquire()` → Handle (freelist pop or grow)
- `release(Handle)` → validates gen, marks free
- `forEach(fn)` → iterates occupied slots
- `forEachHandle(fn)` → provides Handle, safe to `release()` inside callback
- Generation == 0 means free

### PlayScene::update() Order (60 Hz)

1. handleInput() — WASD polling
2. movePlayer(dt) — apply velocity, clamp world bounds, countdown invincibility
3. updatePlayerAnimation(dt) — direction detection, frame advance
4. WeaponSystem::update() — each slot: cooldown → behavior->fire() or tickAoE()
5. updateEnemies(dt) — AI: move toward player + sprite animation; cull far-away enemies
6. updateProjectiles(dt) — Linear (move by vel) or Orbit (rotate around player)
7. updateXPGems(dt) — magnet range check → accelerate toward player
8. updateDamageTexts(dt) — float upward + fade
9. CollisionSystem::processCollisions() — spatial-hash grid → projectile↔enemy (with i-frame), player↔enemy, player↔gem; AoE explosion; death loot; cleanup
10. SpawningSystem::update() — boss timer, wave timer, weighted random types, per-map difficulty
11. updateCamera() — center view on player, clamp to world
12. Death check → deferred switch to GameOverScene
13. Level-up check → pause, generate 3 upgrade options

### Weapon System

Strategy pattern: `IWeaponBehavior` interface with `fire()` and `tickAoE()`.

| Weapon | Class | Motion | Behavior |
|--------|-------|--------|----------|
| MagicWand | MagicWandBehavior | Linear | Homing bolt at nearest enemy in range |
| Knife | KnifeBehavior | Linear | Fan of piercing projectiles |
| Axe | AxeBehavior | Orbit | Orbiting projectiles around player |
| Fireball | FireballBehavior | Linear | Slow projectile, explodes on hit (AoE) |
| Garlic | GarlicBehavior | None (AoE) | Tick damage to enemies in range |

Adding a new weapon:
1. Add `WeaponType` enum value in `WeaponDefs.hpp`
2. Add `WeaponDef` entry (data + factory lambda) in `WeaponDefs.cpp`
3. Create behavior class in `WeaponBehaviors.hpp/.cpp`

Level scaling (1–8): `cooldown *= 0.95^(N-1)`, `damage *= 1.30^(N-1)`, `pierce += (N-1)/3`, `projectileCount += (N-1)/2`.

### Projectile Motion

Tagged union in `EntityTypes.hpp`:
```cpp
enum class ProjMotion : uint8_t { Linear, Orbit };
union { struct { float angle, radius, speed; } orbit; } state;
```

### Collision System — Spatial Hashing

Dynamic grid (100px cells), sized from world dimensions at runtime. Enemies inserted by position, projectiles check only adjacent cells. O(N+M).

Anti-bug protections:
- `e->hitFlashTimer > 0` skip: prevents same projectile hitting same enemy in consecutive frames
- `e->hp <= 0` skip in `findNearestEnemy`: prevents targeting already-killed enemies
- `e->killed` flag: ensures AoE kills (outside collision grid) still get death loot in cleanup

### Map System

Maps defined in `MapDefs.hpp/.cpp`. World size auto-reads from TMX file via `TilemapRenderer`.

`TilemapRenderer`: loads TMX via tmxlite, builds `sf::VertexArray` once, single draw call.

Adding a new map:
1. Add `MapType` enum value
2. Add `MapDef` entry (spawn params + TMX/tileset paths)
3. Create `.tmx` file in Tiled editor

### Upgrade System

Table-driven: `UpgradeDef` entries with function pointers for availability, description formatting, and application. `generateUpgrades()` iterates all entries, shuffles, picks 3.

Three categories: NewWeapon (auto-generated from WEAPON_DEFS), WeaponUpgrade, StatBoost.

### Rendering Pipeline

```
window.setView(m_camera)    → TilemapRenderer::draw() → WorldRenderer::render() (entities)
window.setView(defaultView) → HUD
if paused                   → UpgradeUI / PauseMenu
```

All UI coordinates use `VIEW_WIDTH`/`VIEW_HEIGHT` ratio-based positioning.

### Sound System

`SoundPlayer` (`src/audio/SoundPlayer.hpp`): 24-sound pool, round-robin, interval protection per sound (prevents stacking on rapid fire/multi-pickup). BGM via `sf::Music`, path from MapDef.

### Resource Manager

`ResourceManager<T>` caches `shared_ptr<T>`. Game owns `ResourceManager<sf::Font>` and `ResourceManager<sf::SoundBuffer>`.

## Adding a New Scene

1. Create `NewScene.hpp/.cpp` in `src/scenes/`
2. Inherit `Scene`, implement `update()` + `render()` (+ optional `handleEvent()`)
3. Add `src/scenes/NewScene.cpp` to CMakeLists.txt
4. Call `m_game.changeScene(std::make_unique<NewScene>(m_game))` to switch

## CMake Configuration

- **C++20**, static SFML linking
- **C language** enabled (needed for tmxlite/miniz)
- **Third-party**: tmxlite (Tiled map parser) built as static library from `third_party/tmxlite/`
- **SFML components**: `System Window Graphics Audio`
- **Warnings**: `-Wall -Wextra -Wpedantic`
- **POST_BUILD**: copies `assets/` next to binary

## SFML 3.1 API Conventions

```cpp
// Events (std::optional)
while (const std::optional event = window.pollEvent()) {
    if (event->is<sf::Event::Closed>()) { }
    if (const auto* kp = event->getIf<sf::Event::KeyPressed>()) { kp->code; }
}

// Window
sf::RenderWindow window(sf::VideoMode({w, h}), "Title", sf::Style::Default, sf::State::Windowed);

// Continuous input (in update(), not handleEvent())
sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right);

// sf::Text — NO default constructor. Must initialize: sf::Text(font, "", size)
// sf::Sound — NO default constructor. Must initialize: sf::Sound(buffer)
// sf::Music — setLooping(true) not setLoop(true)
```

## C++20 Conventions

- **Designated initializers**: all definition tables (`WEAPON_DEFS[]`, `ENEMY_DEFS[]`, `CHARACTER_DEFS[]`, `MAP_DEFS[]`)
- **`std::format`**: all string formatting (no `snprintf`/`char buf[]`)
- **`std::ssize()`**: signed container sizes
- **`std::clamp()`**: bounds clamping
- **`std::span`**: SpawningSystem::setEnemySprites
- **`[[unlikely]]`**: hot-path collision checks
- **`using enum`**: `UpgradeDefs.cpp`, `WeaponBehaviors.cpp`
- **`.contains()`**: `ResourceManager::has()`
- **`constexpr`**: `circleCircle`, `distanceSq`, `getWeaponStats` (with `constpow` helper)

## Design Conventions

- **Data-oriented**: plain structs in pools, no Entity base class
- **Data-driven tables**: all definitions in arrays with `static_assert` size checks
- **Strategy pattern for weapons**: `IWeaponBehavior` interface, factory lambda per entry
- **Tagged union for projectile motion**: `ProjMotion` enum + `union state`
- **Constants file**: universal tuning values only; per-type data lives in definition tables
- **Zero heap allocation in hot paths**: pools pre-allocate, `sf::VertexArray` for batching
- **Include order**: project headers → SFML headers → system headers
