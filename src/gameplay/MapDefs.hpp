#pragma once

#include <cstdint>

/// 地图类型枚举——按添加顺序排列。
enum class MapType : std::uint8_t { Forest, Count };

/// 地图定义——世界尺寸 + 生成参数 + 资源路径，集中管理。
struct MapDef {
    MapType type;
    const char* name;
    const char* bgmPath;

    // 世界尺寸
    float worldWidth;
    float worldHeight;

    // 生成参数
    float spawnDistance;     // 敌人生成距玩家距离
    float spawnJitter;       // 生成位置随机抖动
    float baseSpawnInterval; // 基础生成间隔（秒）
    float minSpawnInterval;  // 最短生成间隔
    int baseEnemiesPerWave;  // 基础每波敌人数
    float difficultyScale;   // 难度递增系数（间隔缩减/秒）
    float waveInterval;      // 每波敌人数增加间隔（秒）
    int maxEnemies;          // 敌人数上限（保护帧率）
    int maxEnemiesPerWave;   // 每波上限（防止后期卡顿）
    float bossInterval;      // Boss 生成间隔（秒）
};

/// 所有地图定义表，按 MapType 枚举顺序排列。
extern const MapDef MAP_DEFS[];
