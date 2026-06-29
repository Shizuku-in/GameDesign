#include "systems/UpgradeDefs.hpp"

#include <algorithm>
#include <random>

// Stat boost definitions
struct StatBoostDef {
    const char* name;
    const char* description;
    float hp;
    float speed;
    float armor;
    float magnet;
    float xpMult;
};

static const StatBoostDef STAT_BOOSTS[] = {
    {"Vitality", "+20 Max HP, heal 20", 20.f, 0.f, 0.f, 0.f, 0.f},
    {"Swiftness", "+10% movement speed", 0.f, 0.10f, 0.f, 0.f, 0.f},
    {"Armor", "+5% damage reduction (max 50%)", 0.f, 0.f, 0.05f, 0.f, 0.f},
    {"Magnet", "+30 pickup range", 0.f, 0.f, 0.f, 30.f, 0.f},
    {"Greed", "+15% XP gain", 0.f, 0.f, 0.f, 0.f, 0.15f},
};

std::vector<UpgradeOption> generateUpgrades(const PlayerState& /*player*/,
                                            const WeaponSystem& weapons) {
    std::vector<UpgradeOption> pool;

    // 1. New weapons (types the player doesn't own, if slots available)
    if (!weapons.isFull()) {
        for (int i = 0; i < static_cast<int>(WeaponType::Count); ++i) {
            auto wt = static_cast<WeaponType>(i);
            if (!weapons.hasWeapon(wt)) {
                const auto& def = WEAPON_DEFS[i];
                UpgradeOption opt{};
                opt.category = UpgradeCategory::NewWeapon;
                opt.name = def.name;
                opt.description = "New weapon";
                opt.weaponType = wt;
                pool.push_back(opt);
            }
        }
    }

    // 2. Upgradeable owned weapons
    auto upgradeable = weapons.getUpgradeableWeapons();
    for (auto wt : upgradeable) {
        const auto& def = WEAPON_DEFS[static_cast<int>(wt)];
        UpgradeOption opt{};
        opt.category = UpgradeCategory::WeaponUpgrade;
        opt.name = def.name;
        opt.description = "Upgrade weapon";
        opt.weaponType = wt;
        pool.push_back(opt);
    }

    // 3. Stat boosts (always available)
    for (const auto& sb : STAT_BOOSTS) {
        UpgradeOption opt{};
        opt.category = UpgradeCategory::StatBoost;
        opt.name = sb.name;
        opt.description = sb.description;
        opt.hpBonus = sb.hp;
        opt.speedBonus = sb.speed;
        opt.armorBonus = sb.armor;
        opt.magnetBonus = sb.magnet;
        opt.xpMultiplierBonus = sb.xpMult;
        pool.push_back(opt);
    }

    // Shuffle and pick up to 3
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(pool.begin(), pool.end(), g);

    int count = std::min(3, static_cast<int>(pool.size()));
    return std::vector<UpgradeOption>(pool.begin(), pool.begin() + count);
}

void applyUpgrade(PlayerState& player, WeaponSystem& weapons, const UpgradeOption& option) {
    switch (option.category) {
    case UpgradeCategory::NewWeapon:
        weapons.addWeapon(option.weaponType);
        break;

    case UpgradeCategory::WeaponUpgrade:
        weapons.upgradeWeapon(option.weaponType);
        break;

    case UpgradeCategory::StatBoost:
        player.maxHp += option.hpBonus;
        player.hp += option.hpBonus; // heal for the same amount
        player.speed += player.speed * option.speedBonus;
        player.armor += option.armorBonus;
        if (player.armor > 0.5f)
            player.armor = 0.5f; // cap at 50%
        player.magnetRange += option.magnetBonus;
        player.xpMultiplier += option.xpMultiplierBonus;
        break;
    }

    // After upgrade: clamp HP to max
    if (player.hp > player.maxHp)
        player.hp = player.maxHp;
    if (player.hp < 0.f)
        player.hp = 0.f;
}
