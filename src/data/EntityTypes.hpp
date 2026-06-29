#pragma once

#include <SFML/System/Vector2.hpp>

#include <cstdint>

// --- 敌人类型 ---
enum class EnemyType : std::uint8_t { Basic, Fast, Tank, Boss, Count };

// --- 敌人实例（存储在 Pool<Enemy> 中）---
struct Enemy {
    sf::Vector2f pos;
    sf::Vector2f vel; // 每帧计算，朝向玩家
    float hp = 0.f;
    float maxHp = 0.f;
    float speed = 0.f;
    float radius = 0.f;
    float damage = 0.f; // 接触伤害（每秒）
    float xpValue = 0.f;
    EnemyType type = EnemyType::Basic;
};

// --- 弹幕实例（存储在 Pool<Projectile> 中）---
struct Projectile {
    sf::Vector2f pos;
    sf::Vector2f vel; // 归一化方向 × 速度（轨道运动时不使用）
    float damage = 0.f;
    float speed = 0.f;
    float lifetime = 0.f; // 剩余存活时间（秒）
    float radius = 0.f;
    int pierceCount = 0; // 剩余穿透数（0 = 命中即消失）

    // 轨道运动状态（orbitRadius > 0 表示弹幕环绕玩家运动，不使用 vel）
    float orbitAngle = 0.f;  // 当前角度（弧度）
    float orbitRadius = 0.f; // 距玩家距离; 0 = 非轨道运动
    float orbitSpeed = 0.f;  // 角速度（弧度/秒）
};

// --- 经验宝石实例（存储在 Pool<XPGem> 中）---
struct XPGem {
    sf::Vector2f pos;
    float value = 0.f;
    float radius = 5.f;
    float magnetTimer = 0.f; // 磁铁吸引启用前的倒计时
};
