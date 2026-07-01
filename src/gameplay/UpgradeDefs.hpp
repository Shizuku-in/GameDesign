#pragma once

#include "data/PlayerState.hpp"
#include "gameplay/WeaponDefs.hpp"
#include "systems/WeaponSystem.hpp"

#include <string>
#include <vector>

enum class UpgradeCategory : std::uint8_t { NewWeapon, WeaponUpgrade, StatBoost };

// --- 升级定义（数据表行） ---

struct UpgradeDef;
using AvailFn = bool (*)(const PlayerState&, const WeaponSystem&);
using DetailFn = std::string (*)(const UpgradeDef&, const PlayerState&, const WeaponSystem&);
using ApplyFn = void (*)(PlayerState&, WeaponSystem&, const UpgradeDef&);

struct UpgradeDef {
    UpgradeCategory category;
    const char* name;
    const char* desc; // 静态描述

    DetailFn detailFn = nullptr; // 动态详情（null = 无详情）

    WeaponType weaponType = WeaponType::MagicWand; // NewWeapon / WeaponUpgrade 时有效

    // StatBoost 属性加成
    float hpBonus = 0.f;
    float speedBonus = 0.f; // 基于基础速度的加法（非复利）
    float armorBonus = 0.f;
    float magnetBonus = 0.f;
    float damageBonus = 0.f;   // Might
    float cooldownBonus = 0.f; // Haste
    float xpMultiplierBonus = 0.f;

    AvailFn available = nullptr; // null = 始终可选
    ApplyFn apply = nullptr;     // null = 无操作
};

// --- 运行时选项（UI 显示用） ---

struct UpgradeOption {
    UpgradeCategory category;
    std::string name;
    std::string description;
    std::string detail;
    WeaponType weaponType = WeaponType::MagicWand;
    int defIndex = -1; // 指向 UPGRADE_DEFS 的索引
};

/// 生成 3 个（或更少）随机升级选项。
std::vector<UpgradeOption> generateUpgrades(const PlayerState& player, const WeaponSystem& weapons);

/// 将选中的升级应用到玩家状态和武器系统。
void applyUpgrade(PlayerState& player, WeaponSystem& weapons, const UpgradeOption& option);
