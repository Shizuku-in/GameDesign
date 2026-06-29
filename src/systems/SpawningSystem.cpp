#include "systems/SpawningSystem.hpp"
#include "data/Constants.hpp"

#include <cmath>
#include <cstdlib>

namespace {
float randFloat() { return static_cast<float>(std::rand()) / RAND_MAX; }
} // namespace

SpawningSystem::SpawningSystem() { reset(); }

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
    if (gameTime > Config::ENEMY_APPEAR_TIME[1])
        activeTypes = 2;
    if (gameTime > Config::ENEMY_APPEAR_TIME[2])
        activeTypes = 3;

    for (int i = 0; i < m_enemiesPerWave; ++i) {
        float totalWeight = 0.f;
        for (int t = 0; t < activeTypes; ++t)
            totalWeight += Config::ENEMY_SPAWN_WEIGHT[t];

        float r = randFloat() * totalWeight;
        float accum = 0.f;
        EnemyType chosen = EnemyType::Basic;
        for (int t = 0; t < activeTypes; ++t) {
            accum += Config::ENEMY_SPAWN_WEIGHT[t];
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
    m_enemiesPerWave = Config::ENEMIES_PER_WAVE_BASE + static_cast<int>(m_difficultyTimer / 10.f);
}

void SpawningSystem::spawnEnemy(EnemyType type, sf::Vector2f playerPos, Pool<Enemy>& enemies) {
    int t = static_cast<int>(type);
    auto handle = enemies.acquire();
    auto* e = enemies.get(handle);
    if (!e)
        return;

    e->pos = randomSpawnPosition(playerPos);
    e->hp = Config::ENEMY_HP[t];
    e->maxHp = Config::ENEMY_HP[t];
    e->speed = Config::ENEMY_SPEED[t];
    e->radius = Config::ENEMY_RADIUS[t];
    e->damage = Config::ENEMY_DAMAGE[t];
    e->xpValue = Config::ENEMY_XP[t];
    e->type = type;
}

sf::Vector2f SpawningSystem::randomSpawnPosition(sf::Vector2f playerPos) const {
    float angle = randFloat() * 2.f * 3.14159265f;
    float dist = Config::ENEMY_SPAWN_DISTANCE + randFloat() * Config::ENEMY_SPAWN_JITTER;
    return playerPos + sf::Vector2f(std::cos(angle) * dist, std::sin(angle) * dist);
}
