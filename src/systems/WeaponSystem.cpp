#include "systems/WeaponSystem.hpp"
#include "math/Collision.hpp"
#include "audio/SoundPlayer.hpp"
#include "gameplay/WeaponFactory.hpp"

#include <cmath>
#include <limits>

WeaponSystem::WeaponSystem() { reset(); }

// --- 槽位管理 ---

bool WeaponSystem::addWeapon(WeaponType type) {
    if (hasWeapon(type))
        return false;
    for (auto& slot : m_slots) {
        if (slot.level == 0) {
            auto newBehavior = WeaponFactory::create(type);
            if (!newBehavior)
                return false;

            slot.type = type;
            slot.level = 1;
            slot.cooldown = 0.f; // 立即开火
            slot.behavior = std::move(newBehavior);
            return true;
        }
    }
    return false; // 所有槽位已满
}

bool WeaponSystem::upgradeWeapon(WeaponType type) {
    for (auto& slot : m_slots) {
        if (slot.level > 0 && slot.type == type) {
            const auto& def = WEAPON_DEFS[static_cast<int>(type)];
            if (slot.level >= def.maxLevel)
                return false;
            ++slot.level;
            return true;
        }
    }
    return false; // 未拥有此武器
}

bool WeaponSystem::hasWeapon(WeaponType type) const {
    for (const auto& slot : m_slots) {
        if (slot.level > 0 && slot.type == type)
            return true;
    }
    return false;
}

int WeaponSystem::getLevel(WeaponType type) const {
    for (const auto& slot : m_slots) {
        if (slot.level > 0 && slot.type == type)
            return slot.level;
    }
    return 0;
}

bool WeaponSystem::isFull() const {
    for (const auto& slot : m_slots) {
        if (slot.level == 0)
            return false;
    }
    return true;
}

int WeaponSystem::emptySlotCount() const {
    int n = 0;
    for (const auto& slot : m_slots) {
        if (slot.level == 0)
            ++n;
    }
    return n;
}

std::vector<WeaponType> WeaponSystem::getUpgradeableWeapons() const {
    std::vector<WeaponType> result;
    for (const auto& slot : m_slots) {
        if (slot.level > 0) {
            const auto& def = WEAPON_DEFS[static_cast<int>(slot.type)];
            if (slot.level < def.maxLevel)
                result.push_back(slot.type);
        }
    }
    return result;
}

void WeaponSystem::reset() {
    for (auto& slot : m_slots) {
        slot.type = WeaponType::MagicWand;
        slot.level = 0;
        slot.cooldown = 0.f;
        slot.behavior = nullptr;
    }
    // 初始携带 MagicWand 1 级
    addWeapon(WeaponType::MagicWand);
}

// --- 主更新 ---

void WeaponSystem::update(float dt, const PlayerState& player, Pool<Enemy>& enemies,
                          Pool<Projectile>& projectiles, SoundPlayer& sounds) {
    for (int i = 0; i < MAX_SLOTS; ++i) {
        auto& slot = m_slots[i];
        if (slot.level == 0)
            continue;

        const auto& def = WEAPON_DEFS[static_cast<int>(slot.type)];
        slot.cooldown -= dt;

        // AoE 武器冷却完毕时每帧造成伤害
        if (def.isAOE) {
            if (slot.cooldown <= 0.f) {
                auto stats = getWeaponStats(slot.type, slot.level);
                slot.cooldown = stats.cooldown;
                if (slot.behavior) {
                    slot.behavior->tickAoE(slot.level, player, enemies);
                }
            }
            continue;
        }

        // 弹幕武器：冷却完毕时发射
        if (slot.cooldown <= 0.f) {
            bool fired = false;
            if (slot.behavior) {
                fired = slot.behavior->fire(slot.level, player, enemies, projectiles);
            }
            if (fired)
                sounds.shoot();
            auto stats = getWeaponStats(slot.type, slot.level);
            slot.cooldown = stats.cooldown;
        }
    }
}
