#include "gameplay/MapDefs.hpp"

const MapDef MAP_DEFS[] = {
    {.type = MapType::Forest,
     .name = "Forest",
     .bgmPath = "assets/sounds/bgm/stone fortress.ogg",
     .worldWidth = 3840.f,
     .worldHeight = 2160.f,
     .spawnDistance = 1200.f,
     .spawnJitter = 200.f,
     .baseSpawnInterval = 3.f,
     .minSpawnInterval = 0.5f,
     .baseEnemiesPerWave = 3,
     .difficultyScale = 0.005f,
     .waveInterval = 10.f,
     .maxEnemies = 500,
     .maxEnemiesPerWave = 30,
     .bossInterval = 60.f},
};

static_assert(sizeof(MAP_DEFS) / sizeof(MAP_DEFS[0]) == static_cast<int>(MapType::Count),
              "MAP_DEFS must have one entry per MapType");
