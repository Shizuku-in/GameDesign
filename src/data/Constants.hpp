#pragma once

#include <SFML/System/Vector2.hpp>

namespace Config {

// --- 世界 ---
constexpr float WORLD_WIDTH = 3840.f;
constexpr float WORLD_HEIGHT = 2160.f;

// --- 玩家 ---
constexpr float PLAYER_RADIUS = 16.f;
constexpr float PLAYER_SPEED = 220.f;
constexpr float PLAYER_MAX_HP = 100.f;
constexpr float PLAYER_ARMOR = 0.f;    // 伤害减免 0–1
constexpr float PLAYER_MAGNET = 80.f;  // 宝石拾取范围
constexpr float PLAYER_IFRAMES = 0.5f; // 受击后无敌时间（秒）
constexpr int PLAYER_MAX_WEAPONS = 6;

// --- 敌人 ---
constexpr float ENEMY_SPAWN_DISTANCE = 1200.f; // 生成时距玩家的最小距离
constexpr float ENEMY_BASE_SPAWN_INTERVAL = 3.f;
constexpr float ENEMY_MIN_SPAWN_INTERVAL = 0.5f;
constexpr int ENEMIES_PER_WAVE_BASE = 3;

// --- 经验 ---
constexpr float XP_BASE_THRESHOLD = 10.f;
constexpr float XP_THRESHOLD_GROWTH = 5.f;  // 每级增加值
constexpr float XP_GEM_MAGNET_DELAY = 1.5f; // 磁铁吸引前的等待时间
constexpr float XP_GEM_MAGNET_SPEED = 400.f;

// --- 相机 / 视口 ---
constexpr float VIEW_WIDTH = 1920.f;
constexpr float VIEW_HEIGHT = 1080.f;

// --- 敌人类型属性表 ---
// 按 EnemyType 枚举顺序: Basic, Fast, Tank, Boss
constexpr float ENEMY_HP[] = {20.f, 10.f, 80.f, 300.f};
constexpr float ENEMY_SPEED[] = {80.f, 160.f, 50.f, 60.f};
constexpr float ENEMY_DAMAGE[] = {10.f, 8.f, 20.f, 30.f};
constexpr float ENEMY_RADIUS[] = {14.f, 10.f, 22.f, 32.f};
constexpr float ENEMY_XP[] = {1.f, 2.f, 5.f, 50.f};
constexpr float ENEMY_SPAWN_WEIGHT[] = {1.f, 0.7f, 0.4f, 0.f}; // Boss 单独生成

// --- 敌人类型出现时间（秒）---
constexpr float ENEMY_APPEAR_TIME[] = {0.f, 30.f, 60.f, -1.f}; // -1 = 不按权重生成

} // namespace Config
