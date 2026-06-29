#include "systems/WeaponBehaviors.hpp"
#include "math/Collision.hpp"
#include "data/Constants.hpp"
#include "gameplay/WeaponFactory.hpp"
#include <cmath>
#include <limits>

namespace {

constexpr float PI = 3.14159265f;

const Enemy* findNearestEnemy(sf::Vector2f from, float maxRange, const Pool<Enemy>& enemies) {
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

} // namespace

std::unique_ptr<IWeaponBehavior> WeaponFactory::create(WeaponType type) {
    switch (type) {
    case WeaponType::MagicWand:
        return std::make_unique<MagicWandBehavior>();
    case WeaponType::Knife:
        return std::make_unique<KnifeBehavior>();
    case WeaponType::Axe:
        return std::make_unique<AxeBehavior>();
    case WeaponType::Fireball:
        return std::make_unique<FireballBehavior>();
    case WeaponType::Garlic:
        return std::make_unique<GarlicBehavior>();
    default:
        return nullptr;
    }
}

bool MagicWandBehavior::fire(int level, const PlayerState& player, Pool<Enemy>& enemies,
                             Pool<Projectile>& proj) {
    auto stats = getWeaponStats(WeaponType::MagicWand, level);

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
    p->motion = ProjMotion::Linear;
    return true;
}

bool KnifeBehavior::fire(int level, const PlayerState& player, Pool<Enemy>& enemies,
                         Pool<Projectile>& proj) {
    auto stats = getWeaponStats(WeaponType::Knife, level);

    sf::Vector2f dir = {1.f, 0.f};
    const auto* target = findNearestEnemy(player.pos, Config::RANGE_UNLIMITED, enemies);
    if (target) {
        dir = target->pos - player.pos;
        float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        if (len > 0.f) {
            dir.x /= len;
            dir.y /= len;
        }
    }

    int count = stats.projectileCount;
    float spread = (count - 1) * Config::KNIFE_SPREAD;
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
        p->motion = ProjMotion::Linear;
        anySpawned = true;
    }
    return anySpawned;
}

bool AxeBehavior::fire(int level, const PlayerState& player, Pool<Enemy>& /*enemies*/,
                       Pool<Projectile>& proj) {
    auto stats = getWeaponStats(WeaponType::Axe, level);

    int count = stats.projectileCount;
    if (count <= 0)
        return false;

    float angleStep = 2.f * PI / static_cast<float>(count);
    bool anySpawned = false;

    for (int i = 0; i < count; ++i) {
        float angle = m_orbitBaseAngle + angleStep * static_cast<float>(i);

        auto handle = proj.acquire();
        auto* p = proj.get(handle);
        if (!p)
            continue;

        p->motion = ProjMotion::Orbit;
        p->state.orbit.angle = angle;
        p->state.orbit.radius = Config::AXE_ORBIT_RADIUS;
        p->state.orbit.speed = Config::AXE_ORBIT_SPEED;
        p->pos =
            player.pos + sf::Vector2f(std::cos(angle), std::sin(angle)) * p->state.orbit.radius;
        p->damage = stats.damage;
        p->speed = 0.f;
        p->lifetime = stats.projectileLifetime;
        p->radius = stats.projectileRadius;
        p->pierceCount = stats.pierce;
        anySpawned = true;
    }

    // 推进基准角度供下次发射
    m_orbitBaseAngle = std::fmod(m_orbitBaseAngle + 1.0f, 2.f * PI);

    return anySpawned;
}

bool FireballBehavior::fire(int level, const PlayerState& player, Pool<Enemy>& enemies,
                            Pool<Projectile>& proj) {
    auto stats = getWeaponStats(WeaponType::Fireball, level);

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
    p->motion = ProjMotion::Linear;
    return true;
}

void GarlicBehavior::tickAoE(int level, const PlayerState& player, Pool<Enemy>& enemies) {
    auto stats = getWeaponStats(WeaponType::Garlic, level);

    enemies.forEach([&](Enemy& e) {
        if (circleCircle(player.pos, stats.aoeRadius, e.pos, e.radius)) {
            e.hp -= stats.damage;
        }
    });
}
