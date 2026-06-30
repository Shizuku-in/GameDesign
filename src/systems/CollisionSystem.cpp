#include "systems/CollisionSystem.hpp"
#include "audio/SoundPlayer.hpp"
#include "core/Random.hpp"
#include "data/Constants.hpp"
#include "gameplay/EnemyDefs.hpp"
#include "math/Collision.hpp"
#include <algorithm>
#include <vector>

namespace CollisionSystem {

namespace {
constexpr float CELL_SIZE = 100.f;
constexpr int GRID_COLS = static_cast<int>(Config::WORLD_WIDTH / CELL_SIZE) + 1;
constexpr int GRID_ROWS = static_cast<int>(Config::WORLD_HEIGHT / CELL_SIZE) + 1;

int getCellIndex(sf::Vector2f pos) {
    int cx = static_cast<int>(pos.x / CELL_SIZE);
    int cy = static_cast<int>(pos.y / CELL_SIZE);
    cx = std::clamp(cx, 0, GRID_COLS - 1);
    cy = std::clamp(cy, 0, GRID_ROWS - 1);
    return cy * GRID_COLS + cx;
}
} // namespace

void processCollisions(PlayerState& player, Pool<Enemy>& enemies, Pool<Projectile>& projectiles,
                       Pool<XPGem>& gems, Pool<DamageText>& damageTexts, int& score,
                       SoundPlayer& sounds) {
    // 使用 thread_local 以保证线程安全，同时复用 std::vector 的 capacity
    // 避免如果直接放在栈上每帧触发 800+ 次析构和堆内存分配带来的极差性能
    thread_local std::vector<Enemy*> grid[GRID_COLS * GRID_ROWS];
    // 缓存最大敌人半径 — ENEMY_DEFS 在运行时不变，只需计算一次
    static const float maxEnemyRadius = [] {
        float m = 0.f;
        for (int i = 0; i < static_cast<int>(EnemyType::Count); ++i)
            if (ENEMY_DEFS[i].radius > m)
                m = ENEMY_DEFS[i].radius;
        return m;
    }();

    // 1. 构建空间哈希网格 (O(N))
    for (auto& cell : grid) {
        cell.clear(); // clear 只归零 size，保留 capacity，速度极快
    }
    enemies.forEach([&](Enemy& e) {
        if (e.hp > 0.f) {
            grid[getCellIndex(e.pos)].push_back(&e);
        }
    });

    // 2. 弹幕 vs 敌人 (利用网格进行宽阶段裁剪)
    projectiles.forEach([&](Projectile& p) {
        if (p.lifetime <= 0.f) [[unlikely]]
            return;

        // 计算弹幕可能触及的格子范围（考虑最大敌人半径，防止边缘漏判）
        float searchRadius = p.radius + maxEnemyRadius;
        int minCx =
            std::clamp(static_cast<int>((p.pos.x - searchRadius) / CELL_SIZE), 0, GRID_COLS - 1);
        int maxCx =
            std::clamp(static_cast<int>((p.pos.x + searchRadius) / CELL_SIZE), 0, GRID_COLS - 1);
        int minCy =
            std::clamp(static_cast<int>((p.pos.y - searchRadius) / CELL_SIZE), 0, GRID_ROWS - 1);
        int maxCy =
            std::clamp(static_cast<int>((p.pos.y + searchRadius) / CELL_SIZE), 0, GRID_ROWS - 1);

        for (int cy = minCy; cy <= maxCy; ++cy) {
            for (int cx = minCx; cx <= maxCx; ++cx) {
                int cellIdx = cy * GRID_COLS + cx;
                for (Enemy* e : grid[cellIdx]) {
                    if (e->hp <= 0.f) [[unlikely]]
                        continue;

                    if (circleCircle(p.pos, p.radius, e->pos, e->radius)) [[unlikely]] {
                        const sf::Vector2f hitPos = p.pos; // 爆炸以命中点为中心
                        e->hp -= p.damage;
                        e->hitFlashTimer = Config::ENEMY_HIT_FLASH_DURATION;

                        // 生成飘字
                        auto dtHandle = damageTexts.acquire();
                        if (auto* dmgTxt = damageTexts.get(dtHandle)) {
                            float offsetX = (Random::getFloat() - 0.5f) * Config::DMGTEXT_X_SPREAD;
                            dmgTxt->pos = e->pos + sf::Vector2f(offsetX, Config::DMGTEXT_Y_OFFSET);
                            dmgTxt->vel = sf::Vector2f(0.f, Config::DMGTEXT_VELOCITY_Y);
                            dmgTxt->damage = p.damage;
                            dmgTxt->maxLifetime = dmgTxt->lifetime = Config::DMGTEXT_LIFETIME;
                        }

                        // AoE 爆炸：对范围内所有敌人造成伤害
                        if (p.aoeRadius > 0.f) [[unlikely]] {
                            float explosionSearch = p.aoeRadius + maxEnemyRadius;
                            int eMinCx = std::clamp(
                                static_cast<int>((hitPos.x - explosionSearch) / CELL_SIZE), 0,
                                GRID_COLS - 1);
                            int eMaxCx = std::clamp(
                                static_cast<int>((hitPos.x + explosionSearch) / CELL_SIZE), 0,
                                GRID_COLS - 1);
                            int eMinCy = std::clamp(
                                static_cast<int>((hitPos.y - explosionSearch) / CELL_SIZE), 0,
                                GRID_ROWS - 1);
                            int eMaxCy = std::clamp(
                                static_cast<int>((hitPos.y + explosionSearch) / CELL_SIZE), 0,
                                GRID_ROWS - 1);
                            for (int ey = eMinCy; ey <= eMaxCy; ++ey) {
                                for (int ex = eMinCx; ex <= eMaxCx; ++ex) {
                                    for (Enemy* oe : grid[ey * GRID_COLS + ex]) {
                                        if (oe == e || oe->hp <= 0.f) [[unlikely]]
                                            continue;
                                        if (circleCircle(hitPos, p.aoeRadius, oe->pos, oe->radius))
                                            [[unlikely]] {
                                            oe->hp -= p.damage;
                                            oe->hitFlashTimer = Config::ENEMY_HIT_FLASH_DURATION;
                                            if (oe->hp <= 0.f) [[unlikely]] {
                                                auto h = gems.acquire();
                                                if (auto* g = gems.get(h)) {
                                                    g->pos = oe->pos;
                                                    g->value = oe->xpValue;
                                                    g->radius = 5.f;
                                                    g->magnetTimer = Config::XP_GEM_MAGNET_DELAY;
                                                }
                                                ++score;
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        --p.pierceCount;

                        if (e->hp <= 0.f) [[unlikely]] {
                            auto handle = gems.acquire();
                            auto* g = gems.get(handle);
                            if (g) {
                                g->pos = e->pos;
                                g->value = e->xpValue;
                                g->radius = 5.f;
                                g->magnetTimer = Config::XP_GEM_MAGNET_DELAY;
                            }
                            ++score;
                            sounds.kill();
                        } else {
                            sounds.hit();
                        }

                        if (p.pierceCount <= 0) {
                            p.lifetime = 0.f;
                            goto NEXT_PROJECTILE; // 穿透耗尽，直接检测下一发弹幕
                        }
                    }
                }
            }
        }
    NEXT_PROJECTILE:;
    });

    // 3. 玩家 vs 敌人 (同样利用网格加速)
    if (player.invincibilityTimer <= 0.f) {
        bool tookDamage = false;

        float searchRadius = player.radius + maxEnemyRadius;
        int minCx = std::clamp(static_cast<int>((player.pos.x - searchRadius) / CELL_SIZE), 0,
                               GRID_COLS - 1);
        int maxCx = std::clamp(static_cast<int>((player.pos.x + searchRadius) / CELL_SIZE), 0,
                               GRID_COLS - 1);
        int minCy = std::clamp(static_cast<int>((player.pos.y - searchRadius) / CELL_SIZE), 0,
                               GRID_ROWS - 1);
        int maxCy = std::clamp(static_cast<int>((player.pos.y + searchRadius) / CELL_SIZE), 0,
                               GRID_ROWS - 1);

        for (int cy = minCy; cy <= maxCy; ++cy) {
            for (int cx = minCx; cx <= maxCx; ++cx) {
                int cellIdx = cy * GRID_COLS + cx;
                for (Enemy* e : grid[cellIdx]) {
                    if (e->hp <= 0.f) [[unlikely]]
                        continue;

                    if (circleCircle(player.pos, player.radius, e->pos, e->radius)) [[unlikely]] {
                        float dmg = e->damage * Config::FIXED_DT * (1.f - player.armor);
                        player.hp -= dmg;
                        player.invincibilityTimer = Config::PLAYER_IFRAMES;
                        tookDamage = true;
                        goto PLAYER_HIT_DONE; // 无敌帧触发，跳出检测
                    }
                }
            }
        }
    PLAYER_HIT_DONE:
        if (tookDamage)
            sounds.hurt();
    }

    // 玩家 vs XP 宝石
    gems.forEach([&](XPGem& g) {
        if (circleCircle(player.pos, player.magnetRange, g.pos, g.radius)) {
            player.xp += g.value * player.xpMultiplier;
            g.value = -1.f;
            sounds.pickup();
        }
    });

    // 清理
    enemies.forEachHandle([&](Pool<Enemy>::Handle h, Enemy& e) {
        if (e.hp <= 0.f)
            enemies.release(h);
    });
    projectiles.forEachHandle([&](Pool<Projectile>::Handle h, Projectile& p) {
        if (p.lifetime <= 0.f)
            projectiles.release(h);
    });
    gems.forEachHandle([&](Pool<XPGem>::Handle h, XPGem& g) {
        if (g.value <= 0.f)
            gems.release(h);
    });
}

} // namespace CollisionSystem
