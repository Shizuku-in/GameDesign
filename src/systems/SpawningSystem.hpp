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
    /// 创建生成状态；地图配置在 setMap 后生效。
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
    /// 从对象池创建并初始化一个指定类型的敌人。
    void spawnEnemy(EnemyType type, sf::Vector2f playerPos, Pool<Enemy>& enemies);
    /// 在玩家周围生成环带内选择一个随机位置。
    [[nodiscard]] sf::Vector2f randomSpawnPosition(sf::Vector2f playerPos) const;

    /// 当前地图生成配置；为空时不生成敌人。
    const MapDef* m_map = nullptr;

    /// 距离下一波普通敌人生成的剩余时间。
    float m_spawnTimer = 0.f;
    /// 根据难度计算出的当前波次间隔。
    float m_spawnInterval = 0.f;
    /// 当前每波生成的敌人数。
    int m_enemiesPerWave = 0;
    /// 用于提高波次难度的累计时间。
    float m_difficultyTimer = 0.f;
    /// 距离下一只 Boss 出现的剩余时间。
    float m_bossTimer = 0.f;

    /// 按 EnemyType 索引的敌人移动精灵表。
    const SpriteSheet* m_spritesMove = nullptr;
    /// 按 EnemyType 索引的敌人受击精灵表。
    const SpriteSheet* m_spritesDamaged = nullptr;
};
