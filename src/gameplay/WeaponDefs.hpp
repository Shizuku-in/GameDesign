#pragma once

#include <cstdint>
#include <memory>

class IWeaponBehavior;

enum class WeaponType : std::uint8_t {
    MagicWand,
    Knife,
    Axe,
    Fireball,
    Garlic, // AoE 光环，不生成弹幕
    Count
};

/// 1 级时的武器属性常量 + 工厂函数。
struct WeaponDef {
    using CreateFn = std::unique_ptr<IWeaponBehavior> (*)();

    WeaponType type;
    const char* name;
    float baseCooldown; // 攻击间隔（秒）
    float baseDamage;
    float projectileSpeed;    // 弹幕速度（像素/秒，AoE 武器为 0）
    float projectileLifetime; // 弹幕存活时间（秒，AoE 武器为 0）
    float projectileRadius;   // 视觉 + 碰撞半径
    float range;              // 最大索敌距离（0 = 无限制）
    int baseProjectiles;      // 每次攻击发射数量
    int basePierce;           // 消失前可命中敌人数（0 = 单体）
    int maxLevel;             // 最高等级（通常为 8）
    bool isAOE;               // 大蒜类：不生成弹幕，每帧对范围内敌人造成伤害
    float aoeRadius;          // 仅 isAOE 时有效
    CreateFn create;          // 工厂函数：创建对应的行为实例
};

extern const WeaponDef WEAPON_DEFS[];

/// 指定等级（1 级起算）下的缩放属性。
struct WeaponStats {
    float cooldown;
    float damage;
    float projectileSpeed;
    float projectileLifetime;
    float projectileRadius;
    float range;
    int projectileCount;
    int pierce;
    float aoeRadius;
};

/// 计算指定武器在等级 L (L >= 1) 时的属性。
WeaponStats getWeaponStats(WeaponType type, int level);

/// 创建指定武器类型的行为实例。
std::unique_ptr<IWeaponBehavior> createWeapon(WeaponType type);
