#include "systems/WeaponSystem.hpp"
#include "data/Collision.hpp"
#include "systems/SoundPlayer.hpp"

#include <cmath>
#include <limits>

WeaponSystem::WeaponSystem() { reset(); }

// --- 槽位管理 ---

bool WeaponSystem::addWeapon(WeaponType type) {
    if (hasWeapon(type))
        return false;
    for (auto& slot : m_slots) {
        if (slot.level == 0) {
            slot.type = type;
            slot.level = 1;
            slot.cooldown = 0.f; // 立即开火
            slot.orbitBaseAngle = 0.f;
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
        slot.orbitBaseAngle = 0.f;
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
                tickGarlic(i, player, enemies);
            }
            continue;
        }

        // 弹幕武器：冷却完毕时发射
        if (slot.cooldown <= 0.f) {
            bool fired = fireWeapon(i, player, enemies, projectiles);
            if (fired)
                sounds.shoot();
            auto stats = getWeaponStats(slot.type, slot.level);
            slot.cooldown = stats.cooldown;
        }
    }
}

// --- 索敌 ---

const Enemy* WeaponSystem::findNearestEnemy(sf::Vector2f from, float maxRange,
                                            const Pool<Enemy>& enemies) const {
    const Enemy* best = nullptr;
    float bestDistSq = maxRange > 0.f ? maxRange * maxRange : std::numeric_limits<float>::max();

    enemies.forEach([&](const Enemy& e) {
        float d2 = distanceSq(from, e.pos);
        if (d2 < bestDistSq) {
            bestDistSq = d2;
            best = &e;
        }
    });
    return best;
}

// --- 发射分发 ---

bool WeaponSystem::fireWeapon(int slotIdx, const PlayerState& player, Pool<Enemy>& enemies,
                              Pool<Projectile>& proj) {
    switch (m_slots[slotIdx].type) {
    case WeaponType::MagicWand:
        return fireMagicWand(slotIdx, player, enemies, proj);
    case WeaponType::Knife:
        return fireKnife(slotIdx, player, enemies, proj);
    case WeaponType::Axe:
        return fireAxe(slotIdx, player, enemies, proj);
    case WeaponType::Fireball:
        return fireFireball(slotIdx, player, enemies, proj);
    case WeaponType::Garlic:
        return false; // 在 update() 的 AoE 路径处理
    default:
        return false;
    }
}

// --- 各武器发射实现 ---

bool WeaponSystem::fireMagicWand(int slotIdx, const PlayerState& player, Pool<Enemy>& enemies,
                                 Pool<Projectile>& proj) {
    auto& slot = m_slots[slotIdx];
    auto stats = getWeaponStats(slot.type, slot.level);

    const auto* target = findNearestEnemy(player.pos, stats.range, enemies);
    if (!target)
        return false;

    sf::Vector2f dir = target->pos - player.pos;
    float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    if (len > 0.f) {
        dir.x /= len;
        dir.y /= len;
    }

    auto handle = proj.acquire();
    auto* p = proj.get(handle);
    if (!p)
        return false;
    p->pos = player.pos;
    p->vel = dir * stats.projectileSpeed;
    p->damage = stats.damage;
    p->speed = stats.projectileSpeed;
    p->lifetime = stats.projectileLifetime;
    p->radius = stats.projectileRadius;
    p->pierceCount = stats.pierce;
    return true;
}

bool WeaponSystem::fireKnife(int slotIdx, const PlayerState& player, Pool<Enemy>& enemies,
                             Pool<Projectile>& proj) {
    auto& slot = m_slots[slotIdx];
    auto stats = getWeaponStats(slot.type, slot.level);

    sf::Vector2f dir = {1.f, 0.f};
    const auto* target = findNearestEnemy(player.pos, 0.f /*无限制*/, enemies);
    if (target) {
        dir = target->pos - player.pos;
        float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        if (len > 0.f) {
            dir.x /= len;
            dir.y /= len;
        }
    }

    int count = stats.projectileCount;
    float spread = (count - 1) * 0.15f;
    float baseAngle = std::atan2(dir.y, dir.x);
    float startAngle = baseAngle - spread / 2.f;
    bool anySpawned = false;

    for (int i = 0; i < count; ++i) {
        float angle = startAngle + (count > 1 ? spread * static_cast<float>(i) / (count - 1) : 0.f);
        sf::Vector2f d(std::cos(angle), std::sin(angle));

        auto handle = proj.acquire();
        auto* p = proj.get(handle);
        if (!p)
            continue;
        p->pos = player.pos;
        p->vel = d * stats.projectileSpeed;
        p->damage = stats.damage;
        p->speed = stats.projectileSpeed;
        p->lifetime = stats.projectileLifetime;
        p->radius = stats.projectileRadius;
        p->pierceCount = stats.pierce;
        anySpawned = true;
    }
    return anySpawned;
}

bool WeaponSystem::fireAxe(int slotIdx, const PlayerState& player, Pool<Enemy>& /*enemies*/,
                           Pool<Projectile>& proj) {
    auto& slot = m_slots[slotIdx];
    auto stats = getWeaponStats(slot.type, slot.level);

    int count = stats.projectileCount;
    float angleStep = 2.f * 3.14159265f / static_cast<float>(count);
    bool anySpawned = false;

    for (int i = 0; i < count; ++i) {
        float angle = slot.orbitBaseAngle + angleStep * static_cast<float>(i);

        auto handle = proj.acquire();
        auto* p = proj.get(handle);
        if (!p)
            continue;

        p->orbitAngle = angle;
        p->orbitRadius = 60.f;
        p->orbitSpeed = 3.0f;
        p->pos = player.pos + sf::Vector2f(std::cos(angle), std::sin(angle)) * p->orbitRadius;
        p->damage = stats.damage;
        p->speed = 0.f;
        p->lifetime = stats.projectileLifetime;
        p->radius = stats.projectileRadius;
        p->pierceCount = stats.pierce;
        anySpawned = true;
    }

    // 推进基准角度供下次发射
    slot.orbitBaseAngle += 1.0f;
    if (slot.orbitBaseAngle > 2.f * 3.14159265f)
        slot.orbitBaseAngle -= 2.f * 3.14159265f;

    return anySpawned;
}

bool WeaponSystem::fireFireball(int slotIdx, const PlayerState& player, Pool<Enemy>& enemies,
                                Pool<Projectile>& proj) {
    auto& slot = m_slots[slotIdx];
    auto stats = getWeaponStats(slot.type, slot.level);

    const auto* target = findNearestEnemy(player.pos, stats.range, enemies);
    if (!target)
        return false;

    sf::Vector2f dir = target->pos - player.pos;
    float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    if (len > 0.f) {
        dir.x /= len;
        dir.y /= len;
    }

    auto handle = proj.acquire();
    auto* p = proj.get(handle);
    if (!p)
        return false;
    p->pos = player.pos;
    p->vel = dir * stats.projectileSpeed;
    p->damage = stats.damage;
    p->speed = stats.projectileSpeed;
    p->lifetime = stats.projectileLifetime;
    p->radius = stats.projectileRadius;
    p->pierceCount = 0; // 火球首次命中即消失
    return true;
}

void WeaponSystem::tickGarlic(int slotIdx, const PlayerState& player, Pool<Enemy>& enemies) {
    auto& slot = m_slots[slotIdx];
    auto stats = getWeaponStats(slot.type, slot.level);

    enemies.forEach([&](Enemy& e) {
        if (circleCircle(player.pos, stats.aoeRadius, e.pos, e.radius)) {
            e.hp -= stats.damage;
        }
    });
}
