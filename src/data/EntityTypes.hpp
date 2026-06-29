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
    float hitFlashTimer = 0.f;
    float maxHp = 0.f;
    float speed = 0.f;
    float radius = 0.f;
    float damage = 0.f; // 接触伤害（每秒）
    float xpValue = 0.f;
    EnemyType type = EnemyType::Basic;
};

// --- 弹幕运动类型 ---
enum class ProjMotion : std::uint8_t { Linear, Orbit };

// --- 弹幕实例（存储在 Pool<Projectile> 中）---
struct Projectile {
    sf::Vector2f pos;
    sf::Vector2f vel; // 归一化方向 × 速度（线性运动时使用）
    float damage = 0.f;
    float speed = 0.f;
    float lifetime = 0.f; // 剩余存活时间（秒）
    float radius = 0.f;
    int pierceCount = 0; // 剩余穿透数（0 = 命中即消失）

    // 运动逻辑分支
    ProjMotion motion = ProjMotion::Linear;

    // 具体运动所需的专有状态（使用 union 节省内存，不增加额外开销）
    union {
        struct {
            float angle;  // 当前角度（弧度）
            float radius; // 距玩家距离
            float speed;  // 角速度（弧度/秒）
        } orbit;
        // 如果以后新增回旋镖、抛物线，可以在这里加 struct
    } state;
};

// --- 经验宝石实例（存储在 Pool<XPGem> 中）---
struct DamageText {
    sf::Vector2f pos;
    sf::Vector2f vel;
    float damage = 0.f;
    float lifetime = 0.f;
    float maxLifetime = 0.f;
};

struct XPGem {
    sf::Vector2f pos;
    float value = 0.f;
    float radius = 5.f;
    float magnetTimer = 0.f; // 磁铁吸引启用前的倒计时
};
