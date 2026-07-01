#pragma once

#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>

#include <cstdint>
#include <numbers>

namespace Config {

// --- 玩家（通用，非按角色） ---
constexpr float PLAYER_IFRAMES = 0.5f; // 受击后无敌时间（秒）
constexpr int PLAYER_MAX_WEAPONS = 6;
constexpr float PLAYER_MAX_ARMOR = 0.5f;              // 护甲上限 50%
constexpr float PLAYER_MAX_COOLDOWN_REDUCTION = 0.6f; // 冷却缩减上限 60%

// --- 敌人（通用，非按地图） ---
constexpr float ENEMY_CULL_MARGIN = 0.35f; // 清理边距（VIEW_WIDTH 比例）

// --- 经验 ---
constexpr float XP_BASE_THRESHOLD = 10.f;
constexpr float XP_THRESHOLD_GROWTH = 5.f;  // 每级增加值
constexpr float XP_GEM_MAGNET_DELAY = 1.5f; // 磁铁吸引前的等待时间
constexpr float XP_GEM_MAGNET_SPEED = 400.f;

// --- 相机 / 视口 ---
constexpr float VIEW_WIDTH = 1920.f;
constexpr float VIEW_HEIGHT = 1080.f;
constexpr float FIXED_DT = 1.f / 60.f; // 固定时间步

// --- 武器 ---
constexpr float RANGE_UNLIMITED = 0.f; // 表示无射程限制

// --- 数学 ---
constexpr float PI = std::numbers::pi_v<float>;
constexpr float TAU = 2.f * PI;

// --- 敌人（非按类型的通用常数）---
// 各类型具体属性见 gameplay/EnemyDefs.hpp — ENEMY_DEFS[] 表
constexpr float ENEMY_ANIM_FRAME_DURATION = 0.12f;  // 精灵动画帧间隔
constexpr float PLAYER_ANIM_FRAME_DURATION = 0.12f; // 玩家动画帧间隔
constexpr float ENEMY_HIT_FLASH_DURATION = 0.1f;    // 受击闪白时长

// --- 伤害飘字 ---
constexpr float DMGTEXT_VELOCITY_Y = -50.f;
constexpr float DMGTEXT_LIFETIME = 0.6f;
constexpr float DMGTEXT_Y_OFFSET = -10.f;
constexpr float DMGTEXT_X_SPREAD = 20.f;

// --- 音频 ---
constexpr float BGM_VOLUME = 50.f;

// --- 资源路径 ---
constexpr const char* FONT_DEFAULT_PATH = "assets/fonts/fusion-pixel-12px-proportional-zh_hans.ttf";

// --- 音效定义（数据驱动：一行一个音效的全部信息）---
enum class SoundId : std::uint8_t { Shoot, Hit, Kill, Hurt, Pickup, LevelUp, Count };

struct SoundDef {
    SoundId id;
    const char* key;      // ResourceManager 查找 key
    const char* path;     // 音频文件路径
    float volume = 100.f; // 音量 0–100
    float interval = 0.f; // 最短触发间隔（秒），0 = 无限制
};

constexpr SoundDef SOUND_DEFS[] = {
    {.id = SoundId::Shoot,
     .key = "shoot",
     .path = "assets/sounds/sfx/shoot.wav",
     .volume = 50.f,
     .interval = 0.08f},
    {.id = SoundId::Hit,
     .key = "hit",
     .path = "assets/sounds/sfx/hit.wav",
     .volume = 45.f,
     .interval = 0.05f},
    {.id = SoundId::Kill,
     .key = "kill",
     .path = "assets/sounds/sfx/kill.wav",
     .volume = 55.f,
     .interval = 0.10f},
    {.id = SoundId::Hurt,
     .key = "hurt",
     .path = "assets/sounds/sfx/hurt.wav",
     .volume = 70.f,
     .interval = 0.25f},
    {.id = SoundId::Pickup,
     .key = "pickup",
     .path = "assets/sounds/sfx/pickup.wav",
     .volume = 40.f,
     .interval = 0.10f},
    {.id = SoundId::LevelUp,
     .key = "levelup",
     .path = "assets/sounds/sfx/levelup.wav",
     .volume = 65.f,
     .interval = 0.f},
};
static_assert(sizeof(SOUND_DEFS) / sizeof(SOUND_DEFS[0]) == static_cast<int>(SoundId::Count),
              "SOUND_DEFS must have one entry per SoundId");

// --- 对象池预分配 ---
constexpr std::size_t POOL_ENEMIES_CAPACITY = 500;
constexpr std::size_t POOL_PROJECTILES_CAPACITY = 200;
constexpr std::size_t POOL_XPGEMS_CAPACITY = 300;
constexpr std::size_t POOL_DAMAGETEXTS_CAPACITY = 200;
constexpr std::size_t POOL_SOUNDS_CAPACITY = 24;

// --- 颜色 ---
inline const sf::Color COLOR_BG_PLAY(30, 30, 30);
inline const sf::Color COLOR_BG_TITLE(20, 20, 20);
inline const sf::Color COLOR_BG_GAMEOVER(20, 0, 0);
inline const sf::Color COLOR_OVERLAY_PAUSE(0, 0, 0, 160);
inline const sf::Color COLOR_OVERLAY_UPGRADE(0, 0, 0, 180);
inline const sf::Color COLOR_TEXT_DEFAULT(200, 200, 200);
inline const sf::Color COLOR_TEXT_SELECTED(sf::Color::Yellow);
inline const sf::Color COLOR_TEXT_MUTED(160, 160, 160);
inline const sf::Color COLOR_TEXT_DIM(150, 150, 150);
inline const sf::Color COLOR_TEXT_SUCCESS(140, 200, 140);
inline const sf::Color COLOR_UI_HP_BG(60, 0, 0);
inline const sf::Color COLOR_UI_XP_BG(60, 60, 0);

} // namespace Config
