#pragma once

#include <SFML/System/Vector2.hpp>

#include <cstdint>

struct SpriteSheet;

/// 敌人种类，用于索引敌人定义和精灵表。
enum class EnemyType : std::uint8_t { Basic, Fast, Tank, Boss, Count };

/// 存储在 Pool<Enemy> 中的敌人运行时状态。
struct Enemy {
    /// 敌人的世界空间位置。
    sf::Vector2f pos;
    /// 每帧计算的移动方向，指向玩家。
    sf::Vector2f vel;
    /// 当前生命值。
    float hp = 0.f;
    /// 受击闪白动画的剩余时间。
    float hitFlashTimer = 0.f;
    /// 生命值上限。
    float maxHp = 0.f;
    /// 移动速度（像素/秒）。
    float speed = 0.f;
    /// 圆形碰撞半径。
    float radius = 0.f;
    /// 接触伤害基数（每秒）。
    float damage = 0.f;
    /// 被击杀后掉落的经验值。
    float xpValue = 0.f;
    /// 是否已经结算击杀掉落，防止重复生成宝石。
    bool killed = false;
    /// 冻结效果的剩余时间。
    float frozenTimer = 0.f;
    /// 敌人种类。
    EnemyType type = EnemyType::Basic;
    /// 精灵绘制缩放倍数。
    float spriteScale = 1.0f;
    /// 水平朝向，true 表示面向右侧。
    bool facingRight = true;

    /// 常态移动动画精灵表。
    const SpriteSheet* spriteMove = nullptr;
    /// 受击动画精灵表。
    const SpriteSheet* spriteDamaged = nullptr;
    /// 当前渲染使用的精灵表。
    const SpriteSheet* currentSprite = nullptr;
    /// 到下一动画帧的累计时间。
    float animTimer = 0.f;
    /// 当前动画帧索引。
    int animFrame = 0;
};

/// 弹幕支持的运动方式。
enum class ProjMotion : std::uint8_t { Linear, Orbit };

/// 存储在 Pool<Projectile> 中的弹幕运行时状态。
struct Projectile {
    /// 弹幕的世界空间位置。
    sf::Vector2f pos;
    /// 线性运动使用的归一化方向乘以速度。
    sf::Vector2f vel;
    /// 单次命中造成的伤害。
    float damage = 0.f;
    /// 线性运动速度（像素/秒）。
    float speed = 0.f;
    /// 剩余存活时间（秒）。
    float lifetime = 0.f;
    /// 弹幕圆形碰撞半径。
    float radius = 0.f;
    /// 爆炸范围半径；大于零时命中触发范围伤害。
    float aoeRadius = 0.f;
    /// 剩余穿透次数，零表示命中后消失。
    int pierceCount = 0;

    /// 当前使用的弹幕运动算法。
    ProjMotion motion = ProjMotion::Linear;

    /// 各种运动算法专有状态的内存复用联合体。
    union {
        struct {
            /// 当前绕行角度（弧度）。
            float angle;
            /// 与玩家的固定距离。
            float radius;
            /// 绕行角速度（弧度/秒）。
            float speed;
        } orbit;
    } state;
};

/// 世界空间中短暂显示的伤害数值。
struct DamageText {
    /// 飘字的世界空间位置。
    sf::Vector2f pos;
    /// 飘字的移动速度。
    sf::Vector2f vel;
    /// 展示的伤害数值。
    float damage = 0.f;
    /// 剩余显示时间。
    float lifetime = 0.f;
    /// 创建时设定的总显示时间，用于计算透明度。
    float maxLifetime = 0.f;
};

/// 可被玩家拾取并提供经验的世界实体。
struct XPGem {
    /// 宝石的世界空间位置。
    sf::Vector2f pos;
    /// 拾取后提供的基础经验值。
    float value = 0.f;
    /// 宝石的圆形碰撞半径。
    float radius = 5.f;
    /// 启用磁吸前的剩余延迟时间。
    float magnetTimer = 0.f;
};
