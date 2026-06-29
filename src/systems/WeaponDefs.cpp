#include "systems/WeaponDefs.hpp"

#include <algorithm>
#include <cmath>

const WeaponDef WEAPON_DEFS[] = {
    // MagicWand — fast, low-damage homing bolt
    {WeaponType::MagicWand, "Magic Wand", 0.8f, 10.f, 500.f, 1.2f, 4.f, 500.f, 1, 0, 8, false, 0.f},
    // Knife — fires in facing direction, high speed, pierces
    {WeaponType::Knife, "Knife", 1.0f, 8.f, 600.f, 1.0f, 3.f, 0.f, 1, 3, 8, false, 0.f},
    // Axe — orbiting projectile around player
    {WeaponType::Axe, "Axe", 2.0f, 25.f, 0.f, 4.0f, 8.f, 0.f, 1, 99, 8, false, 0.f},
    // Fireball — slow projectile with AoE explosion on first hit
    {WeaponType::Fireball, "Fireball", 1.5f, 20.f, 250.f, 1.5f, 6.f, 400.f, 1, 0, 8, false, 60.f},
    // Garlic — persistent AoE damage around player, no projectiles
    {WeaponType::Garlic, "Garlic", 0.5f, 5.f, 0.f, 0.f, 0.f, 0.f, 0, 0, 8, true, 80.f},
};

static_assert(sizeof(WEAPON_DEFS) / sizeof(WEAPON_DEFS[0]) == static_cast<int>(WeaponType::Count),
              "WEAPON_DEFS must have one entry per WeaponType");

WeaponStats getWeaponStats(WeaponType type, int level) {
    const auto& def = WEAPON_DEFS[static_cast<int>(type)];
    int lvl = std::clamp(level, 1, def.maxLevel);
    int n = lvl - 1; // number of upgrades from base

    WeaponStats s{};
    s.cooldown = def.baseCooldown * std::pow(0.95f, static_cast<float>(n));
    s.damage = def.baseDamage * std::pow(1.30f, static_cast<float>(n));
    s.projectileSpeed = def.projectileSpeed;
    s.projectileLifetime = def.projectileLifetime;
    s.projectileRadius = def.projectileRadius;
    s.range = def.range;
    s.projectileCount = def.baseProjectiles + n / 2;
    s.pierce = def.basePierce + n / 3;
    s.aoeRadius = def.isAOE ? def.aoeRadius * (1.0f + 0.1f * static_cast<float>(n)) : 0.f;
    return s;
}
