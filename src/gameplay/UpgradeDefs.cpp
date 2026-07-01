#include "gameplay/UpgradeDefs.hpp"

#include "core/Random.hpp"
#include "gameplay/WeaponDefs.hpp"

#include <algorithm>
#include <format>

using enum UpgradeCategory;
using enum WeaponType;

// ===========================================================================
// 辅助函数
// ===========================================================================

namespace {

// --- StatBoost ---

bool statAvailable(const PlayerState& /*unused*/, const WeaponSystem& /*unused*/) { return true; }

void applyStatBoost(PlayerState& player, WeaponSystem& /*unused*/, const UpgradeDef& def) {
    player.maxHp += def.hpBonus;
    player.hp += def.hpBonus;
    player.speed += player.baseSpeed * def.speedBonus; // 基于基础速度，非复利
    player.armor += def.armorBonus;
    player.armor = std::min(player.armor, Config::PLAYER_MAX_ARMOR);
    player.magnetRange += def.magnetBonus;
    player.damageBonus += def.damageBonus;
    player.cooldownReduction += def.cooldownBonus;
    player.cooldownReduction =
        std::min(player.cooldownReduction, Config::PLAYER_MAX_COOLDOWN_REDUCTION);
    player.xpMultiplier += def.xpMultiplierBonus;
}

std::string detailStatBoost(const UpgradeDef& def, const PlayerState& player,
                            const WeaponSystem& /*unused*/) {
    if (def.hpBonus > 0.f) {
        return std::format("Current Max HP: {:.0f}", player.maxHp);
    }
    if (def.speedBonus > 0.f) {
        return std::format("Current speed: {:.0f}", player.speed);
    }
    if (def.armorBonus > 0.f) {
        return std::format("Current armor: {:.0f}%", player.armor * 100.f);
    }
    if (def.magnetBonus > 0.f) {
        return std::format("Current range: {:.0f}", player.magnetRange);
    }
    if (def.damageBonus > 0.f) {
        return std::format("Current Might: +{:.0f}%", player.damageBonus * 100.f);
    }
    if (def.cooldownBonus > 0.f) {
        return std::format("Current CDR: {:.0f}%", player.cooldownReduction * 100.f);
    }
    if (def.xpMultiplierBonus > 0.f) {
        return std::format("Current XP mult: {:.0f}%", player.xpMultiplier * 100.f);
    }
    return "";
}

UpgradeDef makeStatBoost(const char* name, const char* desc, float hp, float speed, float armor,
                         float magnet, float dmg, float cd, float xpMult) {
    return {StatBoost, name, desc,   detailStatBoost, MagicWand,     hp, speed, armor, magnet,
            dmg,       cd,   xpMult, statAvailable,   applyStatBoost};
}

// --- NewWeapon ---

bool newWeaponAvailable(const PlayerState& /*unused*/, const WeaponSystem& weapons) {
    return !weapons.isFull();
}

void applyNewWeapon(PlayerState& /*unused*/, WeaponSystem& weapons, const UpgradeDef& def) {
    weapons.addWeapon(def.weaponType);
}

std::string detailNewWeapon(const UpgradeDef& def, const PlayerState& /*unused*/,
                            const WeaponSystem& /*unused*/) {
    const auto& wd = WEAPON_DEFS[static_cast<int>(def.weaponType)];
    if (wd.isAOE) {
        return "AoE aura, no projectiles";
    }
    if (wd.aoeRadius > 0.f) {
        return "Projectile, explodes on hit";
    }
    return "Auto-targeting projectile";
}

// --- WeaponUpgrade ---

void applyWeaponUpgrade(PlayerState& /*unused*/, WeaponSystem& weapons, const UpgradeDef& def) {
    weapons.upgradeWeapon(def.weaponType);
}

std::string detailWeaponUpgrade(const UpgradeDef& def, const PlayerState& /*unused*/,
                                const WeaponSystem& weapons) {
    int curLvl = weapons.getLevel(def.weaponType);
    auto cur = getWeaponStats(def.weaponType, curLvl);
    auto nxt = getWeaponStats(def.weaponType, curLvl + 1);
    const auto& wd = WEAPON_DEFS[static_cast<int>(def.weaponType)];

    if (wd.isAOE) {
        return std::format("Dmg {:.0f}->{:.0f} | AoE {:.0f}->{:.0f} | CD {:.2f}->{:.2f}s",
                           cur.damage, nxt.damage, cur.aoeRadius, nxt.aoeRadius, cur.cooldown,
                           nxt.cooldown);
    }
    return std::format("Dmg {:.0f}->{:.0f} | CD {:.2f}->{:.2f}s | Proj {}->{} | Pierce {}->{}",
                       cur.damage, nxt.damage, cur.cooldown, nxt.cooldown, cur.projectileCount,
                       nxt.projectileCount, cur.pierce, nxt.pierce);
}

// 构建完整升级表
std::vector<UpgradeDef> buildUpgradeDefs() {
    std::vector<UpgradeDef> d;

    // --- 属性提升（7 项） ---
    d.push_back(makeStatBoost("Vitality", "+3 Max HP, heal 3", 3.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f));
    d.push_back(
        makeStatBoost("Swiftness", "+10% movement speed", 0.f, 0.10f, 0.f, 0.f, 0.f, 0.f, 0.f));
    d.push_back(makeStatBoost("Armor", "+5% damage reduction (max 50%)", 0.f, 0.f, 0.05f, 0.f, 0.f,
                              0.f, 0.f));
    d.push_back(makeStatBoost("Magnet", "+30 pickup range", 0.f, 0.f, 0.f, 30.f, 0.f, 0.f, 0.f));
    d.push_back(makeStatBoost("Might", "+25% weapon damage", 0.f, 0.f, 0.f, 0.f, 0.25f, 0.f, 0.f));
    d.push_back(makeStatBoost("Haste", "+10% cooldown reduction (max 60%)", 0.f, 0.f, 0.f, 0.f, 0.f,
                              0.10f, 0.f));
    d.push_back(makeStatBoost("Greed", "+15% XP gain", 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.15f));

    // --- 新武器 + 武器升级（从 WEAPON_DEFS 自动生成） ---
    for (int i = 0; i < static_cast<int>(Count); ++i) {
        const auto& wd = WEAPON_DEFS[i];

        // NewWeapon
        d.push_back({NewWeapon, wd.name, "New weapon", detailNewWeapon, static_cast<WeaponType>(i),
                     0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, newWeaponAvailable, applyNewWeapon});

        // WeaponUpgrade
        d.push_back({WeaponUpgrade, wd.name, "Upgrade weapon", detailWeaponUpgrade,
                     static_cast<WeaponType>(i), 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, nullptr,
                     applyWeaponUpgrade});
    }

    return d;
}

const std::vector<UpgradeDef>& getUpgradeDefs() {
    static const std::vector<UpgradeDef> DEFS = buildUpgradeDefs();
    return DEFS;
}

} // namespace

// ===========================================================================
// 运行时逻辑
// ===========================================================================

std::vector<UpgradeOption> generateUpgrades(const PlayerState& player,
                                            const WeaponSystem& weapons) {
    const auto& defs = getUpgradeDefs();
    std::vector<UpgradeOption> pool;

    for (int i = 0; i < std::ssize(defs); ++i) {
        const auto& def = defs[i];

        // 武器升级走专用路径（需要 getUpgradeableWeapons）
        if (def.category == WeaponUpgrade) {
            auto upgradeable = weapons.getUpgradeableWeapons();
            bool found = false;
            for (auto wt : upgradeable) {
                if (wt == def.weaponType) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                continue;
            }
        } else if ((def.available != nullptr) && !def.available(player, weapons)) {
            continue;
        }

        UpgradeOption opt{};
        opt.category = def.category;
        opt.name = def.name;
        opt.description = def.desc;
        opt.weaponType = def.weaponType;
        opt.defIndex = i;

        if (def.detailFn != nullptr) {
            opt.detail = def.detailFn(def, player, weapons);
        }

        pool.push_back(opt);
    }

    std::shuffle(pool.begin(), pool.end(), Random::getEngine());

    auto count = std::min(std::ptrdiff_t{3}, std::ssize(pool));
    return std::vector<UpgradeOption>(pool.begin(), pool.begin() + count);
}

void applyUpgrade(PlayerState& player, WeaponSystem& weapons, const UpgradeOption& option) {
    if (option.defIndex < 0 || option.defIndex >= std::ssize(getUpgradeDefs())) {
        return;
    }

    const auto& def = getUpgradeDefs()[option.defIndex];
    if (def.apply != nullptr) {
        def.apply(player, weapons, def);
    }

    // 升级后：HP 钳制到最大值
    player.hp = std::min(player.hp, player.maxHp);
    player.hp = std::max(player.hp, 0.f);
}
