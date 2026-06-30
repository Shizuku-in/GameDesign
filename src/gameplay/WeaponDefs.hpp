#pragma once

#include <algorithm>
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

    // 武器专属参数（不随等级缩放）
    float spread;      // 多弹幕散布角（弧度，Knife 用）
    float orbitRadius; // 轨道半径（Axe 用）
    float orbitSpeed;  // 轨道角速度（弧度/秒，Axe 用）

    CreateFn create; // 工厂函数：创建对应的行为实例
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
constexpr float constpow(float base, int n) {
    float r = 1.f;
    for (int i = 0; i < n; ++i)
        r *= base;
    return r;
}

inline constexpr WeaponStats getWeaponStats(WeaponType type, int level) {
    const auto& def = WEAPON_DEFS[static_cast<int>(type)];
    int lvl = std::clamp(level, 1, def.maxLevel);
    int n = lvl - 1; // 从基础等级起的升级次数

    WeaponStats s{};
    s.cooldown = def.baseCooldown * constpow(0.95f, n);
    s.damage = def.baseDamage * constpow(1.30f, n);
    s.projectileSpeed = def.projectileSpeed;
    s.projectileLifetime = def.projectileLifetime;
    s.projectileRadius = def.projectileRadius;
    s.range = def.range;
    s.projectileCount = def.baseProjectiles + n / 2;
    s.pierce = def.basePierce + n / 3;
    s.aoeRadius = (def.isAOE || def.aoeRadius > 0.f)
                      ? def.aoeRadius * (1.0f + 0.1f * static_cast<float>(n))
                      : 0.f;
    return s;
}

/// 创建指定武器类型的行为实例。
std::unique_ptr<IWeaponBehavior> createWeapon(WeaponType type);
