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

/// 主游戏场景，编排实体、武器、生成、渲染、升级与死亡流程。
class PlayScene : public Scene {
public:
    /// 加载默认地图、资源和玩家初始状态。
    explicit PlayScene(Game& game);

    /// 处理游戏、暂停、升级和死亡状态下的输入。
    void handleEvent(const sf::Event& event) override;
    /// 推进一帧固定步长的游戏逻辑。
    void update(sf::Time dt) override;
    /// 绘制世界、HUD 与各类覆盖层。
    void render(sf::RenderWindow& window) override;

private:
    /// 用于资源访问、窗口操作和场景切换的游戏实例。
    Game& m_game;

    /// 存放当前存活敌人的对象池。
    Pool<Enemy> m_enemies;
    /// 存放当前飞行或驻留弹幕的对象池。
    Pool<Projectile> m_projectiles;
    /// 存放未拾取经验宝石的对象池。
    Pool<XPGem> m_xpGems;
    /// 存放短暂伤害飘字的对象池。
    Pool<DamageText> m_damageTexts;

    /// 当前局唯一的玩家状态。
    PlayerState m_player;

    /// 当前地图的数据定义。
    const MapDef* m_map = nullptr;
    /// 从 TMX 文件读取的世界像素宽度。
    float m_worldWidth = 0.f;
    /// 从 TMX 文件读取的世界像素高度。
    float m_worldHeight = 0.f;

    /// 处理武器槽、冷却和攻击行为的系统。
    WeaponSystem m_weapons;
    /// 负责波次、难度和 Boss 生成的系统。
    SpawningSystem m_spawning;
    /// 本局的音效播放对象池。
    SoundPlayer m_sounds;
    /// 地图背景音乐播放器。
    sf::Music m_bgm;
    /// 绘制世界实体和伤害飘字的渲染器。
    WorldRenderer m_worldRenderer;
    /// 加载并绘制 TMX 地图的渲染器。
    TilemapRenderer m_tilemap;
    /// 屏幕空间状态栏；字体加载失败时为空。
    std::unique_ptr<HUD> m_hud;

    /// 敌人类型数量，用于静态精灵表数组。
    static constexpr std::size_t K_ENEMY_TYPE_COUNT = static_cast<std::size_t>(EnemyType::Count);
    /// 按 EnemyType 索引的敌人移动精灵表。
    std::array<SpriteSheet, K_ENEMY_TYPE_COUNT> m_spritesMove;
    /// 按 EnemyType 索引的敌人受击精灵表。
    std::array<SpriteSheet, K_ENEMY_TYPE_COUNT> m_spritesDamaged;

    /// 玩家精灵表的数组索引；横向动作通过翻转复用右朝向素材。
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
    /// 按 PlayerSpriteIdx 索引的玩家动画精灵表。
    std::array<SpriteSheet, KCount> m_playerSprites;

    /// 攻击精灵表完整播放一次的时长。
    float m_attackAnimDuration = 0.f;
    /// 受击精灵表完整播放一次的时长。
    float m_hitAnimDuration = 0.f;
    /// 死亡精灵表完整播放一次的时长。
    float m_deathAnimDuration = 0.f;

    /// 跟随玩家并受地图边界限制的世界相机。
    sf::View m_camera;
    /// 用于世界飘字和死亡覆盖层的默认字体。
    const sf::Font* m_font = nullptr;

    /// 本局未暂停的累计游戏时长（秒）。
    float m_gameTime = 0.f;
    /// 本局击杀敌人的数量。
    int m_score = 0;
    /// 是否因升级选择而暂停。
    bool m_paused = false;
    /// 是否因 Escape 菜单而暂停。
    bool m_menuPaused = false;

    /// 当前死亡序列阶段。
    DeathPhase m_deathPhase = DeathPhase::None;
    /// 死亡动画阶段的剩余时间。
    float m_deathAnimTimer = 0.f;
    /// 死亡瞬间相机的初始视口尺寸。
    sf::Vector2f m_deathCameraInitialSize;
    /// 死亡镜头缩放后的目标视口尺寸。
    sf::Vector2f m_deathCameraTargetSize;
    /// 冻结阶段相机的目标中心，带有构图偏移。
    sf::Vector2f m_deathCameraTargetCenter;

    /// 当前升级暂停时展示的可选项。
    std::vector<UpgradeOption> m_upgradeOptions;
    /// 键盘当前选中的升级或菜单选项索引。
    int m_selectedOption = 0;

    /// 读取键盘输入并更新玩家移动方向。
    void handleInput();
    /// 按当前速度移动玩家并钳制在地图边界内。
    void movePlayer(float dt);
    /// 更新敌人移动、冻结和动画状态。
    void updateEnemies(float dt);
    /// 更新弹幕运动、寿命和命中前状态。
    void updateProjectiles(float dt);
    /// 更新经验宝石的磁吸与移动状态。
    void updateXPGems(float dt);
    /// 更新伤害飘字的位置和生命周期。
    void updateDamageTexts(float dt);
    /// 让相机跟随玩家并限制在地图范围内。
    void updateCamera();
    /// 选择并推进玩家当前动作的动画帧。
    void updatePlayerAnimation(float dt);
    /// 提升玩家等级并生成升级选项。
    void onLevelUp();

    /// 进入死亡动画并初始化镜头过渡状态。
    void beginDeath();
    /// 推进死亡精灵和镜头缩放动画。
    void updateDeathAnimation(float dt);
    /// 绘制死亡冻结阶段的暗幕和结算信息。
    void renderDeathOverlay(sf::RenderWindow& window);
};
