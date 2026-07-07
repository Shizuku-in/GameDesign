#include "systems/WeaponBehaviors.hpp"

#include "data/Constants.hpp"
#include "math/Collision.hpp"

#include <cmath>
#include <limits>

using enum WeaponType;

namespace {

const Enemy* findNearestEnemy(sf::Vector2f from, float maxRange, const Pool<Enemy>& enemies) {
    const Enemy* best = nullptr;
    float bestDistSq = maxRange > 0.f ? maxRange * maxRange : std::numeric_limits<float>::max();

    enemies.forEach([&](const Enemy& e) {
        if (e.hp <= 0.f) { // 忽略已被击杀但尚未清理的敌人
            return;
        }
        float d2 = distanceSq(from, e.pos);
        if (d2 < bestDistSq) {
            bestDistSq = d2;
            best = &e;
        }
    });
    return best;
}

} // namespace

bool MagicWandBehavior::fire(int level, const PlayerState& player, Pool<Enemy>& enemies,
                             Pool<Projectile>& proj) {
    auto stats = getWeaponStats(MagicWand, level);

    const auto* target = findNearestEnemy(player.pos, stats.range, enemies);
    if (target == nullptr) {
        return false;
    }

    sf::Vector2f dir = target->pos - player.pos;
    float len = std::sqrt((dir.x * dir.x) + (dir.y * dir.y));
    if (len > 0.f) {
        dir.x /= len;
        dir.y /= len;
    }

    auto handle = proj.acquire();
    auto* p = proj.get(handle);
    if (p == nullptr) {
        return false;
    }
    p->pos = player.pos;
    p->vel = dir * stats.projectileSpeed;
    p->damage = stats.damage * (1.0f + player.damageBonus);
    p->speed = stats.projectileSpeed;
    p->lifetime = stats.projectileLifetime;
    p->radius = stats.projectileRadius;
    p->pierceCount = stats.pierce;
    p->motion = ProjMotion::Linear;
    return true;
}

bool KnifeBehavior::fire(int level, const PlayerState& player, Pool<Enemy>& enemies,
                         Pool<Projectile>& proj) {
    auto stats = getWeaponStats(Knife, level);

    sf::Vector2f dir = {1.f, 0.f};
    const auto* target = findNearestEnemy(player.pos, Config::RANGE_UNLIMITED, enemies);
    if (target != nullptr) {
        dir = target->pos - player.pos;
        float len = std::sqrt((dir.x * dir.x) + (dir.y * dir.y));
        if (len > 0.f) {
            dir.x /= len;
            dir.y /= len;
        }
    }

    int count = stats.projectileCount;
    const auto& knifeDef = WEAPON_DEFS[static_cast<int>(Knife)];
    float spread = (count - 1) * knifeDef.spread;
    float baseAngle = std::atan2(dir.y, dir.x);
    float startAngle = baseAngle - (spread / 2.f);
    bool anySpawned = false;

    for (int i = 0; i < count; ++i) {
        float angle = startAngle + (count > 1 ? spread * static_cast<float>(i) / (count - 1) : 0.f);
        sf::Vector2f d(std::cos(angle), std::sin(angle));

        auto handle = proj.acquire();
        auto* p = proj.get(handle);
        if (p == nullptr) {
            continue;
        }
        p->pos = player.pos;
        p->vel = d * stats.projectileSpeed;
        p->damage = stats.damage * (1.0f + player.damageBonus);
        p->speed = stats.projectileSpeed;
        p->lifetime = stats.projectileLifetime;
        p->radius = stats.projectileRadius;
        p->pierceCount = stats.pierce;
        p->motion = ProjMotion::Linear;
        anySpawned = true;
    }
    return anySpawned;
}

bool AxeBehavior::fire(int level, const PlayerState& player, Pool<Enemy>& /*enemies*/,
                       Pool<Projectile>& proj) {
    auto stats = getWeaponStats(Axe, level);
    const auto& axeDef = WEAPON_DEFS[static_cast<int>(Axe)];

    int count = stats.projectileCount;
    if (count <= 0) {
        return false;
    }

    float angleStep = Config::TAU / static_cast<float>(count);
    bool anySpawned = false;

    for (int i = 0; i < count; ++i) {
        float angle = m_orbitBaseAngle + (angleStep * static_cast<float>(i));

        auto handle = proj.acquire();
        auto* p = proj.get(handle);
        if (p == nullptr) {
            continue;
        }

        p->motion = ProjMotion::Orbit;
        p->state.orbit.angle = angle;
        p->state.orbit.radius = axeDef.orbitRadius;
        p->state.orbit.speed = axeDef.orbitSpeed;
        p->pos =
            player.pos + sf::Vector2f(std::cos(angle), std::sin(angle)) * p->state.orbit.radius;
        p->damage = stats.damage * (1.0f + player.damageBonus);
        p->speed = 0.f;
        p->lifetime = stats.projectileLifetime;
        p->radius = stats.projectileRadius;
        p->pierceCount = stats.pierce;
        anySpawned = true;
    }

    // 推进基准角度供下次发射（实现无缝衔接）
    m_orbitBaseAngle =
        std::fmod(m_orbitBaseAngle + (stats.cooldown * axeDef.orbitSpeed), Config::TAU);

    return anySpawned;
}

bool FireballBehavior::fire(int level, const PlayerState& player, Pool<Enemy>& enemies,
                            Pool<Projectile>& proj) {
    auto stats = getWeaponStats(Fireball, level);

    const auto* target = findNearestEnemy(player.pos, stats.range, enemies);
    if (target == nullptr) {
        return false;
    }

    sf::Vector2f dir = target->pos - player.pos;
    float len = std::sqrt((dir.x * dir.x) + (dir.y * dir.y));
    if (len > 0.f) {
        dir.x /= len;
        dir.y /= len;
    }

    auto handle = proj.acquire();
    auto* p = proj.get(handle);
    if (p == nullptr) {
        return false;
    }
    p->pos = player.pos;
    p->vel = dir * stats.projectileSpeed;
    p->damage = stats.damage * (1.0f + player.damageBonus);
    p->speed = stats.projectileSpeed;
    p->lifetime = stats.projectileLifetime;
    p->radius = stats.projectileRadius;
    p->pierceCount = 0; // 火球首次命中即消失
    p->aoeRadius = stats.aoeRadius;
    p->motion = ProjMotion::Linear;
    return true;
}

void GarlicBehavior::tickAoE(int level, const PlayerState& player, Pool<Enemy>& enemies) {
    auto stats = getWeaponStats(Garlic, level);

    enemies.forEach([&](Enemy& e) {
        if (circleCircle(player.pos, stats.aoeRadius, e.pos, e.radius)) {
            e.hp -= stats.damage * (1.0f + player.damageBonus);
        }
    });
}

void TimeStopBehavior::tickAoE(int level, const PlayerState& player, Pool<Enemy>& enemies) {
    auto stats = getWeaponStats(WeaponType::TimeStop, level);
    const float freezeDuration = 0.1f; // 每次tick增加的冻结时间

    enemies.forEach([&](Enemy& e) {
        if (circleCircle(player.pos, stats.aoeRadius, e.pos, e.radius)) {
            // 冻结敌人，增加冻结时间
            e.frozenTimer = std::max(e.frozenTimer, freezeDuration);
        }
    });
}
