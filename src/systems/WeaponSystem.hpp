#pragma once

#include "core/Pool.hpp"
#include "data/EntityTypes.hpp"
#include "data/PlayerState.hpp"
#include "gameplay/WeaponDefs.hpp"
#include "systems/IWeaponBehavior.hpp"

#include <memory>
#include <vector>

class SoundPlayer;

/// 管理玩家武器槽，处理自动攻击逻辑。
class WeaponSystem {
public:
    /// 玩家可同时装备的最大武器数。
    static constexpr int MAX_SLOTS = 6;

    /// 创建空武器槽并添加默认武器。
    WeaponSystem();

    /// 尝试添加指定类型的 1 级武器；槽位已满或已拥有时失败。
    bool addWeapon(WeaponType type);
    /// 升级指定武器；未拥有或达到最高等级时返回 false。
    bool upgradeWeapon(WeaponType type);
    /// 判断指定武器是否已装备。
    [[nodiscard]] bool hasWeapon(WeaponType type) const;
    /// 返回指定武器的等级；未装备时返回 0。
    [[nodiscard]] int getLevel(WeaponType type) const;
    /// 判断所有武器槽是否均已占用。
    [[nodiscard]] bool isFull() const;
    /// 返回当前仍可升级的已装备武器列表。
    [[nodiscard]] std::vector<WeaponType> getUpgradeableWeapons() const;

    /// 更新所有武器冷却、发射和范围效果；返回 true 表示至少一次主动攻击成功。
    bool update(float dt, const PlayerState& player, Pool<Enemy>& enemies,
                Pool<Projectile>& projectiles, SoundPlayer& sounds);

    /// 清空所有武器槽，并恢复默认武器。
    void reset();

private:
    /// 单个武器槽的类型、等级、冷却与行为实例。
    struct Slot {
        /// 槽内武器类型；空槽时值无业务含义。
        WeaponType type = WeaponType::MagicWand;
        int level = 0; // 0 = 空槽位
        /// 距离下一次主动攻击的剩余时间。
        float cooldown = 0.f;
        /// 当前武器类型对应的行为策略。
        std::unique_ptr<IWeaponBehavior> behavior;
    };

    /// 固定大小的玩家武器槽数组。
    Slot m_slots[MAX_SLOTS];
};
