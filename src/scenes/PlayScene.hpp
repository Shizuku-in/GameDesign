#pragma once

#include "audio/SoundPlayer.hpp"
#include "core/Pool.hpp"
#include "core/Scene.hpp"
#include "data/EntityTypes.hpp"
#include "data/PlayerState.hpp"
#include "gameplay/UpgradeDefs.hpp"
#include "graphics/WorldRenderer.hpp"
#include "systems/SpawningSystem.hpp"
#include "systems/WeaponSystem.hpp"
#include "ui/HUD.hpp"

#include <SFML/Audio/Music.hpp>
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
    Pool<DamageText> m_damageTexts;

    // 玩家状态（单例）
    PlayerState m_player;

    // 子系统
    WeaponSystem m_weapons;
    SpawningSystem m_spawning;
    SoundPlayer m_sounds;
    sf::Music m_bgm;
    WorldRenderer m_worldRenderer;
    std::unique_ptr<HUD> m_hud;

    // 相机
    sf::View m_camera;
    const sf::Font* m_font = nullptr;

    // 游戏状态
    float m_gameTime = 0.f;
    int m_score = 0;
    bool m_paused = false;     // 升级暂停
    bool m_menuPaused = false; // 菜单暂停 (Escape)
    bool m_gameOver = false;

    // 升级UI状态
    std::vector<UpgradeOption> m_upgradeOptions;
    int m_selectedOption = 0;

    // --- 保留的子步骤 ---
    void handleInput();
    void movePlayer(float dt);
    void updateEnemies(float dt);
    void updateProjectiles(float dt);
    void updateXPGems(float dt);
    void updateDamageTexts(float dt);
    void updateCamera();
    void onLevelUp();
    void onDeath();
};
