# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

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
Use `cmake --build build --target format` before committing.

## Project: Survivor-like (Vampire Survivors clone)

2D top-down "bullet heaven". WASD move, weapons auto-fire, kill → XP gems → level up → choose upgrades, survive as long as possible.

Design doc: `docs/design-doc.md`. Resolution: 1920×1080, world 3840×2160.

## Directory Structure

```
src/
├── main.cpp
├── core/          Engine infrastructure
│   ├── Game.hpp/.cpp          Game loop, scene mgmt, resource loading
│   ├── Scene.hpp/.cpp         Abstract scene base (handleEvent/update/render)
│   ├── Pool.hpp               Freelist object pool with generation-counted handles
│   ├── ResourceManager.hpp    Resource cache template (shared_ptr-based)
│   └── Random.hpp/.cpp        Mersenne Twister singleton (replaces std::rand)
├── data/          Pure data definitions (header-only)
│   ├── Constants.hpp          All tuning values + resource paths + colors
│   ├── EntityTypes.hpp        Enemy, Projectile (with ProjMotion union), XPGem
│   └── PlayerState.hpp        Player singleton state struct
├── math/          Math utilities
│   └── Collision.hpp          circleCircle() + distanceSq() inline functions
├── audio/         Audio
│   └── SoundPlayer.hpp/.cpp   sf::Sound pool × 24, named play methods, volume+interval in Constants
├── graphics/      World-space rendering
│   └── WorldRenderer.hpp/.cpp Grid background + entities + player drawing
├── gameplay/      Game rules & data tables
│   ├── WeaponDefs.hpp/.cpp    Weapon definition table + getWeaponStats() scaling formula
│   ├── UpgradeDefs.hpp/.cpp   Random upgrade generation + application
│   └── WeaponFactory.hpp      Factory: WeaponType → IWeaponBehavior
├── systems/       Runtime gameplay systems
│   ├── IWeaponBehavior.hpp    Strategy interface (fire / tickAoE)
│   ├── WeaponBehaviors.hpp/.cpp 5 weapon behavior classes
│   ├── WeaponSystem.hpp/.cpp 6-slot weapon manager, delegates to behaviors
│   ├── CollisionSystem.hpp/.cpp Spatial-hash collision detection + cleanup
│   └── SpawningSystem.hpp/.cpp Wave spawning, boss timer, difficulty scaling
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

### Game Loop — Accumulator-Based Fixed Timestep

`Game::run()` (`src/core/Game.cpp`): Glenn Fiedler "Fix Your Timestep".

- **Fixed update**: 1/60 s (`TIME_PER_FRAME`), constant `FIXED_DT` available in `Constants.hpp`
- **Accumulator cap**: 1/15 s — at most 4 updates per render
- **Framerate**: uncapped, self-managed timing
- **Deferred scene swap**: at end of each outer-loop iteration

### Deferred Scene Switching

`Game::changeScene()` stores in `m_pendingScene`. Actual swap at loop end when no scene code is on the stack — safe to call from within `update()` / `handleEvent()`.

### Object Pool — Pool\<T\>

`Pool<T>` (`src/core/Pool.hpp`): contiguous freelist + generation handles (`{idx, gen}`).

- `acquire()` → Handle (freelist pop or grow)
- `release(Handle)` → validates gen, marks free
- `forEach(fn)` → iterates occupied slots
- `forEachHandle(fn)` → provides Handle, safe to `release()` inside callback
- No `alive` flags — generation == 0 means free

### Random — Random

`Random` (`src/core/Random.hpp`): static `std::mt19937` singleton.

- `Random::init()` — seed from `random_device`
- `Random::getFloat()` / `getFloat(min,max)` / `getInt(min,max)`
- `Random::getEngine()` — for `std::shuffle`

### PlayScene::update() Order (60 Hz)

1. handleInput() — WASD polling
2. movePlayer(dt) — apply velocity, clamp world bounds, countdown invincibility
3. WeaponSystem::update() — each slot: cooldown → behavior->fire() or tickAoE()
4. updateEnemies(dt) — AI: move toward player; cull far-away enemies
5. updateProjectiles(dt) — Linear (move by vel) or Orbit (rotate around player)
6. updateXPGems(dt) — magnet timer → accelerate toward player
7. CollisionSystem::processCollisions() — spatial-hash grid → projectile↔enemy, player↔enemy, player↔gem; cleanup dead entities
8. SpawningSystem::update() — boss every 60s, wave timer, weighted random types, difficulty ramp
9. updateCamera() — center view on player, clamp to world
10. Death check → deferred switch to GameOverScene
11. Level-up check → pause, generate 3 options

### Weapon System

Strategy pattern: `IWeaponBehavior` interface + per-weapon classes.

| Weapon | Class | Behavior |
|--------|-------|----------|
| MagicWand | MagicWandBehavior | Homing bolt at nearest enemy in range |
| Knife | KnifeBehavior | Fan of piercing knives toward nearest enemy |
| Axe | AxeBehavior | Orbiting projectiles (ProjMotion::Orbit) |
| Fireball | FireballBehavior | Slow projectile, explodes on first hit |
| Garlic | GarlicBehavior | AoE tick damage (no projectiles) |

Adding a new weapon:
1. Add `WeaponType` enum value in `gameplay/WeaponDefs.hpp`
2. Add `WeaponDef` entry in `gameplay/WeaponDefs.cpp`
3. Create behavior class in `systems/WeaponBehaviors.hpp/.cpp`
4. Register in `gameplay/WeaponFactory.hpp` factory method

Level scaling (1–8): `cooldown *= 0.95^(N-1)`, `damage *= 1.30^(N-1)`, `pierce += (N-1)/3`, `projectileCount += (N-1)/2`.

### Projectile Motion

`ProjMotion` enum + union in `EntityTypes.hpp`:

```cpp
enum class ProjMotion : uint8_t { Linear, Orbit };
// Projectile has:
ProjMotion motion;
union { struct { float angle, radius, speed; } orbit; } state;
```

Update logic in `PlayScene::updateProjectiles()` switches on `motion`.

### Collision System — Spatial Hashing

`CollisionSystem::processCollisions()` builds a uniform grid (100px cells), inserts enemies, then checks only adjacent cells. O(N+M) instead of O(N×M).

### Upgrade System

Level-up pauses game, generates 3 random choices:
- **NewWeapon** — if slots < 6 and not owned
- **WeaponUpgrade** — if owned and level < max (shows stat diff)
- **StatBoost** — Vitality, Swiftness, Armor, Magnet, Greed

### Rendering Pipeline

```
window.setView(m_camera)     → WorldRenderer: grid + enemies + projectiles + gems + player
window.setView(defaultView)  → HUD: HP/XP bars, level, timer, weapon list
if paused                    → UpgradeUI::draw() / PauseMenu::draw()
```

All UI coordinates use `VIEW_WIDTH`/`VIEW_HEIGHT` ratio-based positioning. Change only `Constants.hpp` to switch resolutions.

### Sound System

`SoundPlayer` (`src/audio/SoundPlayer.hpp`): 24-sound pool, round-robin. Config in `Constants.hpp`:

```cpp
SoundConfig { volume, interval }  // interval = min time between same sound (prevents stacking)
```

Named methods: `shoot()`, `hit()`, `kill()`, `hurt()`, `pickup()`, `levelup()`.

BGM uses `sf::Music` directly in PlayScene constructor.

### Resource Manager

`ResourceManager<T>` caches `shared_ptr<T>`. Game owns `ResourceManager<sf::Font>` and `ResourceManager<sf::SoundBuffer>`. Access via `Game::getFonts()` / `Game::getSounds()`.

## Adding a New Scene

1. Create `NewScene.hpp/.cpp` in `src/scenes/`
2. Inherit `Scene`, implement `update()` + `render()` (+ optional `handleEvent()`)
3. Add `src/scenes/NewScene.cpp` to CMakeLists.txt
4. Call `m_game.changeScene(std::make_unique<NewScene>(m_game))` to switch

## CMake Configuration

- **C++17**, static SFML linking
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

## Design Conventions

- **Data-oriented**: plain structs in pools, no Entity base class
- **Strategy pattern for weapons**: `IWeaponBehavior` interface, factory-created per slot
- **Tagged union for projectile motion**: `ProjMotion` enum + `union state`
- **Constants in one file**: all tuning, paths, colors in `data/Constants.hpp`
- **Zero heap allocation in hot paths**: pools pre-allocate
- **Include order**: project headers → SFML headers → system headers
