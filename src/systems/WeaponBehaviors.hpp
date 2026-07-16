#pragma once
#include "systems/IWeaponBehavior.hpp"

/// 魔法杖策略：向最近敌人发射追踪弹。
class MagicWandBehavior : public IWeaponBehavior {
public:
    /// 生成当前等级的追踪魔法弹。
    bool fire(int level, const PlayerState& player, Pool<Enemy>& enemies,
              Pool<Projectile>& proj) override;
};

/// 飞刀策略：按扇形角度发射多枚直线弹幕。
class KnifeBehavior : public IWeaponBehavior {
public:
    /// 生成当前等级的扇形飞刀。
    bool fire(int level, const PlayerState& player, Pool<Enemy>& enemies,
              Pool<Projectile>& proj) override;
};

/// 斧头策略：生成围绕玩家旋转的轨道弹幕。
class AxeBehavior : public IWeaponBehavior {
public:
    /// 生成当前等级的轨道斧。
    bool fire(int level, const PlayerState& player, Pool<Enemy>& enemies,
              Pool<Projectile>& proj) override;

private:
    /// 下一枚轨道斧的起始角度，保证连续发射时均匀分布。
    float m_orbitBaseAngle = 0.f;
};

/// 火球策略：发射命中后造成范围伤害的弹幕。
class FireballBehavior : public IWeaponBehavior {
public:
    /// 生成当前等级的爆炸火球。
    bool fire(int level, const PlayerState& player, Pool<Enemy>& enemies,
              Pool<Projectile>& proj) override;
};

/// 大蒜策略：对范围内敌人施加持续伤害。
class GarlicBehavior : public IWeaponBehavior {
public:
    /// 执行一帧范围伤害结算。
    void tickAoE(int level, const PlayerState& player, Pool<Enemy>& enemies) override;
};

/// 时停策略：对范围内敌人施加短暂冻结。
class TimeStopBehavior : public IWeaponBehavior {
public:
    /// 执行一帧范围冻结结算。
    void tickAoE(int level, const PlayerState& player, Pool<Enemy>& enemies) override;
};
