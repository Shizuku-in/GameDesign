#pragma once

#include "data/PlayerState.hpp"
#include "systems/WeaponSystem.hpp"

#include <string>
#include <vector>

enum class UpgradeCategory { NewWeapon, WeaponUpgrade, StatBoost };

struct UpgradeOption {
    UpgradeCategory category;
    std::string name;
    std::string description; // 简短说明
    std::string detail;      // 详细数值对比（WeaponUpgrade 时显示前后属性）
    WeaponType weaponType;   // NewWeapon 或 WeaponUpgrade 时有效

    // 属性加成（仅 StatBoost 时有效）
    float hpBonus = 0.f;
    float speedBonus = 0.f;
    float armorBonus = 0.f;
    float magnetBonus = 0.f;
    float xpMultiplierBonus = 0.f;
};

/// 生成 3 个（或更少，若选项池不足）随机升级选项。
std::vector<UpgradeOption> generateUpgrades(const PlayerState& player, const WeaponSystem& weapons);

/// 将选中的升级应用到玩家状态和武器系统。
void applyUpgrade(PlayerState& player, WeaponSystem& weapons, const UpgradeOption& option);
