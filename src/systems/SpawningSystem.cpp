#include "systems/SpawningSystem.hpp"
#include "core/Random.hpp"
#include "data/Constants.hpp"
#include "gameplay/EnemyDefs.hpp"
#include "graphics/SpriteSheet.hpp"

#include <cmath>

SpawningSystem::SpawningSystem() { reset(); }

void SpawningSystem::setEnemySprites(const SpriteSheet* spritesMove,
                                     const SpriteSheet* spritesDamaged) {
    m_spritesMove = spritesMove;
    m_spritesDamaged = spritesDamaged;
}

void SpawningSystem::reset() {
    m_spawnTimer = 0.f;
    m_spawnInterval = Config::ENEMY_BASE_SPAWN_INTERVAL;
    m_enemiesPerWave = Config::ENEMIES_PER_WAVE_BASE;
    m_difficultyTimer = 0.f;
    m_bossTimer = Config::ENEMY_BOSS_INTERVAL; // 首个 Boss
}

void SpawningSystem::update(float dt, float gameTime, sf::Vector2f playerPos,
                            Pool<Enemy>& enemies) {
    m_spawnTimer -= dt;
    m_difficultyTimer += dt;
    m_bossTimer -= dt;

    // 每 60 秒生成 Boss
    if (m_bossTimer <= 0.f) {
        spawnEnemy(EnemyType::Boss, playerPos, enemies);
        m_bossTimer = Config::ENEMY_BOSS_INTERVAL;
    }

    if (m_spawnTimer > 0.f)
        return;

    // 敌人数上限保护帧率
    if (enemies.activeCount() >= Config::ENEMY_MAX_COUNT)
        return;

    // 根据已过时间解锁敌人类型
    int activeTypes = 1;
    for (int t = 1; t < static_cast<int>(EnemyType::Count); ++t) {
        if (ENEMY_DEFS[t].appearTime >= 0.f && gameTime > ENEMY_DEFS[t].appearTime)
            activeTypes = t + 1;
    }

    for (int i = 0; i < m_enemiesPerWave; ++i) {
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
    m_spawnInterval =
        Config::ENEMY_BASE_SPAWN_INTERVAL - m_difficultyTimer * Config::ENEMY_DIFFICULTY_SCALE;
    if (m_spawnInterval < Config::ENEMY_MIN_SPAWN_INTERVAL)
        m_spawnInterval = Config::ENEMY_MIN_SPAWN_INTERVAL;

    m_spawnTimer = m_spawnInterval;
    m_enemiesPerWave = Config::ENEMIES_PER_WAVE_BASE +
                       static_cast<int>(m_difficultyTimer / Config::ENEMY_DIFFICULTY_WAVE_INTERVAL);
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
    float dist = Config::ENEMY_SPAWN_DISTANCE + Random::getFloat() * Config::ENEMY_SPAWN_JITTER;
    return playerPos + sf::Vector2f(std::cos(angle) * dist, std::sin(angle) * dist);
}
