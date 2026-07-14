#include "systems/CollisionSystem.hpp"

#include "audio/SoundPlayer.hpp"
#include "core/Random.hpp"
#include "data/Constants.hpp"
#include "gameplay/EnemyDefs.hpp"
#include "math/Collision.hpp"

#include <algorithm>
#include <cassert>
#include <vector>

namespace CollisionSystem {

namespace {

constexpr float CELL_SIZE = 100.f;

// 缓存最大敌人半径 — ENEMY_DEFS 在运行时不变，只需计算一次
float maxEnemyRadius() {
    static const float M = [] {
        float m = 0.f;
        for (int i = 0; i < static_cast<int>(EnemyType::Count); ++i) {
            m = std::max(ENEMY_DEFS[i].radius, m);
        }
        return m;
    }();
    return M;
}

// ---------------------------------------------------------------------------
// 子步骤
// ---------------------------------------------------------------------------

using Grid = std::vector<std::vector<Enemy*>>;

/// 弹幕 vs 敌人 + AoE 爆炸。
void processProjectileEnemyCollisions(Pool<Projectile>& projectiles, Grid& grid, int gridCols,
                                      int gridRows, Pool<DamageText>& damageTexts,
                                      Pool<XPGem>& gems, int& score, SoundPlayer& sounds) {
    assert(grid.size() == static_cast<std::size_t>(gridCols) * static_cast<std::size_t>(gridRows));

    projectiles.forEach([&](Projectile& p) {
        if (p.lifetime <= 0.f) {
            [[unlikely]] return;
        }

        float searchRadius = p.radius + maxEnemyRadius();
        int minCx =
            std::clamp(static_cast<int>((p.pos.x - searchRadius) / CELL_SIZE), 0, gridCols - 1);
        int maxCx =
            std::clamp(static_cast<int>((p.pos.x + searchRadius) / CELL_SIZE), 0, gridCols - 1);
        int minCy =
            std::clamp(static_cast<int>((p.pos.y - searchRadius) / CELL_SIZE), 0, gridRows - 1);
        int maxCy =
            std::clamp(static_cast<int>((p.pos.y + searchRadius) / CELL_SIZE), 0, gridRows - 1);

        for (int cy = minCy; cy <= maxCy; ++cy) {
            for (int cx = minCx; cx <= maxCx; ++cx) {
                int cellIdx = (cy * gridCols) + cx;
                for (Enemy* e : grid[cellIdx]) {
                    if (e->hp <= 0.f) {
                        [[unlikely]] continue;
                    }
                    if (e->hitFlashTimer > 0.f) {
                        continue;
                    }

                    if (circleCircle(p.pos, p.radius, e->pos, e->radius)) [[unlikely]] {
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
                            float explosionSearch = p.aoeRadius + maxEnemyRadius();
                            int eMinCx = std::clamp(
                                static_cast<int>((p.pos.x - explosionSearch) / CELL_SIZE), 0,
                                gridCols - 1);
                            int eMaxCx = std::clamp(
                                static_cast<int>((p.pos.x + explosionSearch) / CELL_SIZE), 0,
                                gridCols - 1);
                            int eMinCy = std::clamp(
                                static_cast<int>((p.pos.y - explosionSearch) / CELL_SIZE), 0,
                                gridRows - 1);
                            int eMaxCy = std::clamp(
                                static_cast<int>((p.pos.y + explosionSearch) / CELL_SIZE), 0,
                                gridRows - 1);
                            for (int ey = eMinCy; ey <= eMaxCy; ++ey) {
                                for (int ex = eMinCx; ex <= eMaxCx; ++ex) {
                                    for (Enemy* oe : grid[(ey * gridCols) + ex]) {
                                        if (oe == e || oe->hp <= 0.f) {
                                            [[unlikely]] continue;
                                        }
                                        if (circleCircle(p.pos, p.aoeRadius, oe->pos, oe->radius))
                                            [[unlikely]] {
                                            oe->hp -= p.damage;
                                            oe->hitFlashTimer = Config::ENEMY_HIT_FLASH_DURATION;
                                            if (oe->hp <= 0.f) [[unlikely]] {
                                                oe->killed = true;
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
                            e->killed = true;
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
                            return;
                        }
                    }
                }
            }
        }
    });
}

/// 玩家 vs 敌人（每个固定更新最多结算一次接触伤害）。
void processPlayerEnemyCollision(PlayerState& player, Grid& grid, int gridCols, int gridRows,
                                 SoundPlayer& sounds) {
    float searchRadius = player.radius + maxEnemyRadius();
    int minCx =
        std::clamp(static_cast<int>((player.pos.x - searchRadius) / CELL_SIZE), 0, gridCols - 1);
    int maxCx =
        std::clamp(static_cast<int>((player.pos.x + searchRadius) / CELL_SIZE), 0, gridCols - 1);
    int minCy =
        std::clamp(static_cast<int>((player.pos.y - searchRadius) / CELL_SIZE), 0, gridRows - 1);
    int maxCy =
        std::clamp(static_cast<int>((player.pos.y + searchRadius) / CELL_SIZE), 0, gridRows - 1);

    for (int cy = minCy; cy <= maxCy; ++cy) {
        for (int cx = minCx; cx <= maxCx; ++cx) {
            int cellIdx = (cy * gridCols) + cx;
            for (Enemy* e : grid[cellIdx]) {
                if (e->hp <= 0.f) {
                    [[unlikely]] continue;
                }
                if (e->frozenTimer > 0.f) {
                    // 冻结的敌人不造成伤害
                    continue;
                }
                if (circleCircle(player.pos, player.radius, e->pos, e->radius)) [[unlikely]] {
                    float dmg = e->damage * Config::PLAYER_CONTACT_DAMAGE_SCALE * Config::FIXED_DT *
                                (1.f - player.armor);
                    player.hp -= dmg;
                    sounds.hurt();
                    return;
                }
            }
        }
    }
}

/// 玩家 vs XP 宝石。
void processPlayerGemPickups(PlayerState& player, Pool<XPGem>& gems, SoundPlayer& sounds) {
    gems.forEach([&](XPGem& g) {
        if (circleCircle(player.pos, player.magnetRange, g.pos, g.radius)) {
            player.xp += g.value * player.xpMultiplier;
            g.value = -1.f;
            sounds.pickup();
        }
    });
}

/// 清理死亡敌人、过期弹幕、已收集宝石。未结算掉落在此补发。
void cleanupEntities(Pool<Enemy>& enemies, Pool<Projectile>& projectiles, Pool<XPGem>& gems,
                     int& score, SoundPlayer& sounds) {
    enemies.forEachHandle([&](Pool<Enemy>::Handle h, Enemy& e) {
        if (e.hp <= 0.f) {
            if (!e.killed) {
                auto gemHandle = gems.acquire();
                if (auto* g = gems.get(gemHandle)) {
                    g->pos = e.pos;
                    g->value = e.xpValue;
                    g->radius = 5.f;
                    g->magnetTimer = Config::XP_GEM_MAGNET_DELAY;
                }
                ++score;
                sounds.kill();
            }
            enemies.release(h);
        }
    });
    projectiles.forEachHandle([&](Pool<Projectile>::Handle h, Projectile& p) {
        if (p.lifetime <= 0.f) {
            projectiles.release(h);
        }
    });
    gems.forEachHandle([&](Pool<XPGem>::Handle h, XPGem& g) {
        if (g.value <= 0.f) {
            gems.release(h);
        }
    });
}

} // namespace

// ===========================================================================
// 主入口
// ===========================================================================

void processCollisions(PlayerState& player, Pool<Enemy>& enemies, Pool<Projectile>& projectiles,
                       Pool<XPGem>& gems, Pool<DamageText>& damageTexts, int& score,
                       SoundPlayer& sounds, float worldWidth, float worldHeight) {
    // 空间哈希网格 — 跨帧复用
    static int gridCols = 0;
    static int gridRows = 0;
    static Grid grid;

    int cols = static_cast<int>(worldWidth / CELL_SIZE) + 1;
    int rows = static_cast<int>(worldHeight / CELL_SIZE) + 1;
    if (cols != gridCols || rows != gridRows) {
        gridCols = cols;
        gridRows = rows;
        grid.resize(static_cast<std::size_t>(cols) * static_cast<std::size_t>(rows));
    }

    // 1. 构建网格
    for (auto& cell : grid) {
        cell.clear();
    }
    enemies.forEach([&](Enemy& e) {
        if (e.hp > 0.f) {
            int cx = std::clamp(static_cast<int>(e.pos.x / CELL_SIZE), 0, gridCols - 1);
            int cy = std::clamp(static_cast<int>(e.pos.y / CELL_SIZE), 0, gridRows - 1);
            grid[(cy * gridCols) + cx].push_back(&e);
        }
    });

    // 2. 碰撞检测
    processProjectileEnemyCollisions(projectiles, grid, gridCols, gridRows, damageTexts, gems,
                                     score, sounds);
    processPlayerEnemyCollision(player, grid, gridCols, gridRows, sounds);
    processPlayerGemPickups(player, gems, sounds);

    // 3. 清理
    cleanupEntities(enemies, projectiles, gems, score, sounds);
}

} // namespace CollisionSystem
