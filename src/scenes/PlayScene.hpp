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

    // 实体对象池
    Pool<Enemy> m_enemies;
    Pool<Projectile> m_projectiles;
    Pool<XPGem> m_xpGems;

    // 玩家状态（单例）
    PlayerState m_player;

    // 子系统
    WeaponSystem m_weapons;
    std::unique_ptr<HUD> m_hud;

    // 相机（世界空间跟随玩家）
    sf::View m_camera;

    // 字体（来自 Game 的 ResourceManager）
    const sf::Font* m_font = nullptr;

    // 游戏状态
    float m_gameTime = 0.f;
    int m_score = 0;
    bool m_paused = false;
    bool m_gameOver = false;

    // 敌人生成
    float m_spawnTimer = 0.f;
    float m_spawnInterval = 0.f;
    int m_enemiesPerWave = 0;
    float m_difficultyTimer = 0.f; // 随时间增长，控制生成规模
    float m_bossTimer = 0.f;       // Boss 倒计时

    // 升级界面状态
    std::vector<UpgradeOption> m_upgradeOptions;
    int m_selectedOption = 0;

    // --- update() 子步骤 ---
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

    // --- 生成辅助函数 ---
    void spawnEnemy(EnemyType type);
    void spawnGem(sf::Vector2f pos, float value);
    sf::Vector2f randomSpawnPosition() const;

    // --- 绘制辅助函数 ---
    void drawEntities(sf::RenderWindow& window);
    void drawUpgradeUI(sf::RenderWindow& window);
};
