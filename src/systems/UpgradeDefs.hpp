#pragma once

#include "data/PlayerState.hpp"
#include "systems/WeaponSystem.hpp"

#include <vector>

enum class UpgradeCategory { NewWeapon, WeaponUpgrade, StatBoost };

struct UpgradeOption {
    UpgradeCategory category;
    const char* name;
    const char* description;
    WeaponType weaponType; // valid if NewWeapon or WeaponUpgrade

    // Stat bonuses (only meaningful for StatBoost)
    float hpBonus = 0.f;
    float speedBonus = 0.f;
    float armorBonus = 0.f;
    float magnetBonus = 0.f;
    float xpMultiplierBonus = 0.f;
};

/// Generate 3 (or fewer if pool exhausted) random upgrade choices.
std::vector<UpgradeOption> generateUpgrades(const PlayerState& player, const WeaponSystem& weapons);

/// Apply the chosen upgrade to player state and weapon system.
void applyUpgrade(PlayerState& player, WeaponSystem& weapons, const UpgradeOption& option);
