#pragma once

#include <SFML/System/Vector2.hpp>

namespace Config {

// --- World ---
constexpr float WORLD_WIDTH = 1600.f;
constexpr float WORLD_HEIGHT = 1200.f;

// --- Player ---
constexpr float PLAYER_RADIUS = 16.f;
constexpr float PLAYER_SPEED = 220.f;
constexpr float PLAYER_MAX_HP = 100.f;
constexpr float PLAYER_ARMOR = 0.f;    // 0–1 damage reduction
constexpr float PLAYER_MAGNET = 80.f;  // gem pickup radius
constexpr float PLAYER_IFRAMES = 0.5f; // invincibility after hit
constexpr int PLAYER_MAX_WEAPONS = 6;

// --- Enemies ---
constexpr float ENEMY_SPAWN_DISTANCE = 600.f; // min distance from player on spawn
constexpr float ENEMY_BASE_SPAWN_INTERVAL = 3.f;
constexpr float ENEMY_MIN_SPAWN_INTERVAL = 0.5f;
constexpr int ENEMIES_PER_WAVE_BASE = 3;

// --- XP ---
constexpr float XP_BASE_THRESHOLD = 10.f;
constexpr float XP_THRESHOLD_GROWTH = 5.f;  // added per level
constexpr float XP_GEM_MAGNET_DELAY = 1.5f; // seconds before magnet pull
constexpr float XP_GEM_MAGNET_SPEED = 400.f;

// --- Camera / View ---
constexpr float VIEW_WIDTH = 800.f;
constexpr float VIEW_HEIGHT = 600.f;

// --- Enemy type stat tables ---
// Ordered by EnemyType enum: Basic, Fast, Tank, Boss
constexpr float ENEMY_HP[] = {20.f, 10.f, 80.f, 300.f};
constexpr float ENEMY_SPEED[] = {80.f, 160.f, 50.f, 60.f};
constexpr float ENEMY_DAMAGE[] = {10.f, 8.f, 20.f, 30.f};
constexpr float ENEMY_RADIUS[] = {14.f, 10.f, 22.f, 32.f};
constexpr float ENEMY_XP[] = {1.f, 2.f, 5.f, 50.f};
constexpr float ENEMY_SPAWN_WEIGHT[] = {1.f, 0.7f, 0.4f, 0.f}; // Boss spawned separately

// --- Enemy type appearance times (seconds) ---
constexpr float ENEMY_APPEAR_TIME[] = {0.f, 30.f, 60.f, -1.f}; // -1 = never by weight

} // namespace Config
