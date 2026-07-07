#pragma once

#include <SFML/System/Vector2.hpp>

#include <cstdint>

struct SpriteSheet;

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
    bool killed = false;     // 已结算掉落（防止重复）
    float frozenTimer = 0.f; // 冻结时间（>0表示被冻结）
    EnemyType type = EnemyType::Basic;
    float spriteScale = 1.0f; // 精灵绘制缩放
    bool facingRight = true;  // 朝向（true=右，false=左）

    // 精灵动画
    const SpriteSheet* spriteMove = nullptr;    // 移动动画精灵表
    const SpriteSheet* spriteDamaged = nullptr; // 受击动画精灵表
    const SpriteSheet* currentSprite = nullptr; // 当前使用的精灵表（每帧更新）
    float animTimer = 0.f;                      // 帧计时器
    int animFrame = 0;                          // 当前帧索引
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
    float aoeRadius = 0.f; // 爆炸半径（> 0 时命中触发范围伤害）
    int pierceCount = 0;   // 剩余穿透数（0 = 命中即消失）

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
