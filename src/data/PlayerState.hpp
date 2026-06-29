#pragma once

#include "data/Constants.hpp"

#include <SFML/System/Vector2.hpp>

/// Singleton player state — not stored in a pool.
struct PlayerState {
    sf::Vector2f pos{Config::WORLD_WIDTH / 2.f, Config::WORLD_HEIGHT / 2.f};
    sf::Vector2f vel; // input direction (normalized), computed each frame

    float speed = Config::PLAYER_SPEED;
    float hp = Config::PLAYER_MAX_HP;
    float maxHp = Config::PLAYER_MAX_HP;
    float radius = Config::PLAYER_RADIUS;

    float armor = Config::PLAYER_ARMOR;        // 0–1 damage reduction
    float magnetRange = Config::PLAYER_MAGNET; // gem pickup range before magnet
    float xpMultiplier = 1.0f;

    int level = 1;
    float xp = 0.f;
    float xpToNext = Config::XP_BASE_THRESHOLD;

    float invincibilityTimer = 0.f; // > 0 → cannot take damage
};
