#pragma once

#include "core/Pool.hpp"
#include "data/EntityTypes.hpp"
#include "data/PlayerState.hpp"
#include "systems/WeaponDefs.hpp"

#include <vector>

/// Manages the player's weapon slots and handles auto-attack logic.
class WeaponSystem {
public:
    static constexpr int MAX_SLOTS = 6;

    WeaponSystem();

    // --- Slot management ---
    bool addWeapon(WeaponType type);
    bool upgradeWeapon(WeaponType type); // returns false if not owned or already maxed
    bool hasWeapon(WeaponType type) const;
    int getLevel(WeaponType type) const;
    bool isFull() const;
    int emptySlotCount() const;
    std::vector<WeaponType> getUpgradeableWeapons() const;

    // --- Main update (called at 60 Hz) ---
    void update(float dt, const PlayerState& player, Pool<Enemy>& enemies,
                Pool<Projectile>& projectiles);

    // --- Reset for new game ---
    void reset();

private:
    struct Slot {
        WeaponType type = WeaponType::MagicWand;
        int level = 0; // 0 = empty slot
        float cooldown = 0.f;
        float orbitBaseAngle = 0.f; // for Axe: starting angle offset
    };

    Slot m_slots[MAX_SLOTS];

    // --- Per-weapon fire helpers ---
    void fireWeapon(int slotIdx, const PlayerState& player, Pool<Enemy>& enemies,
                    Pool<Projectile>& projectiles);
    void fireMagicWand(int slotIdx, const PlayerState& player, Pool<Enemy>& enemies,
                       Pool<Projectile>& proj);
    void fireKnife(int slotIdx, const PlayerState& player, Pool<Enemy>& enemies,
                   Pool<Projectile>& proj);
    void fireAxe(int slotIdx, const PlayerState& player, Pool<Enemy>& enemies,
                 Pool<Projectile>& proj);
    void fireFireball(int slotIdx, const PlayerState& player, Pool<Enemy>& enemies,
                      Pool<Projectile>& proj);
    void tickGarlic(int slotIdx, const PlayerState& player, Pool<Enemy>& enemies);

    // --- Targeting ---
    const Enemy* findNearestEnemy(sf::Vector2f from, float maxRange,
                                  const Pool<Enemy>& enemies) const;
};
