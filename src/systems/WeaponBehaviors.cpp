#include "systems/WeaponBehaviors.hpp"
#include "data/Constants.hpp"
#include "gameplay/WeaponFactory.hpp"
#include "math/Collision.hpp"
#include <cmath>
#include <limits>

namespace {

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

const std::array<WeaponFactory::FactoryFn, WeaponFactory::kCount> WeaponFactory::s_factories = {
    []() -> std::unique_ptr<IWeaponBehavior> { return std::make_unique<MagicWandBehavior>(); },
    []() -> std::unique_ptr<IWeaponBehavior> { return std::make_unique<KnifeBehavior>(); },
    []() -> std::unique_ptr<IWeaponBehavior> { return std::make_unique<AxeBehavior>(); },
    []() -> std::unique_ptr<IWeaponBehavior> { return std::make_unique<FireballBehavior>(); },
    []() -> std::unique_ptr<IWeaponBehavior> { return std::make_unique<GarlicBehavior>(); },
};

std::unique_ptr<IWeaponBehavior> WeaponFactory::create(WeaponType type) {
    auto idx = static_cast<std::size_t>(type);
    if (idx >= kCount)
        return nullptr;
    return s_factories[idx]();
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

    float angleStep = Config::TAU / static_cast<float>(count);
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

    // 推进基准角度供下次发射（实现无缝衔接）
    m_orbitBaseAngle =
        std::fmod(m_orbitBaseAngle + stats.cooldown * Config::AXE_ORBIT_SPEED, Config::TAU);

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
