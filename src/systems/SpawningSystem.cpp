#include "systems/SpawningSystem.hpp"
#include "core/Random.hpp"
#include "data/Constants.hpp"
#include "gameplay/EnemyDefs.hpp"
#include "gameplay/MapDefs.hpp"
#include "graphics/SpriteSheet.hpp"

#include <algorithm>
#include <cmath>
#include <span>

SpawningSystem::SpawningSystem() { reset(); }

void SpawningSystem::setMap(const MapDef& map) { m_map = &map; }

void SpawningSystem::setEnemySprites(std::span<const SpriteSheet> spritesMove,
                                     std::span<const SpriteSheet> spritesDamaged) {
    m_spritesMove = spritesMove.data();
    m_spritesDamaged = spritesDamaged.data();
}

void SpawningSystem::reset() {
    m_spawnTimer = 0.f;
    m_difficultyTimer = 0.f;
    if (m_map) {
        m_spawnInterval = m_map->baseSpawnInterval;
        m_enemiesPerWave = m_map->baseEnemiesPerWave;
        m_bossTimer = m_map->bossInterval;
    }
}

void SpawningSystem::update(float dt, float gameTime, sf::Vector2f playerPos,
                            Pool<Enemy>& enemies) {
    if (!m_map)
        return;

    m_spawnTimer -= dt;
    m_difficultyTimer += dt;
    m_bossTimer -= dt;

    // Boss 定时生成
    if (m_bossTimer <= 0.f) {
        spawnEnemy(EnemyType::Boss, playerPos, enemies);
        m_bossTimer = m_map->bossInterval;
    }

    if (m_spawnTimer > 0.f)
        return;

    // 敌人数上限保护帧率
    if (enemies.activeCount() >= static_cast<std::size_t>(m_map->maxEnemies))
        return;

    // 根据已过时间解锁敌人类型
    int activeTypes = 1;
    for (int t = 1; t < static_cast<int>(EnemyType::Count); ++t) {
        if (ENEMY_DEFS[t].appearTime >= 0.f && gameTime > ENEMY_DEFS[t].appearTime)
            activeTypes = t + 1;
    }

    // 每波上限
    int waveCount = std::min(m_enemiesPerWave, m_map->maxEnemiesPerWave);
    for (int i = 0; i < waveCount; ++i) {
        // 逐个检查上限，防止波次超出
        if (enemies.activeCount() >= static_cast<std::size_t>(m_map->maxEnemies))
            break;

        float totalWeight = 0.f;
        for (int t = 0; t < activeTypes; ++t) {
            if (ENEMY_DEFS[t].spawnWeight > 0.f)
                totalWeight += ENEMY_DEFS[t].spawnWeight;
        }

        float r = Random::getFloat() * totalWeight;
        float accum = 0.f;
        EnemyType chosen = EnemyType::Basic;
        for (int t = 0; t < activeTypes; ++t) {
            if (ENEMY_DEFS[t].spawnWeight <= 0.f)
                continue;
            accum += ENEMY_DEFS[t].spawnWeight;
            if (r <= accum) {
                chosen = static_cast<EnemyType>(t);
                break;
            }
        }
        spawnEnemy(chosen, playerPos, enemies);
    }

    // 难度递增
    m_spawnInterval = m_map->baseSpawnInterval - m_difficultyTimer * m_map->difficultyScale;
    if (m_spawnInterval < m_map->minSpawnInterval)
        m_spawnInterval = m_map->minSpawnInterval;

    m_spawnTimer = m_spawnInterval;
    m_enemiesPerWave =
        m_map->baseEnemiesPerWave + static_cast<int>(m_difficultyTimer / m_map->waveInterval);
}

void SpawningSystem::spawnEnemy(EnemyType type, sf::Vector2f playerPos, Pool<Enemy>& enemies) {
    int t = static_cast<int>(type);
    const auto& def = ENEMY_DEFS[t];

    auto handle = enemies.acquire();
    auto* e = enemies.get(handle);
    if (!e)
        return;

    e->pos = randomSpawnPosition(playerPos);
    e->hp = def.hp;
    e->maxHp = def.hp;
    e->speed = def.speed;
    e->radius = def.radius;
    e->damage = def.damage;
    e->xpValue = def.xpValue;
    e->type = type;
    e->spriteScale = def.spriteScale;
    e->spriteMove = m_spritesMove ? &m_spritesMove[t] : nullptr;
    e->spriteDamaged = m_spritesDamaged ? &m_spritesDamaged[t] : nullptr;
    e->currentSprite = e->spriteMove; // 首帧可见
}

sf::Vector2f SpawningSystem::randomSpawnPosition(sf::Vector2f playerPos) const {
    float angle = Random::getFloat() * Config::TAU;
    float dist = m_map->spawnDistance + Random::getFloat() * m_map->spawnJitter;
    return playerPos + sf::Vector2f(std::cos(angle) * dist, std::sin(angle) * dist);
}
