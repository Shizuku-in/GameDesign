#pragma once

#include <cstdint>

enum class WeaponType : std::uint8_t {
    MagicWand,
    Knife,
    Axe,
    Fireball,
    Garlic, // AoE aura, no projectiles
    Count
};

/// Weapon tuning constants at level 1.
struct WeaponDef {
    WeaponType type;
    const char* name;
    float baseCooldown; // seconds between attacks
    float baseDamage;
    float projectileSpeed;    // pixels/sec (0 for AoE)
    float projectileLifetime; // seconds (0 for AoE)
    float projectileRadius;   // visual + collision
    float range;              // max targeting distance (0 = unlimited)
    int baseProjectiles;      // how many spawned per attack
    int basePierce;           // enemies hit before disappearing (0 = single target)
    int maxLevel;             // typically 8
    bool isAOE;               // Garlic-style: no projectiles, ticks damage in radius each frame
    float aoeRadius;          // only meaningful if isAOE
};

extern const WeaponDef WEAPON_DEFS[];

/// Scaled stats for a weapon at a given level (1-based).
struct WeaponStats {
    float cooldown;
    float damage;
    float projectileSpeed;
    float projectileLifetime;
    float projectileRadius;
    float range;
    int projectileCount;
    int pierce;
    float aoeRadius;
};

/// Compute stats for a weapon at level L (L >= 1).
WeaponStats getWeaponStats(WeaponType type, int level);
