#pragma once

#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>

namespace Config {

// --- 世界 ---
constexpr float WORLD_WIDTH = 3840.f;
constexpr float WORLD_HEIGHT = 2160.f;

// --- 玩家 ---
constexpr float PLAYER_RADIUS = 16.f;
constexpr float PLAYER_SPEED = 220.f;
constexpr float PLAYER_MAX_HP = 100.f;
constexpr float PLAYER_ARMOR = 0.f;    // 伤害减免 0–1
constexpr float PLAYER_MAGNET = 80.f;  // 宝石拾取范围
constexpr float PLAYER_IFRAMES = 0.5f; // 受击后无敌时间（秒）
constexpr int PLAYER_MAX_WEAPONS = 6;

// --- 敌人 ---
constexpr float ENEMY_SPAWN_DISTANCE = 1200.f; // 生成时距玩家的最小距离
constexpr float ENEMY_BASE_SPAWN_INTERVAL = 3.f;
constexpr float ENEMY_MIN_SPAWN_INTERVAL = 0.5f;
constexpr int ENEMIES_PER_WAVE_BASE = 3;
constexpr float ENEMY_BOSS_INTERVAL = 60.f;      // Boss 生成间隔（秒）
constexpr int ENEMY_MAX_COUNT = 200;             // 敌人数上限
constexpr float ENEMY_DIFFICULTY_SCALE = 0.005f; // 难度递增系数
constexpr float ENEMY_SPAWN_JITTER = 200.f;      // 生成位置随机抖动范围
constexpr float ENEMY_CULL_MARGIN = 0.35f;       // 清理边距（VIEW_WIDTH 比例）

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
constexpr float RANGE_UNLIMITED = 0.f;   // 表示无射程限制
constexpr float KNIFE_SPREAD = 0.15f;    // 飞刀散布角（弧度）
constexpr float AXE_ORBIT_RADIUS = 60.f; // 斧头轨道半径
constexpr float AXE_ORBIT_SPEED = 3.0f;  // 斧头轨道角速度（弧度/秒）

// --- 敌人类型属性表 ---
// 按 EnemyType 枚举顺序: Basic, Fast, Tank, Boss
constexpr float ENEMY_HP[] = {20.f, 10.f, 80.f, 300.f};
constexpr float ENEMY_SPEED[] = {80.f, 160.f, 50.f, 60.f};
constexpr float ENEMY_DAMAGE[] = {10.f, 8.f, 20.f, 30.f};
constexpr float ENEMY_RADIUS[] = {14.f, 10.f, 22.f, 32.f};
constexpr float ENEMY_XP[] = {1.f, 2.f, 5.f, 50.f};
constexpr float ENEMY_SPAWN_WEIGHT[] = {1.f, 0.7f, 0.4f, 0.f}; // Boss 单独生成

// --- 敌人类型出现时间（秒）---
constexpr float ENEMY_APPEAR_TIME[] = {0.f, 30.f, 60.f, -1.f}; // -1 = 不按权重生成

// --- 资源路径 ---
constexpr const char* FONT_DEFAULT_PATH = "assets/fonts/DejaVuSans.ttf";
constexpr const char* SOUND_SHOOT_PATH = "assets/sounds/shoot.wav";
constexpr const char* SOUND_HIT_PATH = "assets/sounds/hit.wav";
constexpr const char* SOUND_KILL_PATH = "assets/sounds/kill.wav";
constexpr const char* SOUND_HURT_PATH = "assets/sounds/hurt.wav";
constexpr const char* SOUND_PICKUP_PATH = "assets/sounds/pickup.wav";
constexpr const char* SOUND_LEVELUP_PATH = "assets/sounds/levelup.wav";
constexpr const char* BGM_PLAY_SCENE_PATH = "assets/sounds/BGM/stone fortress.ogg";

// --- 音效配置 ---
struct SoundConfig {
    float volume = 100.f; // 音量 0-100
    float interval = 0.f; // 最短触发间隔（秒），0 = 无限制
};

constexpr SoundConfig SOUND_CFG_SHOOT{50.f, 0.08f};
constexpr SoundConfig SOUND_CFG_HIT{45.f, 0.05f};
constexpr SoundConfig SOUND_CFG_KILL{55.f, 0.10f};
constexpr SoundConfig SOUND_CFG_HURT{70.f, 0.f};
constexpr SoundConfig SOUND_CFG_PICKUP{40.f, 0.f};
constexpr SoundConfig SOUND_CFG_LEVELUP{65.f, 0.f};

// --- 对象池预分配 ---
constexpr std::size_t POOL_ENEMIES_CAPACITY = 500;
constexpr std::size_t POOL_PROJECTILES_CAPACITY = 200;
constexpr std::size_t POOL_XPGEMS_CAPACITY = 300;
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

inline const sf::Color COLOR_ENEMY_BASIC(sf::Color::Red);
inline const sf::Color COLOR_ENEMY_FAST(255, 100, 100);
inline const sf::Color COLOR_ENEMY_TANK(180, 0, 0);
inline const sf::Color COLOR_ENEMY_BOSS(255, 0, 100);

} // namespace Config
