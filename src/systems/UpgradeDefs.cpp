#include "systems/UpgradeDefs.hpp"

#include "core/Random.hpp"
#include <algorithm>
#include <cstdio>

// 属性提升定义
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
    char buf[128];

    // 1. 新武器（未拥有且槽位有空余）
    if (!weapons.isFull()) {
        for (int i = 0; i < static_cast<int>(WeaponType::Count); ++i) {
            auto wt = static_cast<WeaponType>(i);
            if (!weapons.hasWeapon(wt)) {
                const auto& def = WEAPON_DEFS[i];
                UpgradeOption opt{};
                opt.category = UpgradeCategory::NewWeapon;
                opt.name = def.name;
                opt.description = "New weapon";
                opt.detail = def.isAOE ? "AoE aura, no projectiles" : "Auto-targeting projectile";
                opt.weaponType = wt;
                pool.push_back(opt);
            }
        }
    }

    // 2. 可升级的已拥有武器
    auto upgradeable = weapons.getUpgradeableWeapons();
    for (auto wt : upgradeable) {
        const auto& def = WEAPON_DEFS[static_cast<int>(wt)];
        int curLvl = weapons.getLevel(wt);
        auto cur = getWeaponStats(wt, curLvl);
        auto nxt = getWeaponStats(wt, curLvl + 1);

        UpgradeOption opt{};
        opt.category = UpgradeCategory::WeaponUpgrade;
        opt.name = def.name;
        opt.weaponType = wt;

        std::snprintf(buf, sizeof(buf), "Lv.%d -> Lv.%d", curLvl, curLvl + 1);
        opt.description = buf;

        if (def.isAOE) {
            std::snprintf(buf, sizeof(buf), "Dmg %.0f->%.0f | AoE %.0f->%.0f | CD %.2f->%.2fs",
                          cur.damage, nxt.damage, cur.aoeRadius, nxt.aoeRadius, cur.cooldown,
                          nxt.cooldown);
        } else {
            std::snprintf(buf, sizeof(buf),
                          "Dmg %.0f->%.0f | CD %.2f->%.2fs | Proj %d->%d | Pierce %d->%d",
                          cur.damage, nxt.damage, cur.cooldown, nxt.cooldown, cur.projectileCount,
                          nxt.projectileCount, cur.pierce, nxt.pierce);
        }
        opt.detail = buf;
        pool.push_back(opt);
    }

    // 3. 属性提升（始终可选）
    for (const auto& sb : STAT_BOOSTS) {
        UpgradeOption opt{};
        opt.category = UpgradeCategory::StatBoost;
        opt.name = sb.name;
        std::snprintf(buf, sizeof(buf), "%s (currently %.0f)", sb.description,
                      sb.hp > 0.f ? 0.f // placeholder — actual stat in player unused
                                  : 0.f);
        opt.description = sb.description;
        opt.detail = "";
        opt.hpBonus = sb.hp;
        opt.speedBonus = sb.speed;
        opt.armorBonus = sb.armor;
        opt.magnetBonus = sb.magnet;
        opt.xpMultiplierBonus = sb.xpMult;
        pool.push_back(opt);
    }

    // 随机打乱，取最多 3 个
    std::shuffle(pool.begin(), pool.end(), Random::getEngine());

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
        player.hp += option.hpBonus; // 同时回复等量 HP
        player.speed += player.speed * option.speedBonus;
        player.armor += option.armorBonus;
        if (player.armor > 0.5f)
            player.armor = 0.5f; // 上限 50%
        player.magnetRange += option.magnetBonus;
        player.xpMultiplier += option.xpMultiplierBonus;
        break;
    }

    // 升级后：HP 钳制到最大值
    if (player.hp > player.maxHp)
        player.hp = player.maxHp;
    if (player.hp < 0.f)
        player.hp = 0.f;
}
