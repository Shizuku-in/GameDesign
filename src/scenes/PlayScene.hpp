#pragma once

#include "core/Pool.hpp"
#include "core/Scene.hpp"
#include "data/EntityTypes.hpp"
#include "data/PlayerState.hpp"
#include "systems/HUD.hpp"
#include "systems/UpgradeDefs.hpp"
#include "systems/WeaponSystem.hpp"

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/View.hpp>

#include <memory>
#include <vector>

class Game;

class PlayScene : public Scene {
public:
    explicit PlayScene(Game& game);

    void handleEvent(const sf::Event& event) override;
    void update(sf::Time dt) override;
    void render(sf::RenderWindow& window) override;

private:
    Game& m_game;

    // Entity pools
    Pool<Enemy> m_enemies;
    Pool<Projectile> m_projectiles;
    Pool<XPGem> m_xpGems;

    // Singleton player state
    PlayerState m_player;

    // Systems
    WeaponSystem m_weapons;
    std::unique_ptr<HUD> m_hud;

    // Camera (follows player in world space)
    sf::View m_camera;

    // Font (from Game's ResourceManager)
    const sf::Font* m_font = nullptr;

    // Game state
    float m_gameTime = 0.f;
    int m_score = 0;
    bool m_paused = false;
    bool m_gameOver = false;

    // Spawning
    float m_spawnTimer = 0.f;
    float m_spawnInterval = 0.f;
    int m_enemiesPerWave = 0;
    float m_difficultyTimer = 0.f; // increases over time, scales spawns
    float m_bossTimer = 0.f;       // countdown to next boss

    // Upgrade UI state
    std::vector<UpgradeOption> m_upgradeOptions;
    int m_selectedOption = 0;

    // --- Sub-steps of update() ---
    void handleInput();
    void movePlayer(float dt);
    void updateEnemies(float dt);
    void updateProjectiles(float dt);
    void updateXPGems(float dt);
    void checkCollisions(float dt);
    void updateSpawning(float dt);
    void updateCamera();
    void onLevelUp();
    void onDeath();

    // --- Spawn helpers ---
    void spawnEnemy(EnemyType type);
    void spawnGem(sf::Vector2f pos, float value);
    sf::Vector2f randomSpawnPosition() const;

    // --- Drawing helpers ---
    void drawEntities(sf::RenderWindow& window);
    void drawUpgradeUI(sf::RenderWindow& window);
};
