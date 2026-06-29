#pragma once

#include "core/Pool.hpp"
#include "data/EntityTypes.hpp"
#include "data/PlayerState.hpp"
#include "systems/WeaponDefs.hpp"
#include <memory>

class IWeaponBehavior {
public:
    virtual ~IWeaponBehavior() = default;

    // 返回 true 表示成功发射，需要播放音效
    virtual bool fire(int /*level*/, const PlayerState& /*player*/, Pool<Enemy>& /*enemies*/,
                      Pool<Projectile>& /*projectiles*/) {
        return false;
    }

    // AoE 武器每帧更新
    virtual void tickAoE(int /*level*/, const PlayerState& /*player*/, Pool<Enemy>& /*enemies*/) {}
};
