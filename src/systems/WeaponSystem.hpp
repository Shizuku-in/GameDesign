#pragma once

#include "core/Pool.hpp"
#include "data/EntityTypes.hpp"
#include "data/PlayerState.hpp"
#include "systems/WeaponDefs.hpp"
#include "systems/IWeaponBehavior.hpp"

#include <vector>
#include <memory>

class SoundPlayer;

/// 管理玩家武器槽，处理自动攻击逻辑。
class WeaponSystem {
public:
    static constexpr int MAX_SLOTS = 6;

    WeaponSystem();

    // --- 槽位管理 ---
    bool addWeapon(WeaponType type);
    bool upgradeWeapon(WeaponType type); // 未拥有或已满级时返回 false
    bool hasWeapon(WeaponType type) const;
    int getLevel(WeaponType type) const;
    bool isFull() const;
    int emptySlotCount() const;
    std::vector<WeaponType> getUpgradeableWeapons() const;

    // --- 主更新（60 Hz 调用）---
    void update(float dt, const PlayerState& player, Pool<Enemy>& enemies,
                Pool<Projectile>& projectiles, SoundPlayer& sounds);

    // --- 新一局重置 ---
    void reset();

private:
    struct Slot {
        WeaponType type = WeaponType::MagicWand;
        int level = 0; // 0 = 空槽位
        float cooldown = 0.f;
        std::unique_ptr<IWeaponBehavior> behavior;
    };

    Slot m_slots[MAX_SLOTS];

};
