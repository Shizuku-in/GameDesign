#pragma once

#include "data/Constants.hpp"

#include <SFML/System/Vector2.hpp>

/// 玩家状态 — 单例，不存入对象池。
struct PlayerState {
    sf::Vector2f pos{Config::WORLD_WIDTH / 2.f, Config::WORLD_HEIGHT / 2.f};
    sf::Vector2f vel; // 输入方向（已归一化），每帧计算

    float speed = Config::PLAYER_SPEED;
    float hp = Config::PLAYER_MAX_HP;
    float maxHp = Config::PLAYER_MAX_HP;
    float radius = Config::PLAYER_RADIUS;

    float armor = Config::PLAYER_ARMOR;        // 伤害减免 0–1
    float magnetRange = Config::PLAYER_MAGNET; // 宝石拾取范围
    float xpMultiplier = 1.0f;

    int level = 1;
    float xp = 0.f;
    float xpToNext = Config::XP_BASE_THRESHOLD;

    float invincibilityTimer = 0.f; // > 0 表示无敌，不受伤害
};
