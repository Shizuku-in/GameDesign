#pragma once

#include <SFML/System/Vector2.hpp>

#include <cstdint>

// --- Enemy type ---
enum class EnemyType : std::uint8_t { Basic, Fast, Tank, Boss, Count };

// --- Enemy instance (stored in Pool<Enemy>) ---
struct Enemy {
    sf::Vector2f pos;
    sf::Vector2f vel; // computed each frame toward player
    float hp = 0.f;
    float maxHp = 0.f;
    float speed = 0.f;
    float radius = 0.f;
    float damage = 0.f; // contact damage per second
    float xpValue = 0.f;
    EnemyType type = EnemyType::Basic;
};

// --- Projectile instance (stored in Pool<Projectile>) ---
struct Projectile {
    sf::Vector2f pos;
    sf::Vector2f vel; // normalized direction × speed (unused when orbiting)
    float damage = 0.f;
    float speed = 0.f;
    float lifetime = 0.f; // seconds remaining
    float radius = 0.f;
    int pierceCount = 0; // remaining pierces (0 = dies on first hit)

    // Orbit state (orbitRadius > 0 → projectile orbits player instead of moving by vel)
    float orbitAngle = 0.f;  // current angle in radians
    float orbitRadius = 0.f; // distance from player; 0 = not orbiting
    float orbitSpeed = 0.f;  // radians per second
};

// --- XP gem instance (stored in Pool<XPGem>) ---
struct XPGem {
    sf::Vector2f pos;
    float value = 0.f;
    float radius = 5.f;
    float magnetTimer = 0.f; // seconds until magnet attraction activates
};
