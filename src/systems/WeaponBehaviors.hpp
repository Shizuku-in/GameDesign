#pragma once
#include "systems/IWeaponBehavior.hpp"

class MagicWandBehavior : public IWeaponBehavior {
public:
    bool fire(int level, const PlayerState& player, Pool<Enemy>& enemies,
              Pool<Projectile>& proj) override;
};

class KnifeBehavior : public IWeaponBehavior {
public:
    bool fire(int level, const PlayerState& player, Pool<Enemy>& enemies,
              Pool<Projectile>& proj) override;
};

class AxeBehavior : public IWeaponBehavior {
public:
    bool fire(int level, const PlayerState& player, Pool<Enemy>& enemies,
              Pool<Projectile>& proj) override;

private:
    float m_orbitBaseAngle = 0.f; // 封装特定武器状态
};

class FireballBehavior : public IWeaponBehavior {
public:
    bool fire(int level, const PlayerState& player, Pool<Enemy>& enemies,
              Pool<Projectile>& proj) override;
};

class GarlicBehavior : public IWeaponBehavior {
public:
    void tickAoE(int level, const PlayerState& player, Pool<Enemy>& enemies) override;
};
