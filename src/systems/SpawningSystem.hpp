#pragma once

#include "core/Pool.hpp"
#include "data/EntityTypes.hpp"

#include <SFML/System/Vector2.hpp>
#include <span>

struct SpriteSheet;
struct MapDef;

/// 敌人生成 + 波次管理 + 难度递增。拥有生成状态。
class SpawningSystem {
public:
    SpawningSystem();

    /// 设置地图配置（PlayScene 构造时调用一次）。
    void setMap(const MapDef& map);

    /// 每帧调用。gameTime = 累计游戏时间，playerPos = 玩家位置。
    void update(float dt, float gameTime, sf::Vector2f playerPos, Pool<Enemy>& enemies);

    /// 设置敌人精灵表指针（PlayScene 加载后调用一次）。
    void setEnemySprites(std::span<const SpriteSheet> spritesMove,
                         std::span<const SpriteSheet> spritesDamaged);

    /// 新一局重置。
    void reset();

private:
    void spawnEnemy(EnemyType type, sf::Vector2f playerPos, Pool<Enemy>& enemies);
    sf::Vector2f randomSpawnPosition(sf::Vector2f playerPos) const;

    const MapDef* m_map = nullptr;

    float m_spawnTimer = 0.f;
    float m_spawnInterval = 0.f;
    int m_enemiesPerWave = 0;
    float m_difficultyTimer = 0.f;
    float m_bossTimer = 0.f;

    const SpriteSheet* m_spritesMove = nullptr;
    const SpriteSheet* m_spritesDamaged = nullptr;
};
