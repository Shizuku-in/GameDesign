#pragma once

#include "core/Pool.hpp"
#include "data/EntityTypes.hpp"
#include "data/PlayerState.hpp"
#include "gameplay/WeaponDefs.hpp"

#include <memory>

/// 武器行为策略的抽象基类，分别支持主动发射和持续范围效果。
class IWeaponBehavior {
public:
    /// 支持经由基类指针销毁具体武器行为。
    virtual ~IWeaponBehavior() = default;

    /// 尝试执行一次主动攻击；返回 true 时调用方播放射击音效。
    virtual bool fire(int /*level*/, const PlayerState& /*player*/, Pool<Enemy>& /*enemies*/,
                      Pool<Projectile>& /*projectiles*/) {
        return false;
    }

    /// 更新范围型武器对敌人的持续效果。
    virtual void tickAoE(int /*level*/, const PlayerState& /*player*/, Pool<Enemy>& /*enemies*/) {}
};
