#include "gameplay/WeaponDefs.hpp"

#include "systems/WeaponBehaviors.hpp"

const WeaponDef WEAPON_DEFS[] = {
    // MagicWand — 快速、低伤追踪弹
    {.type = WeaponType::MagicWand,
     .name = "Magic Wand",
     .baseCooldown = 0.8f,
     .baseDamage = 10.f,
     .projectileSpeed = 500.f,
     .projectileLifetime = 1.2f,
     .projectileRadius = 4.f,
     .range = 500.f,
     .baseProjectiles = 1,
     .basePierce = 0,
     .maxLevel = 8,
     .create = []() -> std::unique_ptr<IWeaponBehavior> {
         return std::make_unique<MagicWandBehavior>();
     }},
    // Knife — 向敌人方向发射，高速穿透
    {.type = WeaponType::Knife,
     .name = "Knife",
     .baseCooldown = 1.0f,
     .baseDamage = 8.f,
     .projectileSpeed = 600.f,
     .projectileLifetime = 1.0f,
     .projectileRadius = 3.f,
     .baseProjectiles = 1,
     .basePierce = 3,
     .maxLevel = 8,
     .spread = 0.15f,
     .create = []() -> std::unique_ptr<IWeaponBehavior> {
         return std::make_unique<KnifeBehavior>();
     }},
    // Axe — 环绕玩家旋转
    {.type = WeaponType::Axe,
     .name = "Axe",
     .baseCooldown = 2.0f,
     .baseDamage = 25.f,
     .projectileLifetime = 4.0f,
     .projectileRadius = 8.f,
     .baseProjectiles = 1,
     .basePierce = 99,
     .maxLevel = 8,
     .orbitRadius = 60.f,
     .orbitSpeed = 3.0f,
     .create = []() -> std::unique_ptr<IWeaponBehavior> {
         return std::make_unique<AxeBehavior>();
     }},
    // Fireball — 慢速火球，首次命中爆炸（AoE）
    {.type = WeaponType::Fireball,
     .name = "Fireball",
     .baseCooldown = 1.5f,
     .baseDamage = 20.f,
     .projectileSpeed = 250.f,
     .projectileLifetime = 1.5f,
     .projectileRadius = 6.f,
     .range = 400.f,
     .baseProjectiles = 1,
     .basePierce = 0,
     .maxLevel = 8,
     .aoeRadius = 60.f,
     .create = []() -> std::unique_ptr<IWeaponBehavior> {
         return std::make_unique<FireballBehavior>();
     }},
    // Garlic — 持续 AoE，无弹幕，每帧对范围内敌人造成伤害
    {.type = WeaponType::Garlic,
     .name = "Garlic",
     .baseCooldown = 0.5f,
     .baseDamage = 5.f,
     .maxLevel = 8,
     .isAOE = true,
     .aoeRadius = 80.f,
     .create = []() -> std::unique_ptr<IWeaponBehavior> {
         return std::make_unique<GarlicBehavior>();
     }},
};

static_assert(sizeof(WEAPON_DEFS) / sizeof(WEAPON_DEFS[0]) == static_cast<int>(WeaponType::Count),
              "WEAPON_DEFS must have one entry per WeaponType");

std::unique_ptr<IWeaponBehavior> createWeapon(WeaponType type) {
    auto idx = static_cast<std::size_t>(type);
    if (idx >= static_cast<std::size_t>(WeaponType::Count)) {
        return nullptr;
    }
    return WEAPON_DEFS[idx].create();
}
