# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

A Vampire Survivors-style "bullet heaven" 2D game. WASD move, weapons auto-fire, kill enemies → XP gems → level up → pick an upgrade, survive as long as possible. SFML 3.1, C++20, CMake.

Fixed render/world resolution: 1920×1080 (`VIEW_WIDTH`/`VIEW_HEIGHT`). World size is per-map, read at runtime from the TMX file.

## Build, Run, Format

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release   # configure
cmake --build build                          # build (copies assets/ next to the binary via POST_BUILD)
./build/game                                 # run — must run from a dir containing assets/, i.e. build/

cmake --build build --target format          # clang-format in place
cmake --build build --target format-check    # dry-run -Werror (CI-friendly)
```

Windows (MSVC): `cmake -B build` then `cmake --build build --config Release`, run `.\build\Release\game.exe`.

There is no test suite. Verification is by building (warnings are `-Wall -Wextra -Wpedantic`, treated seriously) and running.

Asset paths are relative to the working directory, so the binary only finds assets when launched from the build dir (where POST_BUILD copies them).

### Formatting is enforced

`.clang-format` is LLVM-based, 4-space, 100-column, `PointerAlignment: Left`. Run the `format` target before committing — `scripts/pre-commit` (install via `git config core.hooksPath scripts`) rejects unformatted staged `src/*.hpp`/`*.cpp`.

### Dependencies

SFML 3.1 (system, statically linked) and tmxlite (bundled, built from `third_party/tmxlite/`). `third_party/` is gitignored and must be cloned manually — see README. SFML 3 renamed/restructured much of the SFML 2 API; match existing usage rather than older SFML 2 examples.

## Architecture

### Data-driven definition tables (the core pattern)

Every kind of game content is defined the same way: an `enum class … : std::uint8_t` (ending in `Count`) → a `struct …Def` → a `static_assert`-sized `constexpr` array of rows using C++20 designated initializers. Adding content means adding an enum value and one table row — no logic changes.

| Table | Enum | File |
|-------|------|------|
| Weapons | `WeaponType` | `gameplay/WeaponDefs.*` |
| Enemies | `EnemyType` | `gameplay/EnemyDefs.*` |
| Characters | `CharacterType` | `gameplay/CharacterDefs.*` |
| Maps | `MapType` | `gameplay/MapDefs.*` |
| Upgrades | built dynamically | `gameplay/UpgradeDefs.*` |

`data/Constants.hpp` (`namespace Config`) holds all universal, non-per-entity tuning: timestep, view size, XP curve, i-frames, pool capacities, sound configs, asset paths, colors. Per-entity numbers live in the tables, not here.

### Game loop — fixed timestep accumulator

`Game::run()` (`core/Game.cpp`) is the Glenn Fiedler "Fix Your Timestep" pattern: world updates at a fixed `1/60 s`, render is uncapped, frame time is clamped (`≤ 1/15 s`, i.e. ≤4 updates/frame) to prevent the spiral of death. Scene swaps are **deferred**: `Game::changeScene()` stores into `m_pendingScene`, and the swap happens at the end of the outer loop — so it is safe to request a scene change from inside `update()`/`handleEvent()`.

### Scenes

`Scene` (`core/Scene.*`) is the abstract base: `handleEvent` / `update` / `render`. Subclasses in `scenes/`: `TitleScene` → `PlayScene` → `GameOverScene`. `PlayScene` owns and orchestrates all gameplay systems and the entity pools.

### Object pool — `Pool<T>` (`core/Pool.hpp`)

Contiguous freelist with generation-counted handles (`Handle{idx, gen}`; `gen == 0` means free). `acquire()`, `release(Handle)` (validates generation), `get(Handle)`, `forEach(fn)`, and `forEachHandle(fn)` — the latter passes the handle so you can `release()` the current element safely mid-iteration. All entity collections (enemies, projectiles, XP gems, damage texts) are pools, pre-reserved to the `POOL_*_CAPACITY` constants.

### `PlayScene::update()` order (per fixed tick)

input → move player (clamp to world, tick i-frames) → player animation → `WeaponSystem` → enemies (move toward player, animate, cull far ones) → projectiles → XP gems (magnet) → damage texts → `CollisionSystem::processCollisions()` → `SpawningSystem` → camera → death check → level-up check. Order matters: collision runs after all motion; spawning after collision.

### Weapon system — strategy pattern

`WeaponSystem` (`systems/`) manages up to 6 slots. Each weapon implements `IWeaponBehavior` (`fire()` for spawning projectiles, `tickAoE()` for aura damage). `WeaponDef` rows carry the level-1 stats plus a factory function pointer that constructs the behavior.

| Weapon | Motion | Behavior |
|--------|--------|----------|
| MagicWand | Linear | homing bolt at nearest enemy in range |
| Knife | Linear | fan of piercing projectiles |
| Axe | Orbit | projectiles orbiting the player |
| Fireball | Linear | slow, explodes on hit (AoE) |
| Garlic | none (AoE) | tick damage to enemies in range |

Add a weapon: `WeaponType` enum value → `WeaponDef` row (stats + factory) → behavior class in `WeaponBehaviors.*`.

Level scaling (1–8): `cooldown *= 0.95^(N-1)`, `damage *= 1.30^(N-1)`, `pierce += (N-1)/3`, `projectileCount += (N-1)/2`.

### Projectile motion — tagged union

`Projectile` (`data/EntityTypes.hpp`) has `ProjMotion { Linear, Orbit }` plus a union of motion-specific state. Linear moves by velocity; Orbit rotates around the player. Check the tag before touching the union.

### Collision — spatial hashing (`systems/CollisionSystem.*`)

Uniform grid (100px cells), dimensions derived from world size at runtime. Enemies are bucketed by position; each projectile only tests the adjacent cells → ~O(N+M). The grid (`gridCols`/`gridRows`) is internal to `processCollisions()`. Handles projectile↔enemy (with per-projectile i-frame to avoid multi-hit in consecutive frames), player↔enemy, player↔gem, AoE explosion, death loot, and cleanup.

Three subtle invariants live here — preserve them when editing:
- skip enemies with `hitFlashTimer > 0` so one projectile can't hit the same enemy on consecutive frames;
- skip `hp <= 0` enemies in `findNearestEnemy` so weapons don't target corpses (the "鞭尸" bug);
- the `killed` flag ensures enemies killed by AoE (which live outside the collision grid) still drop loot during cleanup.

### Maps & tilemap (`graphics/TilemapRenderer.*`)

`TilemapRenderer` loads a Tiled `.tmx` via tmxlite and bakes it into one `sf::VertexArray` (`Triangles`, 6 verts/tile) for a single draw call; world size (`getWidth`/`getHeight`) is derived from the TMX tile count × tile size. Add a map: `MapType` enum → `MapDef` row (spawn params + TMX/tileset paths) → author the `.tmx` in Tiled.

Constraints baked into the loader — respect them when authoring maps:
- **Only the first non-empty Tile layer renders.** `loadFromFile` returns on the first Tile layer that has tiles; object layers, image layers, and any later tile layers are ignored. The map is a purely visual ground layer — there is **no collision layer** (world bounds are just the rectangle clamp).
- **Single tileset, `firstgid` = 1.** GID → tile index is `gid - 1`, and columns come from one tileset texture (`tilesetWidth / tileWidth`). Multiple tilesets or a non-1 `firstgid` will mismap tiles.
- **The tileset image comes from `MapDef::tilesetPath`, not the TMX.** The `.tmx`'s own embedded tileset reference is not read; the path in the table must point at the matching image. `setSmooth(false)` keeps pixel art crisp — GID `0` is the empty tile.

### Upgrades (`gameplay/UpgradeDefs.*`)

`generateUpgrades()` collects all eligible `UpgradeDef`s (each has function pointers for availability test, description formatting, and application), shuffles, and offers 3 on level-up. Categories: NewWeapon (auto-derived from the weapon table), WeaponUpgrade, StatBoost.

### Rendering order

World view (camera): `TilemapRenderer::draw()` → `WorldRenderer::render()` (projectiles, gems, enemies, player, damage texts). Then default view: `HUD`, and `UpgradeUI`/`PauseMenu` when paused. UI positions are ratios of `VIEW_WIDTH`/`VIEW_HEIGHT`, not absolute pixels.

### Audio (`audio/SoundPlayer.*`)

24-voice round-robin `sf::Sound` pool. Each named SFX has a minimum re-trigger interval (`SoundConfig` in `Constants.hpp`) to stop stacking on rapid fire / multi-pickup. BGM is `sf::Music`, path from the active `MapDef`.

## SFML 3.1 API — key differences from SFML 2

Training data skews heavily toward SFML 2. The patterns below are what this project uses; do not revert to the SFML 2 equivalents.

**Event polling** — returns `std::optional`; use `getIf<T>()` / `is<T>()`:
```cpp
// SFML 3 ✓
while (const std::optional event = m_window.pollEvent()) {
    if (event->is<sf::Event::Closed>()) { ... }
    if (const auto* kp = event->getIf<sf::Event::KeyPressed>()) {
        // kp->code, kp->scancode, kp->alt, kp->shift, ...
    }
}
// SFML 2 ✗ — event.type == sf::Event::KeyPressed, event.key.code
```

**`sf::Text` requires a font at construction** — no default constructor:
```cpp
sf::Text text(font);        // font is sf::Font& ✓
sf::Text text(*fontPtr);    // dereference pointer  ✓
// sf::Text text; text.setFont(font);  ✗ (SFML 2 pattern)
```

**Window creation** — `sf::VideoMode` takes braced size; `sf::State::Windowed` is the 4th arg:
```cpp
sf::RenderWindow m_window{
    sf::VideoMode({DEFAULT_WIDTH, DEFAULT_HEIGHT}),
    TITLE,
    sf::Style::Default,
    sf::State::Windowed   // required in SFML 3 — no equivalent in SFML 2
};
```

**Scoped status enum** — `sf::Sound::Status::Stopped`, not `sf::Sound::Stopped`.

**Everything else used here is unchanged from SFML 2**: `sf::Texture::loadFromFile()`, `sf::SoundBuffer::loadFromFile()`, `sf::Music::openFromFile()`, `sf::Keyboard::isKeyPressed()`, `sf::Color`, `sf::Vector2f`, `sf::View`, `sf::Sprite`, `sf::VertexArray`.

## Conventions

- Comments and commit messages are in Chinese (Conventional Commits prefixes: `feat:`/`fix:`/`refactor:`/`docs:`). Match the surrounding language when editing a file.
- C++20: prefer designated initializers, `std::ssize`, `std::format`, `auto` over narrowing casts — recent refactors moved deliberately in this direction.
- `core/` is engine-generic; `gameplay/` is pure data tables; `systems/` is runtime logic; keep that separation.
