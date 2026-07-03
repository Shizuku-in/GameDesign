#pragma once

#include "audio/SoundPlayer.hpp"
#include "core/Pool.hpp"
#include "core/Scene.hpp"
#include "data/EntityTypes.hpp"
#include "data/PlayerState.hpp"
#include "gameplay/MapDefs.hpp"
#include "gameplay/UpgradeDefs.hpp"
#include "graphics/SpriteSheet.hpp"
#include "graphics/TilemapRenderer.hpp"
#include "graphics/WorldRenderer.hpp"
#include "systems/SpawningSystem.hpp"
#include "systems/WeaponSystem.hpp"
#include "ui/HUD.hpp"

#include <SFML/Audio/Music.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/View.hpp>

#include <array>
#include <memory>
#include <vector>

class Game;

/// 死亡阶段：控制死亡动画 → 冻结 → 遮罩的状态机
enum class DeathPhase : std::uint8_t { None, Animation, Frozen };

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

    // 地图配置
    const MapDef* m_map = nullptr;
    float m_worldWidth = 0.f; // 从 TMX 自动读取
    float m_worldHeight = 0.f;

    // 子系统
    WeaponSystem m_weapons;
    SpawningSystem m_spawning;
    SoundPlayer m_sounds;
    sf::Music m_bgm;
    WorldRenderer m_worldRenderer;
    TilemapRenderer m_tilemap;
    std::unique_ptr<HUD> m_hud;

    // 敌人精灵表（按 EnemyType 索引）
    static constexpr std::size_t K_ENEMY_TYPE_COUNT = static_cast<std::size_t>(EnemyType::Count);
    std::array<SpriteSheet, K_ENEMY_TYPE_COUNT> m_spritesMove;
    std::array<SpriteSheet, K_ENEMY_TYPE_COUNT> m_spritesDamaged;

    // 角色精灵表索引（仅保留右朝向，左朝向通过翻转实现）
    enum PlayerSpriteIdx : std::uint8_t {
        KForward,
        KBack,
        KSide,
        KIdle,
        KAttack,
        KHit,
        KDeath,
        // 移动中攻击/受击变体
        KMovingAttackForward,
        KMovingAttackBack,
        KMovingAttackSide,
        KMovingHitBack,
        KMovingHitSide,
        KCount
    };
    std::array<SpriteSheet, KCount> m_playerSprites;

    // 攻击/受击/死亡动画时长（加载精灵后计算）
    float m_attackAnimDuration = 0.f;
    float m_hitAnimDuration = 0.f;
    float m_deathAnimDuration = 0.f;

    // 相机
    sf::View m_camera;
    const sf::Font* m_font = nullptr;

    // 游戏状态
    float m_gameTime = 0.f;
    int m_score = 0;
    bool m_paused = false;     // 升级暂停
    bool m_menuPaused = false; // 菜单暂停 (Escape)

    // 死亡动画状态机
    DeathPhase m_deathPhase = DeathPhase::None;
    float m_deathAnimTimer = 0.f;          // 死亡动画阶段剩余时间（倒数计时）
    sf::Vector2f m_deathCameraInitialSize; // 死亡瞬间的相机视口尺寸
    sf::Vector2f m_deathCameraTargetSize;  // 缩放目标视口尺寸（4× 放大）
    sf::Vector2f m_deathCameraTargetCenter; // 冻结阶段相机中心（偏移使角色在上半部）

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
    void updatePlayerAnimation(float dt);
    void onLevelUp();

    // 死亡动画序列
    void beginDeath();
    void updateDeathAnimation(float dt);
    void renderDeathOverlay(sf::RenderWindow& window);
};
